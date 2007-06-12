/* OpenSceneGraph example, osgspacewarp.
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
#include <osg/Geometry>

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>

float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }


struct DrawCallback : public osg::Drawable::DrawCallback
{

    DrawCallback():
        _firstTime(true) {}

    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        osg::State& state = *renderInfo.getState();
    
        if (!_firstTime)
        {
            _previousModelViewMatrix = _currentModelViewMatrix;
            _currentModelViewMatrix = state.getModelViewMatrix();
            _inverseModelViewMatrix.invert(_currentModelViewMatrix);
            
            osg::Matrix T(_previousModelViewMatrix*_inverseModelViewMatrix);

            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(const_cast<osg::Drawable*>(drawable));
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            for(unsigned int i=0;i+1<vertices->size();i+=2)
            {
                (*vertices)[i+1] = (*vertices)[i]*T;
            }
        }
        else
        {
            _currentModelViewMatrix = state.getModelViewMatrix();
        }
                
        _firstTime = false;

        drawable->drawImplementation(renderInfo);
    }
    
    mutable bool _firstTime;
    mutable osg::Matrix _currentModelViewMatrix;
    mutable osg::Matrix _inverseModelViewMatrix;
    mutable osg::Matrix _previousModelViewMatrix;
};




osg::Node* createScene(unsigned int noStars)
{
    
    osg::Geometry* geometry = new osg::Geometry;
    
    // set up vertices
    osg::Vec3Array* vertices = new osg::Vec3Array(noStars*2);
    geometry->setVertexArray(vertices);
    
    float min = -1.0f;
    float max = 1.0f;
    unsigned int j = 0;
    unsigned int i = 0;
    for(i=0;i<noStars;++i,j+=2)
    {
        (*vertices)[j].set(random(min,max),random(min,max),random(min,max));
        (*vertices)[j+1] = (*vertices)[j]+osg::Vec3(0.0f,0.0f,0.001f);
    }    

    // set up colours
    osg::Vec4Array* colours = new osg::Vec4Array(1);
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);

    // set up the primitive set to draw lines
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,noStars*2));

    // set up the points for the stars.
    osg::DrawElementsUShort* points = new osg::DrawElementsUShort(GL_POINTS,noStars);
    geometry->addPrimitiveSet(points);
    for(i=0;i<noStars;++i)
    {
        (*points)[i] = i*2;
    }

    geometry->setUseDisplayList(false);
    geometry->setDrawCallback(new DrawCallback);
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::Group* group = new osg::Group;
    group->addChild(geode);        
    
    return group;
}

int main(int , char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // set the scene to render
    viewer.setSceneData(createScene(50000));

    return viewer.run();
}

