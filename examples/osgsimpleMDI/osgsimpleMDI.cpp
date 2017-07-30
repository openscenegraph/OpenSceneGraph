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

#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/PrimitiveSetIndirect>
#include <osg/BufferIndexBinding>




///////////////////////////////////////////////////////////////////////////
#define MAXX 100
#define MAXY 100

int main( int argc, char**argv )
{

    osg::ArgumentParser arguments(&argc,argv);

    bool MDIenable=true;
    if(arguments.read("--classic"))
        MDIenable=false;

    osg::Geode* root( new osg::Geode );
    osg::BoundingBox bb;

    osg::Vec3 corner;
    osg::ref_ptr<osg::VertexBufferObject> vertbo=new osg::VertexBufferObject;
    osg::ref_ptr<osg::VertexBufferObject> texbo=new osg::VertexBufferObject;
    osg::ref_ptr<osg::ElementBufferObject> ebo=new osg::ElementBufferObject;

    ///create empty mdi
    osg::MultiDrawElementsIndirectUShort* mdi=new  osg::MultiDrawElementsIndirectUShort(osg::PrimitiveSet::TRIANGLE_STRIP);
    osg::DefaultIndirectCommandDrawElements* mdicommands= new osg::DefaultIndirectCommandDrawElements();
    mdi->setIndirectCommandArray(mdicommands);
    mdi->setBufferObject(ebo);

    osg::ref_ptr<osg::Geometry>	oldprHolder=new osg::Geometry();

    osg::Vec3 myCoords[] =
    {
        osg::Vec3(0,0.0f,0.7f),
        osg::Vec3(0,0.0f,0),
        osg::Vec3(0.7,0.0f,0),
        osg::Vec3(0.7f,0.0f,0.7f)
    };

    osg::Vec2 myTexCoords[] =
    {
        osg::Vec2(0,1),
        osg::Vec2(0,0),
        osg::Vec2(1,0),
        osg::Vec2(1,1)
    };
    unsigned short myIndices[] =
    {
        0,
        1,
        3,
        2
    };
    for(int j =0 ; j<MAXY; ++j) {
        for(int i =0 ; i<MAXX; ++i) {

            ///begin generate indexed quad
            corner=osg::Vec3(i,0,j);
            osg::Geometry* geom = new osg::Geometry();
            geom->setUseVertexBufferObjects(true);
            osg::Vec3Array * verts=new osg::Vec3Array(4,myCoords);
            for(int z=0; z<4; z++)(*verts)[z]+=corner;
            geom->setVertexArray(verts);
            geom->setTexCoordArray(0,new osg::Vec2Array(4,myTexCoords));
            geom->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP,4,myIndices));
            ///end generate indexed quad

            bb.expandBy(geom->getBoundingBox());

            if(MDIenable) {
                ///setup bos
                geom->getVertexArray()->setBufferObject(vertbo);
                geom->getTexCoordArray(0)->setBufferObject(texbo);
                osg::DrawElementsUShort *dre = static_cast<osg::DrawElementsUShort*>(geom->getPrimitiveSet(0));
                dre->setBufferObject(ebo);

                ///keep old pr alive for indices upload purpose
                oldprHolder->addPrimitiveSet(dre);
                geom->removePrimitiveSet(0,1);

                ///create indirect command
                osg::DrawElementsIndirectCommand cmd();
                cmd.count=4;
                cmd.instanceCount=1;
                cmd.firstIndex=root->getNumChildren()*4;
                cmd.baseVertex=root->getNumChildren()*4;
                mdicommands->push_back(cmd);

                if(cmd.firstIndex==0) {
                    ///only first geom have a mdi primset
                    geom->addPrimitiveSet(mdi);
                    ///avoid bound computation to fail because of the lack of support of basevertex in functors
                    geom->setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback);
                }

            }
            root->addChild(geom);
        }
    }

    if(MDIenable) {
        ///override bounds
        root->getDrawable(0)->setInitialBound(bb);
        ///put it somewhere
        root->setUserData( oldprHolder);
    }

    osgViewer::Viewer viewer;
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.setSceneData( root );
    return viewer.run();
}
