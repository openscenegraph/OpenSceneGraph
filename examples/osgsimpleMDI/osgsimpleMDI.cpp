/* OpenSceneGraph example, osgtransformfeedback
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

/* file:        examples/osgsimpleMDI/osgsimpleMDI.cpp
* author:      Julien Valentin 2017-08-01
* copyright:   (C) 2013
* license:     OpenSceneGraph Public License (OSGPL)
*
* A simple example of mdi with basevertex
*
*/


#include <osg/GL2Extensions>
#include <osg/Notify>
#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Program>
#include <osg/Shader>
#include <osg/BlendFunc>

#include <iostream>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/PrimitiveSetIndirect>
#include <osg/BufferIndexBinding>



int main( int argc, char**argv )
{

    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates Multi Indirect Draw with basevertex");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] ");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--numX","square count on X");
    arguments.getApplicationUsage()->addCommandLineOption("--numY","square count on Y");
    arguments.getApplicationUsage()->addCommandLineOption("--classic","disable MDI and use classic DrawElements");

    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    int MAXX=200;
    int MAXY=200;
    arguments.read("--numX",MAXX);
    arguments.read("--numY",MAXY);

    enum PrimtiveSetUsage
    {
        MultiDraw,
        MultiplePrimitiveSets,
        SinglePrimitiveSet
    };

    PrimtiveSetUsage usage = MultiDraw;
    if(arguments.read("--classic"))
    {
        usage = MultiplePrimitiveSets;
        OSG_WARN<<"disabling MDI, using multiple PrimitiveSet"<<std::endl;
    }

    if(arguments.read("--single"))
    {
        usage = SinglePrimitiveSet;
        OSG_WARN<<"disabling MDI, using single PrimitiveSet"<<std::endl;
    }

    osg::Geode* root( new osg::Geode );

    osg::ref_ptr<osg::ElementBufferObject> ebo=new osg::ElementBufferObject;

    ///create empty mdi
    osg::MultiDrawElementsIndirectUShort* mdi=new  osg::MultiDrawElementsIndirectUShort(osg::PrimitiveSet::TRIANGLE_STRIP);
    osg::DefaultIndirectCommandDrawElements* mdicommands= new osg::DefaultIndirectCommandDrawElements();
    mdi->setIndirectCommandArray(mdicommands);

    osg::ref_ptr<osg::Geometry>  geom=new osg::Geometry();
    geom->setUseVertexBufferObjects(true);

    osg::BoundingBox bb;
    bb.set(0,0,0,MAXX,0,MAXY);
    //set bounds by hand cause of the lack of support of basevertex in PrimitiveFunctors
    geom->setInitialBound(bb);

    osg::Vec3 myCoords[] =
    {
        osg::Vec3(0,0.0f,0.7f),
        osg::Vec3(0,0.0f,0),
        osg::Vec3(0.7,0.0f,0),
        osg::Vec3(0.7f,0.0f,0.7f)
    };

    unsigned short myIndices[] =   {       0,        1,        3,        2    };
    unsigned int myIndicesUI[] =   {       0,        1,        3,        2    };

    osg::Vec3Array * verts=new osg::Vec3Array();

    for(int j =0 ; j<MAXY; ++j)
    {
        for(int i =0 ; i<MAXX; ++i)
        {
            ///create indirect command
            osg::DrawElementsIndirectCommand cmd;
            cmd.count=4;
            cmd.instanceCount=1;
            cmd.firstIndex=verts->size();
            cmd.baseVertex=verts->size();
            mdicommands->push_back(cmd);

            for(int z=0; z<4; z++)
            {
                verts->push_back(osg::Vec3(i,0,j)+myCoords[z]);
                mdi->addElement(myIndices[z]);
            }
        }
    }

    geom->setVertexArray(verts);

    switch(usage)
    {
        case(MultiDraw):
        {
            geom->addPrimitiveSet(mdi);
            break;

        }
        case(MultiplePrimitiveSets):
        {
            for(int i=0; i<MAXY*MAXX; ++i)
            {
                osg::ref_ptr<osg::DrawElementsUInt> dre = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP,4,myIndicesUI);
                dre->setElementBufferObject(ebo.get());
                geom->addPrimitiveSet(dre.get());
                for(int z=0; z<4; z++) myIndicesUI[z]+=4;
            }
            break;
        }
        case(SinglePrimitiveSet):
        {
            osg::ref_ptr<osg::DrawElementsUInt> primitives = new osg::DrawElementsUInt(GL_TRIANGLES);
            primitives->setElementBufferObject(ebo.get());
            geom->addPrimitiveSet(primitives.get());

            unsigned int vi = 0;
            for(int i=0; i<MAXY*MAXX; ++i)
            {
                primitives->push_back(vi);
                primitives->push_back(vi+2);
                primitives->push_back(vi+1);
                primitives->push_back(vi+1);
                primitives->push_back(vi+2);
                primitives->push_back(vi+3);
                vi += 4;
            }
            break;
        }
    }

    root->addChild(geom);

    osgViewer::Viewer viewer;
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.setSceneData( root );
    return viewer.run();
}
