/* OpenSceneGraph example, osgmovie.
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

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/ReadFile>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/TexMat>
#include <osg/CullFace>
#include <osg/ImageStream>
#include <osg/io_utils>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/EventVisitor>

#include <iostream>

osg::ImageStream* s_imageStream = 0;
class MovieEventHandler : public osgGA::GUIEventHandler
{
public:

    MovieEventHandler():_trackMouse(false),playToggle_(true) {}
    
    void setMouseTracking(bool track) { _trackMouse = track; }
    bool getMouseTracking() const { return _trackMouse; }
    
    void set(osg::Node* node);

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

    typedef std::vector< osg::observer_ptr<osg::ImageStream> > ImageStreamList;

protected:

    virtual ~MovieEventHandler() {}

    class FindImageStreamsVisitor : public osg::NodeVisitor
    {
    public:
        FindImageStreamsVisitor(ImageStreamList& imageStreamList):
            _imageStreamList(imageStreamList) {}
            
        virtual void apply(osg::Geode& geode)
        {
            apply(geode.getStateSet());

            for(unsigned int i=0;i<geode.getNumDrawables();++i)
            {
                apply(geode.getDrawable(i)->getStateSet());
            }
        
            traverse(geode);
        }

        virtual void apply(osg::Node& node)
        {
            apply(node.getStateSet());
            traverse(node);
        }
        
        inline void apply(osg::StateSet* stateset)
        {
            if (!stateset) return;
            
            osg::StateAttribute* attr = stateset->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
            if (attr)
            {
                osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(attr);
                if (texture2D) apply(dynamic_cast<osg::ImageStream*>(texture2D->getImage()));

                osg::TextureRectangle* textureRec = dynamic_cast<osg::TextureRectangle*>(attr);
                if (textureRec) apply(dynamic_cast<osg::ImageStream*>(textureRec->getImage()));
            }
        }
        
        inline void apply(osg::ImageStream* imagestream)
        {
            if (imagestream)
            {
                _imageStreamList.push_back(imagestream); 
                s_imageStream = imagestream;
            }
        }
        
        ImageStreamList& _imageStreamList;
    };


    bool            playToggle_;
    bool            _trackMouse;
    ImageStreamList _imageStreamList;
    
};



void MovieEventHandler::set(osg::Node* node)
{
    _imageStreamList.clear();
    if (node)
    {
        FindImageStreamsVisitor fisv(_imageStreamList);
        node->accept(fisv);
    }
}


bool MovieEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::PUSH):
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            if (_trackMouse)
            {
                osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                osgUtil::LineSegmentIntersector::Intersections intersections;
                bool foundIntersection = view==0 ? false :
                    (nv==0 ? view->computeIntersections(ea.getX(), ea.getY(), intersections) :
                             view->computeIntersections(ea.getX(), ea.getY(), nv->getNodePath(), intersections));
                
                if (foundIntersection)
                {

                    // use the nearest intersection                 
                    const osgUtil::LineSegmentIntersector::Intersection& intersection = *(intersections.begin());
                    osg::Drawable* drawable = intersection.drawable.get();
                    osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;
                    osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;
                    if (vertices)
                    {
                        // get the vertex indices.
                        const osgUtil::LineSegmentIntersector::Intersection::IndexList& indices = intersection.indexList;
                        const osgUtil::LineSegmentIntersector::Intersection::RatioList& ratios = intersection.ratioList;

                        if (indices.size()==3 && ratios.size()==3)
                        {
                            unsigned int i1 = indices[0];
                            unsigned int i2 = indices[1];
                            unsigned int i3 = indices[2];

                            float r1 = ratios[0];
                            float r2 = ratios[1];
                            float r3 = ratios[2];

                            osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
                            osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
                            if (texcoords_Vec2Array)
                            {
                                // we have tex coord array so now we can compute the final tex coord at the point of intersection.                                
                                osg::Vec2 tc1 = (*texcoords_Vec2Array)[i1];
                                osg::Vec2 tc2 = (*texcoords_Vec2Array)[i2];
                                osg::Vec2 tc3 = (*texcoords_Vec2Array)[i3];
                                osg::Vec2 tc = tc1*r1 + tc2*r2 + tc3*r3;

                                osg::notify(osg::NOTICE)<<"We hit tex coords "<<tc<<std::endl;

                            }
                        }
                        else
                        {
                            osg::notify(osg::NOTICE)<<"Intersection has insufficient indices to work with";
                        }

                    }
                }
                else
                {
                    osg::notify(osg::NOTICE)<<"No intersection"<<std::endl;
                }
            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='p')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    playToggle_ = !playToggle_;
                    if ( playToggle_ )
                    {
                        // playing, so pause
                        std::cout<<"Play"<<std::endl;
                        (*itr)->play();
                    }
                    else
                    {
                        // playing, so pause
                        std::cout<<"Pause"<<std::endl;
                        (*itr)->pause();
                    }
                }
                return true;
            }
            else if (ea.getKey()=='r')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    std::cout<<"Restart"<<std::endl;
                    (*itr)->rewind();
                    (*itr)->play();
                }
                return true;
            }
            else if (ea.getKey()=='L')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    if ( (*itr)->getLoopingMode() == osg::ImageStream::LOOPING)
                    {
                        std::cout<<"Toggle Looping Off"<<std::endl;
                        (*itr)->setLoopingMode( osg::ImageStream::NO_LOOPING );
                    }
                    else
                    {
                        std::cout<<"Toggle Looping On"<<std::endl;
                        (*itr)->setLoopingMode( osg::ImageStream::LOOPING );
                    }
                }
                return true;
            }
            return false;
        }

        default:
            return false;
    }
    return false;
}

void MovieEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("p","Play/Pause movie");
    usage.addKeyboardMouseBinding("r","Restart movie");
    usage.addKeyboardMouseBinding("l","Toggle looping of movie");
}


osg::Geometry* myCreateTexturedQuadGeometry(const osg::Vec3& pos,float width,float height, osg::Image* image, bool useTextureRectangle, bool xyPlane, bool option_flip)
{
    bool flip = image->getOrigin()==osg::Image::TOP_LEFT;
    if (option_flip) flip = !flip;
    
    if (useTextureRectangle)
    {
        osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(pos,
                                           osg::Vec3(width,0.0f,0.0f),
                                           xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height),
                                           0.0f, flip ? image->t() : 0.0, image->s(), flip ? 0.0 : image->t());

        osg::TextureRectangle* texture = new osg::TextureRectangle(image);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        
        
        pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                                                                        texture,
                                                                        osg::StateAttribute::ON);
        
        return pictureQuad;
    }
    else
    {
        osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(pos,
                                           osg::Vec3(width,0.0f,0.0f),
                                           xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height),
                                           0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);
                                    
        osg::Texture2D* texture = new osg::Texture2D(image);
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        
        
        pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                    texture,
                    osg::StateAttribute::ON);

        return pictureQuad;
    }
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example demonstrates the use of ImageStream for rendering movies as textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--texture2D","Use Texture2D rather than TextureRectangle.");
    arguments.getApplicationUsage()->addCommandLineOption("--shader","Use shaders to post process the video.");
    arguments.getApplicationUsage()->addCommandLineOption("--interactive","Use camera manipulator to allow movement around movie.");
    arguments.getApplicationUsage()->addCommandLineOption("--flip","Flip the movie so top becomes bottom.");
    arguments.getApplicationUsage()->addCommandLineOption("--devices","Print the Video input capability via QuickTime and exit.");
    
    bool useTextureRectangle = true;
    bool useShader = false;

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // if user requests devices video capability.
    if (arguments.read("-devices") || arguments.read("--devices"))
    {
        // Force load QuickTime plugin, probe video capability, exit
        osgDB::readImageFile("devices.live");
        return 1;
    }

    while (arguments.read("--texture2D")) useTextureRectangle=false;
    while (arguments.read("--shader")) useShader=true;

    bool mouseTracking = false;
    while (arguments.read("--mouse")) mouseTracking=true; 


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    bool fullscreen = !arguments.read("--interactive");
    bool flip = arguments.read("--flip");

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    if (useShader)
    {
        //useTextureRectangle = false;

        static const char *shaderSourceTextureRec = {
            "uniform vec4 cutoff_color;\n"
            "uniform samplerRect movie_texture;\n"
            "void main(void)\n"
            "{\n"
            "    vec4 texture_color = textureRect(movie_texture, gl_TexCoord[0]); \n"
            "    if (all(lessThanEqual(texture_color,cutoff_color))) discard; \n"
            "    gl_FragColor = texture_color;\n"
            "}\n"
        };

        static const char *shaderSourceTexture2D = {
            "uniform vec4 cutoff_color;\n"
            "uniform sampler2D movie_texture;\n"
            "void main(void)\n"
            "{\n"
            "    vec4 texture_color = texture2D(movie_texture, gl_TexCoord[0]); \n"
            "    if (all(lessThanEqual(texture_color,cutoff_color))) discard; \n"
            "    gl_FragColor = texture_color;\n"
            "}\n"
        };

        osg::Program* program = new osg::Program;

        program->addShader(new osg::Shader(osg::Shader::FRAGMENT,
                                           useTextureRectangle ? shaderSourceTextureRec : shaderSourceTexture2D));

        stateset->addUniform(new osg::Uniform("cutoff_color",osg::Vec4(0.1f,0.1f,0.1f,1.0f)));
        stateset->addUniform(new osg::Uniform("movie_texture",0));

        stateset->setAttribute(program);

    }

    osg::Vec3 pos(0.0f,0.0f,0.0f);
    osg::Vec3 topleft = pos;
    osg::Vec3 bottomright = pos;
    
    bool xyPlane = fullscreen;
    
    for(int i=1;i<arguments.argc();++i)
    {
        if (arguments.isString(i))
        {
            osg::Image* image = osgDB::readImageFile(arguments[i]);
            osg::ImageStream* imagestream = dynamic_cast<osg::ImageStream*>(image);
            if (imagestream) imagestream->play();

            if (image)
            {
                geode->addDrawable(myCreateTexturedQuadGeometry(pos,image->s(),image->t(),image, useTextureRectangle, xyPlane, flip));
        
                bottomright = pos + osg::Vec3(static_cast<float>(image->s()),static_cast<float>(image->t()),0.0f);

                pos.y() += image->t()*1.5f;
            }
            else
            {
                std::cout<<"Unable to read file "<<arguments[i]<<std::endl;
            }            
        }
    }
    
    // set the scene to render
    viewer.setSceneData(geode.get());

    if (viewer.getSceneData()==0)
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // pass the model to the MovieEventHandler so it can pick out ImageStream's to manipulate.
    MovieEventHandler* meh = new MovieEventHandler();
    meh->setMouseTracking( mouseTracking );
    meh->set( viewer.getSceneData() );
    viewer.addEventHandler( meh );

    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() ) );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // add the record camera path handler
    viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (fullscreen)
    {
        viewer.realize();

        viewer.getCamera()->setViewMatrix(osg::Matrix::identity());
        viewer.getCamera()->setProjectionMatrixAsOrtho2D(topleft.x(),bottomright.x(),topleft.y(),bottomright.y());

        while(!viewer.done())
        {
            viewer.frame();
        }
        return 0;
    }
    else
    {
        // create the windows and run the threads.
        return viewer.run();
    }
}
