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

osg::Geode* createTile(const osg::Vec3& lb, const osg::Vec3& rb,
                       const osg::Vec3& lt, const osg::Vec3& rt,
                       const std::string& imageFile)
{

    // create the geometry.
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0] = lt;
    (*coords)[1] = lb;
    (*coords)[2] = rb;
    (*coords)[3] = rt;
    geom->setVertexArray(coords);

    osg::Vec2Array* tcoords = new osg::Vec2Array(4);
    (*tcoords)[0].set(0.0f,1.0f);
    (*tcoords)[1].set(0.0f,0.0f);
    (*tcoords)[2].set(1.0f,0.0f);
    (*tcoords)[3].set(1.0f,1.0f);
    geom->setTexCoordArray(0,tcoords);

    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    geom->setColorArray(colours);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = (rb-lb)^(lt-lb);
    (*normals)[0].normalize();
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    
    
    // add the texture to the geometry.
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, // texture unit 0.
        new osg::Texture2D(osgDB::readImageFile(imageFile)),
        osg::StateAttribute::ON);
    
    
    // create the geode.
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    
    return geode;

}


osg::Node* createPagedModel()
{
    osg::PagedLOD* level_0 = new osg::PagedLOD;

    float distance = 1000.0f;
    
    osg::Vec3 lb(0.0f,0.0f,0.0f);
    osg::Vec3 rb(distance,0.0f,0.0f);
    osg::Vec3 rt(distance,0.0f,distance);
    osg::Vec3 lt(0.0f,0.0f,distance);
    
    level_0->addChild(createTile(lb,rb,lt,rt,"lz.rgb"),distance,1e5,"");
    level_0->addChild(createTile(lb,rb,lt,rt,"land_shallow_topo_2048.jpg"),0,distance,"level_1.osg");

    return level_0;    
}

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
            if (!filename.empty()) osgDB::writeNodeFile(*child,filename);
        }
    
        traverse(plod);
    }    
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


    osg::ref_ptr<osg::Node> model = createPagedModel();
    
    if (model.valid())
    {
        osgDB::writeNodeFile(*model,"level_0.osg");
        
        WriteOutPagedLODSubgraphsVistor woplsv;
        model->accept(woplsv);
    }

    return 0;
}
