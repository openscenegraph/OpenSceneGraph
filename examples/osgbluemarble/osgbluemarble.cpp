/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/Texture2D>
#include <osg/Geometry>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ImageOptions>
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>

#include <osgProducer/Viewer>

osg::Vec3 computePosition(bool leftHemisphere, double x, double y)
{
    double latitude = osg::PI*x;
    double longitude = osg::PI*y;
    double sin_longitude = sin(longitude);

    if (leftHemisphere) return osg::Vec3(cos(latitude)*sin_longitude,sin(latitude)*sin_longitude,-cos(longitude));
    else return osg::Vec3(-cos(latitude)*sin_longitude,-sin(latitude)*sin_longitude,-cos(longitude));
}


osg::Node* createTile(const std::string& filename, bool leftHemisphere, double x, double y, double w,double h)
{
    osg::Geode* geode = new osg::Geode;
    
    osg::StateSet* stateset = new osg::StateSet();
    
    
    osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
    options->_sourceImageWindowMode = osgDB::ImageOptions::RATIO_WINDOW;
    options->_sourceRatioWindow.set(x,1-(y+h),w,h);

    options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
    options->_destinationPixelWindow.set(0,0,512,512);

    osgDB::Registry::instance()->setOptions(options.get());
    osg::Image* image = osgDB::readImageFile(filename.c_str());
    if (image)
    {
	osg::Texture2D* texture = new osg::Texture2D;
	texture->setImage(image);
        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
        texture->setMaxAnisotropy(8);
	stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    
    geode->setStateSet( stateset );

    unsigned int numColumns = 20;
    unsigned int numRows = 20;
    unsigned int r;
    unsigned int c;

    osg::Geometry* geometry = new osg::Geometry;

    osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
    osg::Vec3Array& n = *(new osg::Vec3Array(numColumns*numRows));
    osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
    osg::UByte4Array& color = *(new osg::UByte4Array(1));

    color[0].set(255,255,255,255);


    float rowTexDelta = 1.0f/(float)(numRows-1);
    float columnTexDelta = 1.0f/(float)(numColumns-1);

    double orig_latitude = osg::PI*x;
    double delta_latitude = osg::PI*w/(double)(numColumns-1);
    
    // measure as 0 at south pole, postive going north
    double orig_longitude = osg::PI*y;
    double delta_longitude = osg::PI*h/(double)(numRows-1);
    
    osg::Vec2 tex(0.0f,0.0f);
    int vi=0;
    double longitude = orig_longitude;
    osg::Vec3 normal;
    for(r=0;r<numRows;++r)
    {
        double latitude = orig_latitude;
        
        tex.x() = 0.0f;
	for(c=0;c<numColumns;++c)
	{
            double sin_longitude = sin(longitude);
            if (leftHemisphere)
            {
                normal.set(cos(latitude)*sin_longitude,sin(latitude)*sin_longitude,-cos(longitude));
            }
            else
            {
                normal.set(-cos(latitude)*sin_longitude,-sin(latitude)*sin_longitude,-cos(longitude));
            }
	    v[vi] = normal;
	    n[vi] = normal;
            
	    t[vi].set(tex.x(),tex.y());
            latitude+=delta_latitude;
            tex.x()+=columnTexDelta;
            ++vi;
	}
        longitude += delta_longitude;
        tex.y() += rowTexDelta;
    }

    geometry->setVertexArray(&v);
    geometry->setNormalArray(&n);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->setColorArray(&color);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->setTexCoordArray(0,&t);

    for(r=0;r<numRows-1;++r)
    {
        osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numColumns));
        geometry->addPrimitiveSet(&drawElements);
        int ei=0;
	for(c=0;c<numColumns;++c)
	{
	    drawElements[ei++] = (r+1)*numColumns+c;
	    drawElements[ei++] = (r)*numColumns+c;
	}
    }

    osgUtil::TriStripVisitor tsv;
    tsv.stripify(*geometry);
    
    geometry->setUseVertexBufferObjects(true);

    geode->addDrawable(geometry);

    return geode;
}

osg::Node* createTileAndRecurse(const std::string& filename, const std::string& basename, const std::string& extension, bool leftHemisphere, double x, double y, double w,double h, unsigned int numLevelsLeft)
{
    osg::Group* group = new osg::Group;
    
    if (numLevelsLeft>0)
    {
    
        std::string basename1 = basename+"_1";
        std::string basename2 = basename+"_2";
        std::string basename3 = basename+"_3";
        std::string basename4 = basename+"_4";
        
        {
            osg::ref_ptr<osg::Node> node = createTileAndRecurse(filename,basename1,extension,leftHemisphere,x,y,w*0.5,h*0.5,numLevelsLeft-1);
            osgDB::writeNodeFile(*node, basename1+extension);
        }

        {
            osg::ref_ptr<osg::Node> node = createTileAndRecurse(filename,basename2,extension,leftHemisphere,x+w*0.5,y,w*0.5,h*0.5,numLevelsLeft-1);
            osgDB::writeNodeFile(*node, basename2+extension);
        }

        {
            osg::ref_ptr<osg::Node> node = createTileAndRecurse(filename,basename3,extension,leftHemisphere,x,y+h*0.5,w*0.5,h*0.5,numLevelsLeft-1);
            osgDB::writeNodeFile(*node, basename3+extension);
        }

        {
            osg::ref_ptr<osg::Node> node = createTileAndRecurse(filename,basename4,extension,leftHemisphere,x+w*0.5,y+h*0.5,w*0.5,h*0.5,numLevelsLeft-1);
            osgDB::writeNodeFile(*node, basename4+extension);
        }

        float cut_off_distance = h*osg::PI;
        float max_visible_distance = 1e7;
    
        // create current layer, and write to disk.
        {

            {
                osg::PagedLOD* pagedlod = new osg::PagedLOD;
                osg::Node* tile = createTile(filename,leftHemisphere,x,y,w*0.5,h*0.5);
                pagedlod->addChild(tile, cut_off_distance,max_visible_distance);
                pagedlod->setRange(1,0.0f,cut_off_distance);
                pagedlod->setFileName(1,basename1+extension);
                pagedlod->setCenter(computePosition(leftHemisphere,x+w*0.25,y+h*0.25));

                group->addChild(pagedlod);
            }

            {
                osg::PagedLOD* pagedlod = new osg::PagedLOD;

                osg::Node* tile = createTile(filename,leftHemisphere,x+w*0.5,y,w*0.5,h*0.5);
                pagedlod->addChild(tile, cut_off_distance,max_visible_distance);
                pagedlod->setRange(1,0.0f,cut_off_distance);
                pagedlod->setFileName(1,basename2+extension);
                pagedlod->setCenter(computePosition(leftHemisphere,x+w*0.75,y+h*0.25));

                group->addChild(pagedlod);
            }

            {
                osg::PagedLOD* pagedlod = new osg::PagedLOD;

                osg::Node* tile = createTile(filename,leftHemisphere,x,y+h*0.5,w*0.5,h*0.5);
                pagedlod->addChild(tile, cut_off_distance,max_visible_distance);
                pagedlod->setRange(1,0.0f,cut_off_distance);
                pagedlod->setFileName(1,basename3+extension);
                pagedlod->setCenter(computePosition(leftHemisphere,x+w*0.25,y+h*0.75));

                group->addChild(pagedlod);
            }

            {
                osg::PagedLOD* pagedlod = new osg::PagedLOD;

                osg::Node* tile = createTile(filename,leftHemisphere,x+w*0.5,y+h*0.5,w*0.5,h*0.5);
                pagedlod->addChild(tile, cut_off_distance,max_visible_distance);
                pagedlod->setRange(1,0.0f,cut_off_distance);
                pagedlod->setFileName(1,basename4+extension);
                pagedlod->setCenter(computePosition(leftHemisphere,x+w*0.75,y+h*0.75));

                group->addChild(pagedlod);
            }
        
        }
        
        
    }
    else
    {
        // create current layer, and write to disk.
        {
            group->addChild(createTile(filename,leftHemisphere,x,y,w*0.5,h*0.5));
            group->addChild(createTile(filename,leftHemisphere,x+w*0.5,y,w*0.5,h*0.5));
            group->addChild(createTile(filename,leftHemisphere,x,y+h*0.5,w*0.5,h*0.5));
            group->addChild(createTile(filename,leftHemisphere,x+w*0.5,y+h*0.5,w*0.5,h*0.5));
        }
    }
    
    return group;
    
}

bool createWorld(const std::string& left_hemisphere, const std::string& right_hemisphere, const std::string& baseName, unsigned int numLevels)
{

    osgDB::ReaderWriter* readerWriter = osgDB::Registry::instance()->getReaderWriterForExtension("gdal");
    if (!readerWriter)
    {
        std::cout<<"Error: GDAL plugin not available, cannot preceed with database creation"<<std::endl;
        return false;
    }


    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // create the world
    

    std::string base = osgDB::getFilePath(baseName)+'/'+osgDB::getStrippedName(baseName);
    std::string extension = '.'+osgDB::getLowerCaseFileExtension(baseName);
    
    std::cout << "baseName = "<<baseName<<std::endl;
    std::cout << "base = "<<base<<std::endl;
    std::cout << "extension = "<<extension<<std::endl;


    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(createTileAndRecurse(left_hemisphere, base+"_west", extension, true, 0.0, 0.0, 1.0, 1.0, numLevels));
    group->addChild(createTileAndRecurse(right_hemisphere, base+"_east", extension, false, 0.0, 0.0, 1.0, 1.0, numLevels));
   
    osg::StateSet* stateset = group->getOrCreateStateSet();
    stateset->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osgDB::writeNodeFile(*group, baseName);

    
    osg::Timer_t end_tick = timer.tick();
    std::cout << "Time to create world "<<timer.delta_s(start_tick,end_tick)<<std::endl;
    
    return true;
}


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
//     if (arguments.argc()<=1)
//     {
//         arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
//         return 1;
//     }

    std::string left_hemisphere("land_shallow_topo_west.tif");
    std::string right_hemisphere("land_shallow_topo_east.tif");
    std::string basename("bluemarble.ive");
    
    createWorld(left_hemisphere,right_hemisphere,basename,4);

    return 0;
}

