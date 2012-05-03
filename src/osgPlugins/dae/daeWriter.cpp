/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#include "daeWriter.h"

#include <dom/domCOLLADA.h>

#include <dom/domNode.h>
#include <dom/domConstants.h>

#include <sstream>
#include <osgDB/ConvertUTF>


namespace osgDAE {

daeWriter::ArrayNIndices::ArrayNIndices( osg::Array* valArray, osg::IndexArray* ind ) :
    vec2(0),  vec3(0),  vec4(0),
    vec2d(0), vec3d(0), vec4d(0),
    vec4ub(0),
    valArray(valArray),
    inds( ind ), mode(NONE)
{
    if ( valArray != NULL )
    {
        switch( valArray->getType() )
        {
        case osg::Array::Vec2ArrayType:
            mode = VEC2F;
            vec2 = (osg::Vec2Array*)valArray;
            break;
        case osg::Array::Vec3ArrayType:
            mode = VEC3F;
            vec3 = (osg::Vec3Array*)valArray;
            break;
        case osg::Array::Vec4ArrayType:
            mode = VEC4F;
            vec4 = (osg::Vec4Array*)valArray;
            break;
        case osg::Array::Vec2dArrayType:
            mode = VEC2D;
            vec2d = (osg::Vec2dArray*)valArray;
            break;
        case osg::Array::Vec3dArrayType:
            mode = VEC3D;
            vec3d = (osg::Vec3dArray*)valArray;
            break;
        case osg::Array::Vec4dArrayType:
            mode = VEC4D;
            vec4d = (osg::Vec4dArray*)valArray;
            break;
        case osg::Array::Vec4ubArrayType:
            mode = VEC4_UB;
            vec4ub = (osg::Vec4ubArray*)valArray;
            break;
        default:
            OSG_WARN << "Array is unsupported vector type" << std::endl;
            break;
        }
    }
}


std::string toString(const osg::Vec3f& value)
{
    std::stringstream str;
    str << value.x() << " " << value.y() << " " << value.z();
    return str.str();
}

std::string toString(const osg::Vec3d& value)
{
    std::stringstream str;
    str << value.x() << " " << value.y() << " " << value.z();
    return str.str();
}

std::string toString(const osg::Matrix& value)
{
    std::stringstream str;
    str << value(0,0) << " " << value(1,0) << " " << value(2,0) << " " << value(3,0) << " "
        << value(0,1) << " " << value(1,1) << " " << value(2,1) << " " << value(3,1) << " "
        << value(0,2) << " " << value(1,2) << " " << value(2,2) << " " << value(3,2) << " "
        << value(0,3) << " " << value(1,3) << " " << value(2,3) << " " << value(3,3);
    return str.str();
}


daeWriter::Options::Options()
    : usePolygons(false),
    googleMode(false),
    writeExtras(true),
    earthTex(false),
    linkOrignialTextures(false),
    forceTexture(false),
    namesUseCodepage(false),
    relativiseImagesPathNbUpDirs(0)
{}

daeWriter::daeWriter( DAE *dae_, const std::string & fileURI, const std::string & directory, const std::string & srcDirectory, const osgDB::ReaderWriter::Options * options, TraversalMode tm, const Options * pluginOptions) : osg::NodeVisitor( tm ),
    dae(dae_),
    _domLibraryAnimations(NULL),
    rootName(*dae_),
    m_CurrentRenderingHint(osg::StateSet::DEFAULT_BIN),
    _options(options),
    _pluginOptions(pluginOptions ? *pluginOptions : Options()),
    _externalWriter(srcDirectory, directory, true, pluginOptions ? pluginOptions->relativiseImagesPathNbUpDirs : 0)
{
    success = true;

    dae->setDatabase( NULL );
    dae->setIOPlugin( NULL );
    //create document
    dae->getDatabase()->createDocument( fileURI.c_str(), &doc );
    dom = (domCOLLADA*)doc->getDomRoot();
    //create scene and instance visual scene
    domCOLLADA::domScene *scene = daeSafeCast< domCOLLADA::domScene >( dom->add( COLLADA_ELEMENT_SCENE ) );
    domInstanceWithExtra *ivs = daeSafeCast< domInstanceWithExtra >( scene->add( COLLADA_ELEMENT_INSTANCE_VISUAL_SCENE ) );
    ivs->setUrl( "#defaultScene" );
    //create library visual scenes and a visual scene and the root node
    lib_vis_scenes = daeSafeCast<domLibrary_visual_scenes>( dom->add( COLLADA_ELEMENT_LIBRARY_VISUAL_SCENES ) );
    vs = daeSafeCast< domVisual_scene >( lib_vis_scenes->add( COLLADA_ELEMENT_VISUAL_SCENE ) );
    vs->setId( "defaultScene" );
    currentNode = daeSafeCast< domNode >( vs->add( COLLADA_ELEMENT_NODE ) );
    currentNode->setId( "sceneRoot" );

    //create Asset
    //createAssetTag(m_ZUpAxis); // we now call this in the set root node

    lib_cameras = NULL;
    lib_effects = NULL;
    lib_controllers = NULL;
    lib_geoms = NULL;
    lib_lights = NULL;
    lib_mats = NULL;

    lastDepth = 0;

    // Clean up caches
    uniqueNames.clear();

    currentStateSet = new osg::StateSet();
}

daeWriter::~daeWriter()
{
}

void daeWriter::debugPrint( osg::Node &node )
{
#ifdef _DEBUG
    std::string indent = "";
    for ( unsigned int i = 0; i < _nodePath.size(); i++ )
    {
        indent += "  ";
    }
    OSG_INFO << indent << node.className() << std::endl;
#endif
}


void daeWriter::setRootNode( const osg::Node &node )
{
    std::string fname = osgDB::findDataFile( node.getName() );
    //create Asset with root node providing meta data
    createAssetTag( node );

    const_cast<osg::Node*>(&node)->accept( _animatedNodeCollector );
}

//### provide a name to node
std::string daeWriter::getNodeName(const osg::Node & node,const std::string & defaultName)
{
    std::string nodeName;
    if (node.getName().empty())
        nodeName=uniquify(defaultName);
    else
        nodeName=uniquify(node.getName());
    return nodeName;
}

//NODE
void daeWriter::apply( osg::Node &node )
{
    debugPrint( node );

    writeNodeExtra(node);

    traverse( node );
}

void daeWriter::updateCurrentDaeNode()
{
    while ( lastDepth >= _nodePath.size() )
    {
        //We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        --lastDepth;
    }
}

std::string daeWriter::uniquify( const std::string &_name )
{
    const std::string name( _pluginOptions.namesUseCodepage ? osgDB::convertStringFromCurrentCodePageToUTF8(_name) : _name );
    std::map< std::string, int >::iterator iter = uniqueNames.find( name );
    if ( iter != uniqueNames.end() )
    {
        iter->second++;
        std::ostringstream num;
        num << std::dec << iter->second;
        return name + "_" + num.str();
    }
    else
    {
        uniqueNames.insert( std::make_pair( name, 0 ) );
        return name;
    }
}

void daeWriter::createAssetTag( bool isZUpAxis )
{
    domAsset *asset = daeSafeCast< domAsset >(dom->add( COLLADA_ELEMENT_ASSET ) );
    domAsset::domCreated *c = daeSafeCast< domAsset::domCreated >(asset->add(COLLADA_ELEMENT_CREATED));
    domAsset::domModified *m = daeSafeCast< domAsset::domModified >(asset->add(COLLADA_ELEMENT_MODIFIED));
    domAsset::domUnit *u = daeSafeCast< domAsset::domUnit >(asset->add(COLLADA_ELEMENT_UNIT));
    domAsset::domUp_axis *up = daeSafeCast< domAsset::domUp_axis >(asset->add(COLLADA_ELEMENT_UP_AXIS));
    up->setValue(UPAXISTYPE_Z_UP);

    //TODO : set date and time
    c->setValue( "2006-07-25T00:00:00Z" );
    m->setValue( "2006-07-25T00:00:00Z" );

    u->setName( "meter" );
    u->setMeter( 1 );
}


// Overloaded version of createAssetTag that provides the ability to specify
// user defined values for child elements within asset tags
void daeWriter::createAssetTag(const osg::Node &node)
{
   domAsset *asset = daeSafeCast< domAsset >(dom->add( COLLADA_ELEMENT_ASSET ) );
   domAsset::domCreated *c = daeSafeCast< domAsset::domCreated >(asset->add("created" ));
   domAsset::domModified *m = daeSafeCast< domAsset::domModified >(asset->add("modified" ));
   domAsset::domUnit *u = daeSafeCast< domAsset::domUnit >(asset->add("unit"));
   domAsset::domUp_axis *up_axis = daeSafeCast< domAsset::domUp_axis >(asset->add("up_axis"));

   domAsset::domContributor *contributor = daeSafeCast< domAsset::domContributor >(asset->add("contributor" ));

   // set date and time
   // Generate the date like this
   // "YYYY-mm-ddTHH:MM:SSZ"  ISO 8601  Date Time format
   const size_t bufSize = 1024;
   static char dateStamp[bufSize];
   time_t rawTime = time(NULL);
   struct tm* timeInfo = localtime(&rawTime);
   strftime(dateStamp, sizeof(dateStamp), "%Y-%m-%dT%H:%M:%SZ", timeInfo);


   // set up fallback defaults
   c->setValue( dateStamp );
   m->setValue( dateStamp );
   u->setName( "meter" );    // NOTE: SketchUp incorrectly sets this to "meters" but it does not really matter.
                             //  Also note that since the default is: <unit meter="1.0" name="meter"/>  this is equivalent to <unit/>
   u->setMeter( 1.0 );        // this is the important units setting as it tells consuming apps how to convert to meters.
   up_axis->setValue(UPAXISTYPE_Z_UP);



   // get description info as name value pairs
   if (node.getDescriptions().size()%2 == 0)
   {
      for(osg::Node::DescriptionList::const_iterator ditr=node.getDescriptions().begin();
         ditr!=node.getDescriptions().end();
         ++ditr)
      {
         std::string attrName( *ditr );   ++ditr;
         std::string attrValue( *ditr );

         if (attrName=="collada_created" && !attrValue.empty())
         {
            c->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_modified" && !attrValue.empty())
         {
            m->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_keywords" && !attrValue.empty())
         {
            domAsset::domKeywords *keywords = daeSafeCast< domAsset::domKeywords >(asset->add("keywords" ));
            keywords->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_revision" && !attrValue.empty())
         {
            domAsset::domRevision *revision = daeSafeCast< domAsset::domRevision >(asset->add("revision" ));
            revision->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_subject" && !attrValue.empty())
         {
            domAsset::domSubject  *subject = daeSafeCast< domAsset::domSubject >(asset->add("subject" ));
            subject->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_title" && !attrValue.empty())
         {
            domAsset::domTitle  *title = daeSafeCast< domAsset::domTitle >(asset->add("title" ));
            title->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_up_axis" && !attrValue.empty())
         {
            if (attrValue=="X_UP")
            {
               up_axis->setValue( UPAXISTYPE_X_UP );
            }
            else if (attrValue=="Y_UP")
            {
               up_axis->setValue( UPAXISTYPE_Y_UP );
            }
            else
            {
               up_axis->setValue( UPAXISTYPE_Z_UP );   // default
            }
         }
         else if (attrName=="collada_unit_name" && !attrValue.empty())
         {
            u->setName( attrValue.c_str() );
         }
         else if (attrName=="collada_unit_meter_length" && !attrValue.empty())
         {
            double fValFromStr(1.0);
            try {
               std::istringstream sConversion(attrValue);
               sConversion >> fValFromStr;
               u->setMeter((domFloat)fValFromStr);
            }
            catch (...)
            {
               // TODO: handle error
               u->setMeter((domFloat)fValFromStr);
               continue;
            }
         }
         else if (attrName=="collada_contributor{0}.author" && !attrValue.empty())
         {
            domAsset::domContributor::domAuthor *author =
               daeSafeCast< domAsset::domContributor::domAuthor >(contributor->add("author" ));
            author->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_contributor{0}.authoring_tool" && !attrValue.empty())
         {
            domAsset::domContributor::domAuthoring_tool *authoring_tool =
               daeSafeCast< domAsset::domContributor::domAuthoring_tool >(contributor->add("authoring_tool" ));
            authoring_tool->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_contributor{0}.comments" && !attrValue.empty())
         {
            domAsset::domContributor::domComments *comments =
               daeSafeCast< domAsset::domContributor::domComments >(contributor->add("comments" ));
            comments->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_contributor{0}.source_data" && !attrValue.empty())
         {
            domAsset::domContributor::domSource_data *source_data =
               daeSafeCast< domAsset::domContributor::domSource_data >(contributor->add("source_data" ));
            source_data->setValue( attrValue.c_str() );
         }
         else if (attrName=="collada_contributor{0}.copyright" && !attrValue.empty())
         {
            domAsset::domContributor::domCopyright *copyright =
               daeSafeCast< domAsset::domContributor::domCopyright >(contributor->add("copyright" ));
            copyright->setValue( attrValue.c_str() );
         }

         // TODO:  handle array of contributor data rather that just the first.
         // also there is probably a better way to pass attribute data as DescriptionList is a bit fragile

      }
   }
}

void daeWriter::traverse (osg::Node &node)
{
    pushStateSet(node.getStateSet());

    osg::NodeVisitor::traverse( node );

    popStateSet(node.getStateSet());
}

void daeWriter::pushStateSet(osg::StateSet* ss)
{
  if (NULL!=ss) {
    // Save our current stateset
    stateSetStack.push(currentStateSet.get());

    // merge with node stateset
    currentStateSet = static_cast<osg::StateSet*>(currentStateSet->clone(osg::CopyOp::SHALLOW_COPY));
    currentStateSet->merge(*ss);
  }
}


void daeWriter::popStateSet(osg::StateSet* ss)
{
    if (NULL!=ss) {
      // restore the previous stateset
      currentStateSet = stateSetStack.top();
      stateSetStack.pop();
    }
}

} // namespace osgdae
