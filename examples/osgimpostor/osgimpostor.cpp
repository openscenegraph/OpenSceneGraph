/* OpenSceneGraph example, osgimpostor.
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

#include <osg/Geometry>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Quat>
#include <osg/Geode>

#include <osgSim/Impostor>
#include <osgSim/InsertImpostorsVisitor>

#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include "TestManipulator.h"


#include <iostream>
#include <list>

// container storing all house nodes
typedef osg::ref_ptr<osg::Node>    NodePtr;
typedef std::list<NodePtr>        NodeContainer;
typedef NodeContainer::iterator    NodeIterator;

NodeContainer                    nodes;

//
osg::Group * Root = 0;

const int HOUSES_SIZE = 25000;        // total number of houses
double XDim = 5000.0f;                // area dimension +/- XDim
double ZDim = 5000.0f;                // area dimension +/- YDim

int GridX = 20;                        // number of grids in x direction
int GridY = 20;                        // number of grids in y direction

bool UseImpostor = true;            // use impostor (or do not use)

float Threshold = 3000.0f;            // distance where impostor are shown

// create houses and store nodes in container
void CreateHouses()
{
    int i;

    GLubyte indices[48] = {
        0, 2, 1,
        3, 2, 0,
        0, 4, 7,
        7, 3, 0,
        0, 1, 5,
        5, 4, 0,
        1, 6, 5,
        2, 6, 1,
        2, 3, 7,
        2, 7, 6,
        4, 8, 7,
        5, 6, 9,
        4, 5, 8,
        8, 5, 9,
        6, 7, 8,
        8, 9, 6
    };

    // use the same color, normal and indices for all houses.
        osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0] = osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // normals
    osg::Vec3Array * normals = new osg::Vec3Array(16);
    (*normals)[0] = osg::Vec3( 0.0f,  -0.0f, -1.0f);
    (*normals)[1] = osg::Vec3( 0.0f,  -0.0f, -1.0f);
    (*normals)[2] = osg::Vec3( 0.0f,  -1.0f,  0.0f);
    (*normals)[3] = osg::Vec3( 0.0f,  -1.0f,  0.0f);
    (*normals)[4] = osg::Vec3( 1.0f,  -0.0f,  0.0f);
    (*normals)[5] = osg::Vec3( 1.0f,  -0.0f,  0.0f);
    (*normals)[6] = osg::Vec3( 0.0f, 1.0f,  0.0f);
    (*normals)[7] = osg::Vec3( 0.0f, 1.0f,  0.0f);
    (*normals)[8] = osg::Vec3(-1.0f,  -0.0f,  0.0f);
    (*normals)[9] = osg::Vec3(-1.0f,  -0.0f,  0.0f);
    (*normals)[10] = osg::Vec3( 0.0f,  -0.928477f, 0.371391f);
    (*normals)[11] = osg::Vec3( 0.0f, 0.928477f, 0.371391f);
    (*normals)[12] = osg::Vec3( 0.707107f,  0.0f, 0.707107f);
    (*normals)[13] = osg::Vec3( 0.707107f,  0.0f, 0.707107f);
    (*normals)[14] = osg::Vec3(-0.707107f,  0.0f, 0.707107f);
    (*normals)[15] = osg::Vec3(-0.707107f,  0.0f, 0.707107f);

    // coordIndices
    osg::UByteArray* coordIndices = new osg::UByteArray(48,indices);

        // share the primitive set.
        osg::PrimitiveSet* primitives = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,48);

    for (int q = 0; q < HOUSES_SIZE; q++)
    {
        float xPos = ((static_cast<double> (rand()) /
                      static_cast<double> (RAND_MAX))
                     * 2.0 * XDim) - XDim;

        float yPos = ((static_cast<double> (rand()) /
                      static_cast<double> (RAND_MAX))
                     * 2 * ZDim) - ZDim;

        float scale = 10.0f;
                
                osg::Vec3 offset(xPos,yPos,0.0f);

        // coords
        osg::Vec3Array* coords = new osg::Vec3Array(10);
        (*coords)[0] = osg::Vec3( 0.5f, -0.7f, 0.0f);
        (*coords)[1] = osg::Vec3( 0.5f,  0.7f, 0.0f);
        (*coords)[2] = osg::Vec3(-0.5f, 0.7f, 0.0f);
        (*coords)[3] = osg::Vec3(-0.5f, -0.7f, 0.0f);
        (*coords)[4] = osg::Vec3( 0.5f, -0.7f, 1.0f);
        (*coords)[5] = osg::Vec3( 0.5f,  0.7f, 1.0f);
        (*coords)[6] = osg::Vec3(-0.5f,  0.7f, 1.0f);
        (*coords)[7] = osg::Vec3(-0.5f, -0.7f, 1.0f);
        (*coords)[8] = osg::Vec3( 0.0f, -0.5f, 1.5f);
        (*coords)[9] = osg::Vec3( 0.0f,  0.5f, 1.5f);

        for (i = 0; i < 10; i++)
        {
            (*coords)[i] = (*coords)[i] * scale + offset;
        }


        // create geometry
        osg::Geometry * geometry = new osg::Geometry();
                
        geometry->addPrimitiveSet(primitives);
                
        geometry->setVertexArray(coords);
                geometry->setVertexIndices(coordIndices);
                
        geometry->setColorArray(colors);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
                
        geometry->setNormalArray(normals);
        geometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

        osg::Geode * geode = new osg::Geode();
        geode->addDrawable(geometry);
        
        nodes.push_back(geode);
    }
}

void LayoutAsGrid()
{
    // calculate bounding box
    osg::BoundingBox bbox;
    for (NodeIterator node = nodes.begin(); node != nodes.end(); ++node)
        bbox.expandBy((*node)->getBound());

    // setup grid information
    osg::Group ** groups = new osg::Group*[GridX * GridY];
        int i;
    for (i = 0; i < GridX * GridY; i++)
        groups[i] = new osg::Group();

    float xGridStart = bbox.xMin();
    float xGridSize = (bbox.xMax() - bbox.xMin()) / GridX;

    float yGridStart = bbox.yMin();
    float yGridSize = (bbox.yMax() - bbox.yMin()) / GridY;

    // arrange buildings into right grid
    for (NodeIterator nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter)
    {
        osg::Node * node = nodeIter->get();
        osg::Vec3 center = node->getBound().center();
        
        int x = (int)floor((center.x() - xGridStart) / xGridSize);
        int z = (int)floor((center.y() - yGridStart) / yGridSize);

        groups[z * GridX + x]->addChild(node);
    }
 
    // add nodes to building root    
    for (i = 0; i < GridX * GridY; i++)
    {
        osg::StateSet * stateset = new osg::StateSet();

        osg::Material * material = new osg::Material();
        osg::Vec4 color = osg::Vec4(
            0.5f + (static_cast<double> (rand()) / (2.0*static_cast<double> (RAND_MAX))),
            0.5f + (static_cast<double> (rand()) / (2.0*static_cast<double> (RAND_MAX))),
            0.5f + (static_cast<double> (rand()) / ( 2.0*static_cast<double>(RAND_MAX))),
            1.0f);
            
        material->setAmbient(osg::Material::FRONT_AND_BACK, color);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
        stateset->setAttributeAndModes(material, osg::StateAttribute::ON);

        groups[i]->setStateSet(stateset);

        if (UseImpostor)
        {
            osgSim::Impostor * impostor = new osgSim::Impostor();
            impostor->setImpostorThreshold(static_cast<float> (Threshold));
            impostor->addChild(groups[i]);
            impostor->setRange(0, 0.0f, 1e7f);
            impostor->setCenter(groups[i]->getBound().center());
            Root->addChild(impostor);
        }
        else
        {
            Root->addChild(groups[i]);
        }
    }

    delete[] groups;
}


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // add local test manipulator more suitable for testing impostors.
    viewer.setCameraManipulator(new TestManipulator);


    // load the nodes from the commandline arguments.
    osg::Node* model = osgDB::readNodeFiles(arguments);
    if (model)
    {
        // the osgSim::InsertImpostorsVisitor used lower down to insert impostors
        // only operators on subclass of Group's, if the model top node is not
        // a group then it won't be able to insert an impostor.  We therefore
        // manually insert an impostor above the model.
        if (dynamic_cast<osg::Group*>(model)==0)
        {
            const osg::BoundingSphere& bs = model->getBound();
            if (bs.valid())
            {

                osgSim::Impostor* impostor = new osgSim::Impostor;

                // standard LOD settings
                impostor->addChild(model);
                impostor->setRange(0,0.0f,1e7f);
                impostor->setCenter(bs.center());

                // impostor specfic settings.
                impostor->setImpostorThresholdToBound(5.0f);

                model = impostor;

            }
        }

        // we insert an impostor node above the model, so we keep a handle
        // on the rootnode of the model, the is required since the
        // InsertImpostorsVisitor can add a new root in automatically and
        // we would know about it, other than by following the parent path
        // up from model.  This is really what should be done, but I'll pass
        // on it right now as it requires a getRoots() method to be added to
        // osg::Node, and we're about to make a release so no new features! 
        osg::Group* rootnode = new osg::Group;
        rootnode->addChild(model);


        // now insert impostors in the model using the InsertImpostorsVisitor.
        osgSim::InsertImpostorsVisitor ov;

        // traverse the model and collect all osg::Group's and osg::LOD's.
        // however, don't traverse the rootnode since we want to keep it as
        // the start of traversal, otherwise the insertImpostor could insert
        // and Impostor above the current root, making it nolonger a root!
        model->accept(ov);

        // insert the Impostors above groups and LOD's
        ov.insertImpostors();
    }
    else
    {
        // no user model so we'll create our own world.
        model = Root = new osg::Group();
        CreateHouses();
        LayoutAsGrid();
    }

    // add model to viewer.
    viewer.setSceneData(model);

    return viewer.run();
}
