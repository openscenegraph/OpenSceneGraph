#include <osg/Group>
#include <osg/Notify>
#include <osg/Geometry>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/PagedLOD>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>

#include <iostream>
#include <sstream>

class WriteOutPagedLODSubgraphsVistor : public osg::NodeVisitor
{
public:
    WriteOutPagedLODSubgraphsVistor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }
    
    virtual void apply(osg::PagedLOD& plod)
    {

        // go through all the named children and write them out to disk.
        for(unsigned int i=0;i<plod.getNumChildren();++i)
        {
            osg::Node* child = plod.getChild(i);
            std::string filename = plod.getFileName(i);
            if (!filename.empty())
            {
                osg::notify(osg::NOTICE)<<"Writing out "<<filename<<std::endl;
                osgDB::writeNodeFile(*child,filename);
            }
        }
    
        traverse(plod);
    }    
};

class ConvertToPageLODVistor : public osg::NodeVisitor
{
public:
    ConvertToPageLODVistor(const std::string& basename, const std::string& extension):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _basename(basename),
        _extension(extension)
    {
    }
    
    virtual ~ConvertToPageLODVistor()
    {
    }

    virtual void apply(osg::LOD& lod)
    {
        _lodSet.insert(&lod);
    
        traverse(lod);
    }    

    virtual void apply(osg::PagedLOD& plod)
    {
        // do thing, but want to avoid call LOD.
        traverse(plod);
    }

    void convert()
    {
        unsigned int lodNum = 0;
        for(LODSet::iterator itr = _lodSet.begin();
            itr != _lodSet.end();
            ++itr, ++lodNum)
        {
            osg::ref_ptr<osg::LOD> lod = const_cast<osg::LOD*>(itr->get());
            
            if (lod->getNumParents()==0)
            {
                osg::notify(osg::NOTICE)<<"Warning can't operator on root node."<<std::endl;
                break;
            }

            osg::notify(osg::NOTICE)<<"Converting LOD to PagedLOD"<<std::endl;
            
            osg::PagedLOD* plod = new osg::PagedLOD;
            
            const osg::LOD::RangeList& originalRangeList = lod->getRangeList();
            typedef std::map< osg::LOD::MinMaxPair , unsigned int > MinMaxPairMap;
            MinMaxPairMap rangeMap;
            unsigned int pos = 0;
            for(osg::LOD::RangeList::const_iterator ritr = originalRangeList.begin();
                ritr != originalRangeList.end();
                ++ritr, ++pos)
            {
                rangeMap[*ritr] = pos;
            }
            
            pos = 0;
            for(MinMaxPairMap::reverse_iterator mitr = rangeMap.rbegin();
                mitr != rangeMap.rend();
                ++mitr, ++pos)
            {
                if (pos==0)
                {
                    plod->addChild(lod->getChild(mitr->second), mitr->first.first, mitr->first.second);
                   osg::notify(osg::NOTICE)<<"  adding staight child"<<std::endl;
                }
                else
                {
                    std::string filename = _basename;
                    std::ostringstream os;
                    os << _basename << "_"<<lodNum<<"_"<<pos<<_extension;
                    
                    plod->addChild(lod->getChild(mitr->second), mitr->first.first, mitr->first.second, os.str());
                   osg::notify(osg::NOTICE)<<"  adding tiled subgraph"<<os.str()<<std::endl;
                }
            }
            
            osg::Node::ParentList parents = lod->getParents();
            for(osg::Node::ParentList::iterator pitr=parents.begin();
                pitr!=parents.end();
                ++pitr)
            {
                (*pitr)->replaceChild(lod.get(),plod);
            }

            plod->setCenter(plod->getBound().center());
            plod->setCenter(plod->getBound().center());

        }
    }

    
    typedef std::set< osg::ref_ptr<osg::LOD> >  LODSet;
    LODSet _lodSet;
    std::string _basename;
    std::string _extension;

};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" creates a hierachy of files for paging which can be later loaded by viewers.");
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


    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    
    if (!model)
    {
        osg::notify(osg::NOTICE)<<"No model loaded."<<std::endl;
        return 1;
    }
    
    ConvertToPageLODVistor converter("tile",".ive");
    model->accept(converter);
    converter.convert();
    
    if (model.valid())
    {
        osgDB::writeNodeFile(*model,"tile.ive");
        
        WriteOutPagedLODSubgraphsVistor woplsv;
        model->accept(woplsv);
    }

    return 0;
}
