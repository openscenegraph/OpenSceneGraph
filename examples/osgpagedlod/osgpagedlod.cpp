/* OpenSceneGraph example, osgpagedlod.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

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
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>

#include <iostream>
#include <sstream>

class NameVistor : public osg::NodeVisitor
{
public:
    NameVistor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _count(0)
    {
    }

    virtual void apply(osg::Node& node)
    {
        std::ostringstream os;
        os << node.className() << "_"<<_count++;

        node.setName(os.str());

        traverse(node);
    }

    unsigned int _count;
};

class CheckVisitor : public osg::NodeVisitor
{
public:
    CheckVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }

    virtual void apply(osg::PagedLOD& plod)
    {
        std::cout<<"PagedLOD "<<plod.getName()<<"  numRanges = "<< plod.getNumRanges()<<"  numFiles = "<<plod.getNumFileNames()<<std::endl;
        for(unsigned int i=0;i<plod.getNumFileNames();++i)
        {
            std::cout<<"  files = '"<<plod.getFileName(i)<<"'"<<std::endl;
        }
    }
};


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
    ConvertToPageLODVistor(const std::string& basename, const std::string& extension, bool makeAllChildrenPaged):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _basename(basename),
        _extension(extension),
        _makeAllChildrenPaged(makeAllChildrenPaged)
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

            if (!_makeAllChildrenPaged && lod->getNumRanges()<2)
            {
                osg::notify(osg::NOTICE)<<"Leaving LOD with one child as is."<<std::endl;
                break;
            }

            osg::notify(osg::NOTICE)<<"Converting LOD to PagedLOD."<<std::endl;

            osg::PagedLOD* plod = new osg::PagedLOD;

            const osg::LOD::RangeList& originalRangeList = lod->getRangeList();
            typedef std::multimap< osg::LOD::MinMaxPair , unsigned int > MinMaxPairMap;
            MinMaxPairMap rangeMap;
            unsigned int pos = 0;
            for(osg::LOD::RangeList::const_iterator ritr = originalRangeList.begin();
                ritr != originalRangeList.end();
                ++ritr, ++pos)
            {
                rangeMap.insert(std::multimap< osg::LOD::MinMaxPair , unsigned int >::value_type(*ritr, pos));
            }

            pos = 0;
            for(MinMaxPairMap::reverse_iterator mitr = rangeMap.rbegin();
                mitr != rangeMap.rend();
                ++mitr, ++pos)
            {
                if (pos==0 && !_makeAllChildrenPaged)
                {
                    plod->addChild(lod->getChild(mitr->second), mitr->first.first, mitr->first.second);
                }
                else
                {
                    std::string filename = _basename;
                    std::ostringstream os;
                    os << _basename << "_"<<lodNum<<"_"<<pos<<_extension;

                    plod->addChild(lod->getChild(mitr->second), mitr->first.first, mitr->first.second, os.str());
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


        }
    }


    typedef std::set< osg::ref_ptr<osg::LOD> >  LODSet;
    LODSet _lodSet;
    std::string _basename;
    std::string _extension;
    bool _makeAllChildrenPaged;
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" creates a hierarchy of files for paging which can be later loaded by viewers.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-o","set the output file (defaults to output.ive)");
    arguments.getApplicationUsage()->addCommandLineOption("--makeAllChildrenPaged","Force all children of LOD to be written out as external PagedLOD children");

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    std::string outputfile("output.ive");
    while (arguments.read("-o",outputfile)) {}


    bool makeAllChildrenPaged = false;
    while (arguments.read("--makeAllChildrenPaged")) { makeAllChildrenPaged = true; }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
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


    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFiles(arguments);

    if (!model)
    {
        osg::notify(osg::NOTICE)<<"No model loaded."<<std::endl;
        return 1;
    }

    std::string basename( osgDB::getNameLessExtension(outputfile) );
    std::string ext = '.'+ osgDB::getFileExtension(outputfile);

    ConvertToPageLODVistor converter(basename,ext, makeAllChildrenPaged);
    model->accept(converter);
    converter.convert();

    NameVistor nameNodes;
    model->accept(nameNodes);

    //CheckVisitor checkNodes;
    //model->accept(checkNodes);

    if (model.valid())
    {
        osgDB::writeNodeFile(*model,outputfile);

        WriteOutPagedLODSubgraphsVistor woplsv;
        model->accept(woplsv);
    }

    return 0;
}
