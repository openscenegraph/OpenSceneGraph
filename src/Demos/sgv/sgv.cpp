#ifdef USE_MEM_CHECK
#include <mcheck.h>
#endif

#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osgUtil/OptimizeStateVisitor>



class TransformFunctor : public osg::Drawable::AttributeFunctor
{
    public:
    
        osg::Matrix _m;
        osg::Matrix _im;

        TransformFunctor(const osg::Matrix& m):
            osg::Drawable::AttributeFunctor(osg::Drawable::COORDS|osg::Drawable::NORMALS)
        {
            _m = m;
            _im.invert(_m);
        }
            
        virtual ~TransformFunctor() {}

        virtual bool apply(osg::Drawable::AttributeBitMask abm,osg::Vec3* begin,osg::Vec3* end)
        {
            if (abm == osg::Drawable::COORDS)
            {
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    (*itr) = (*itr)*_m;
                }
                return true;
            }
            else if (abm == osg::Drawable::NORMALS)
            {
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    // note post mult by inverse for normals.
                    (*itr) = osg::Matrix::transform3x3(_im,(*itr));
                    (*itr).normalize();
                }
                return true;
            }
            return false;

        }

};


class FlattenStaticTransformsVisitor : public osg::NodeVisitor
{
    public:
    
        typedef std::vector<osg::Matrix> MatrixStack;
        MatrixStack                      _matrixStack;
        
        typedef std::set<osg::Transform*> TransformList;
        TransformList                     _transformList;
    
        FlattenStaticTransformsVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

        virtual void apply(osg::Geode& geode)
        {
            if (!_matrixStack.empty())
            {
                TransformFunctor tf(_matrixStack.back());
                for(int i=0;i<geode.getNumDrawables();++i)
                {
                    geode.getDrawable(i)->applyAttributeOperation(tf);
                }
            }
        }
        
        virtual void apply(osg::Billboard& billboard)
        {
            if (!_matrixStack.empty())
            {
                osg::Matrix& matrix = _matrixStack.back();
                TransformFunctor tf(matrix);

                osg::Vec3 axis = osg::Matrix::transform3x3(tf._im,billboard.getAxis());
                billboard.setAxis(axis);
               
                for(int i=0;i<billboard.getNumDrawables();++i)
                {
                    billboard.setPos(i,billboard.getPos(i)*matrix);
                    billboard.getDrawable(i)->applyAttributeOperation(tf);
                }
            }
        }

        virtual void apply(osg::LOD& lod)
        {
            if (!_matrixStack.empty())
            {
                lod.setCenter(lod.getCenter()*_matrixStack.back());
            }
            traverse(lod);
        }

        virtual void apply(osg::Transform& transform)
        {
            if (_matrixStack.empty())
            {
                _matrixStack.push_back(transform.getMatrix());
            }
            else
            {
                _matrixStack.push_back(transform.getMatrix()*_matrixStack.back());
            }
            
            traverse(transform);
            
            _transformList.insert(&transform);

            // reset the matrix to identity.
            transform.getMatrix().makeIdent();
            
            _matrixStack.pop_back();
        }
        
        void removeTransforms()
        {
            for(TransformList::iterator itr=_transformList.begin();
                itr!=_transformList.end();
                ++itr)
            {
                osg::ref_ptr<osg::Transform> transform = *itr;
                osg::ref_ptr<osg::Group>     group = new osg::Group;
               
                int i;
                for(i=0;i<transform->getNumChildren();++i)
                {
                    for(int j=0;j<transform->getNumParents();++j)
                    {
                        group->addChild(transform->getChild(i));
                    }
                }

                for(i=transform->getNumParents()-1;i>=0;--i)
                {
                    transform->getParent(i)->replaceChild(transform.get(),group.get());
                }                
                
            }
            _transformList.clear();
        }
        
};


class RemoveRedundentNodesVisitor : public osg::NodeVisitor
{
    public:
    
        typedef std::set<osg::Node*> NodeList;
        NodeList                     _redundentNodeList;
    
        RemoveRedundentNodesVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

        virtual void apply(osg::Group& group)
        {
            if (typeid(group)==typeid(osg::Group))
            {
                if (group.getNumParents()>0 && group.getNumChildren()<=1)
                {
                    _redundentNodeList.insert(&group);
                }
            }
            traverse(group);
        }

        
        void removeRedundentNodes()
        {
            for(NodeList::iterator itr=_redundentNodeList.begin();
                itr!=_redundentNodeList.end();
                ++itr)
            {
                osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(*itr);
                if (group.valid())
                {

                    for(int j=group->getNumParents()-1;j>=0;--j)
                    {
                        for(int i=0;i<group->getNumChildren();++i)
                        {
                            group->getParent(j)->addChild(group->getChild(i));
                        }
                        group->getParent(j)->removeChild(group.get());
                    }
                }                                
            }
            _redundentNodeList.clear();
        }
        
};

class CombineLODsVisitor : public osg::NodeVisitor
{
    public:
    
        typedef std::set<osg::Group*>  GroupList;
        GroupList                      _groupList;
    
        CombineLODsVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

        virtual void apply(osg::LOD& lod)
        {
            for(int i=0;i<lod.getNumParents();++i)
            {
                if (typeid(*lod.getParent(i))==typeid(osg::Group))
                {
                    _groupList.insert(lod.getParent(i));
                }
            }
            traverse(lod);
        }

        
        void combineLODs()
        {
            for(GroupList::iterator itr=_groupList.begin();
                itr!=_groupList.end();
                ++itr)
            {
                osg::Group* group = *itr;

                typedef std::set<osg::LOD*>                 LODSet;
                
                LODSet    lodChildren;
                
                for(int i=0;i<group->getNumChildren();++i)
                {
                    osg::Node* child = group->getChild(i);
                    osg::LOD* lod = dynamic_cast<osg::LOD*>(child);
                    if (lod)
                    {
                        if (lod->getNumRanges()-1==lod->getNumChildren())
                        {
                            lodChildren.insert(lod);
                        }
                        else
                        {
                            // wonky LOD, numRanges should = numChildren+1
                        }
                    }
                }
                
                if (lodChildren.size()>=2)
                {
                    osg::BoundingBox bb;
                    LODSet::iterator lod_itr;
                    for(lod_itr=lodChildren.begin();
                        lod_itr!=lodChildren.end();
                        ++lod_itr)
                    {

                        bb.expandBy((*lod_itr)->getCenter());
                    }
                    if (bb.radius()<1e-2)
                    {
                        typedef std::pair<float,float> RangePair;
                        typedef std::multimap<RangePair,osg::Node*> RangeMap;
                        RangeMap rangeMap;
                        float maxRange = 0.0f;
                        for(lod_itr=lodChildren.begin();
                            lod_itr!=lodChildren.end();
                            ++lod_itr)
                        {
                        
                            osg::LOD* lod = *lod_itr;
                            for(int i=0;i<lod->getNumRanges()-1;++i)
                            {
                                if (maxRange<lod->getRange(i+1)) maxRange = lod->getRange(i+1);
                                rangeMap.insert(RangeMap::value_type(RangePair(lod->getRange(i),lod->getRange(i+1)),lod->getChild(i)));
                            }
                            
                        }

                        // create new LOD containing all other LOD's children.
                        osg::LOD* newLOD = new osg::LOD;
                        newLOD->setName("newLOD");
                        newLOD->setCenter(bb.center());
                        
                        int i=0;
                        for(RangeMap::iterator c_itr=rangeMap.begin();
                            c_itr!=rangeMap.end();
                            ++c_itr,++i)
                        {
                            newLOD->setRange(i,c_itr->first.first);
                            newLOD->addChild(c_itr->second);
                        }
                        newLOD->setRange(i,maxRange);
                        
                        // add LOD into parent.
                        group->addChild(newLOD);
                        
                        // remove all the old LOD's from group.
                        for(lod_itr=lodChildren.begin();
                            lod_itr!=lodChildren.end();
                            ++lod_itr)
                        {
                            group->removeChild(*lod_itr);
                        }

                    }

                }
            }
            _groupList.clear();
        }
        
};



/*
 * Function to read several files (typically one) as specified on the command
 * line, and return them in an osg::Node
 */
osg::Node* getNodeFromFiles(int argc,char **argv)
{
    osg::Node *rootnode = new osg::Node;

    int i;

    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;
    for( i = 1; i < argc; i++ )
    {

        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case('l'):
                    ++i;
                    if (i<argc)
                    {
                        osgDB::Registry::instance()->loadLibrary(argv[i]);
                    }
                    break;
                case('e'):
                    ++i;
                    if (i<argc)
                    {
                        std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[i]);
                        osgDB::Registry::instance()->loadLibrary(libName);
                    }
                    break;
            }
        } else
        {
            osg::Node *node = osgDB::readNodeFile( argv[i] );

            if( node != (osg::Node *)0L )
            {
                if (node->getName().empty()) node->setName( argv[i] );
                nodeList.push_back(node);
            }
        }

    }

    if (nodeList.size()==0)
    {
        osg::notify(osg::WARN) << "No data loaded."<<endl;
        exit(0);
    }

    if (nodeList.size()==1)
    {
        rootnode = nodeList.front();
    }
    else                         // size >1
    {
        osg::Group* group = new osg::Group();
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        rootnode = group;
    }

    return rootnode;
}


int main( int argc, char **argv )
{

#ifdef USE_MEM_CHECK
    mtrace();
#endif

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        osg::notify(osg::NOTICE)<<"usage:"<<endl;
        osg::notify(osg::NOTICE)<<"    sgv [options] infile1 [infile2 ...]"<<endl;
        osg::notify(osg::NOTICE)<<endl;
        osg::notify(osg::NOTICE)<<"options:"<<endl;
        osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<endl;
        osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<endl;
        osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<endl;
        osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<endl;
        osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<endl;
        osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<endl;
        osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<endl;
        osg::notify(osg::NOTICE)<<endl;

        return 0;
    }

    osg::Timer timer;
    osg::Timer_t before_load = timer.tick();
    
    // comment out right now, but the following allos users to pass option data to
    // the ReaderWriter plugins. By default the options are set to NULL. The basic
    // osgDB::ReaderWriter::Options stucture has just a string, but this can be
    // subclassed to extend it to handle any options that a user desires.
    // osgDB::Registry::instance()->setOptions(new osgDB::ReaderWriter::Options("test options"));

    osg::Node* rootnode = getNodeFromFiles( argc, argv);
    
    osg::Timer_t after_load = timer.tick();
    cout << "Time for load = "<<timer.delta_s(before_load,after_load)<<" seconds"<<endl;

    // note, the Microsoft visual C++ compilers can't handle the STL
    // in the OptimizeStateVisitor and crash if we run it.  For the
    // time being we'll just not use the optimize visitor under windows.
    #ifndef WIN32
    // optimize the state in scene graph, removing duplicate state.
    osgUtil::OptimizeStateVisitor osv;
    rootnode->accept(osv);
    osv.optimize();
    #endif

    CombineLODsVisitor clv;
    rootnode->accept(clv);        
    clv.combineLODs();

    FlattenStaticTransformsVisitor fstv;
    rootnode->accept(fstv);
    fstv.removeTransforms();
    
    RemoveRedundentNodesVisitor rrnv;
    rootnode->accept(rrnv);
    rrnv.removeRedundentNodes();

     
    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.addViewport( rootnode );

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    viewer.open();
    viewer.run();

    return 0;
}
