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

#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/EventVisitor>

#include <iostream>

class ImageStreamStateCallback : public osg::DrawableUpdateCallback {
public:
    ImageStreamStateCallback(osgText::Text* text, osg::ImageStream* is)
        : osg::DrawableUpdateCallback()
        , _text(text)
        , _imageStream(is)
        , _fps(0)
        , _lastData(NULL)
        , _lastDataTimeStamp(0)
    {
    }

    void setImageStream(osg::ImageStream* is) { _imageStream = is; }

    virtual void update(osg::NodeVisitor* nv, osg::Drawable*)
    {
        if (_text.valid() && _imageStream.valid())
        {
            if (_imageStream->data() != _lastData)
            {
                double dt = nv->getFrameStamp()->getReferenceTime() - _lastDataTimeStamp;

                _fps = 0.9 * _fps + 0.1 * (1 / dt);
                _fps = osg::round(10 * _fps) / 10.0;

                _lastDataTimeStamp = nv->getFrameStamp()->getReferenceTime();
                _lastData = _imageStream->data();
            }

            std::ostringstream ss;
            ss << _imageStream->s() << "x" << _imageStream->t() << " | " << _fps << "fps";
            ss << " | len: " << osg::round(_imageStream->getLength()*10) / 10.0;
            ss << " | cur: " << osg::round(_imageStream->getCurrentTime()*10) / 10.0;
            if (_imageStream->getStatus() == osg::ImageStream::PLAYING)
            {
                ss << " | playing";
            }
            else
            {
                ss << " | paused";
                _fps = 0;
            }
            if (_imageStream->getLoopingMode() == osg::ImageStream::LOOPING) {
                ss << " | looping";
            }
            else
            {
                ss << " | don't loop";
            }
            _text->setText(ss.str());
        }
    }


private:
    osg::observer_ptr<osgText::Text> _text;
    osg::observer_ptr<osg::ImageStream> _imageStream;
    float _fps;
    unsigned char* _lastData;
    double _lastDataTimeStamp;

};

class MovieEventHandler : public osgGA::GUIEventHandler
{
public:

    MovieEventHandler()
        : osgGA::GUIEventHandler()
        , _currentImageStream()
        , _currentGeometry()
    {
    }


    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv);

    virtual void getUsage(osg::ApplicationUsage& usage) const;

protected:

    void setColor(osg::Geometry* geo, const osg::Vec4& color)
    {
        if (!geo)
            return;

        osg::Vec4Array* c = dynamic_cast<osg::Vec4Array*>(geo->getColorArray());
        if (c) (*c)[0] = color;
        geo->dirtyGLObjects();
        c->dirty();
    }

    virtual ~MovieEventHandler() {}

    osg::observer_ptr<osg::ImageStream> _currentImageStream;
    osg::observer_ptr<osg::Geometry> _currentGeometry;

};


bool MovieEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* /*nv*/)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        {
            if(_currentImageStream.valid() && (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT))
            {
                float scalar = (ea.getXnormalized()+1) / 2.0;
                _currentImageStream->seek(scalar * _currentImageStream->getLength());
            }
        }
        break;

        case(osgGA::GUIEventAdapter::RELEASE):
        {

            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            osgUtil::LineSegmentIntersector::Intersections intersections;
            bool foundIntersection = view==0 ? false : view->computeIntersections(ea, intersections);

            if (foundIntersection)
            {
                // use the nearest intersection
                const osgUtil::LineSegmentIntersector::Intersection& intersection = *(intersections.begin());
                osg::Drawable* drawable = intersection.drawable.get();
                osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;

                if (geometry) {
                    osg::Texture* tex = geometry->getStateSet() ? dynamic_cast<osg::Texture*>(geometry->getStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE)) : NULL;
                    if (tex) {
                        osg::ImageStream* is = dynamic_cast<osg::ImageStream*>(tex->getImage(0));
                        if (is)
                        {
                            setColor(_currentGeometry.get(), osg::Vec4(0.7, 0.7, 0.7, 1.0));
                            _currentGeometry = geometry;
                            setColor(_currentGeometry.get(), osg::Vec4(1,1,1,1));
                            _currentImageStream = is;

                            if (is->getStatus() == osg::ImageStream::PLAYING)
                            {
                                is->pause();
                            }
                            else
                            {
                                is->play();
                            }

                        }
                    }
                }
            }

            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (!_currentImageStream.valid())
                return false;

            if (ea.getKey()=='p')
            {
                osg::ImageStream::StreamStatus playToggle = _currentImageStream->getStatus();
                if (playToggle != osg::ImageStream::PLAYING)
                {
                    std::cout<< _currentImageStream.get() << " Play"<<std::endl;
                    _currentImageStream->play();
                }
                else
                {
                    // playing, so pause
                    std::cout<< _currentImageStream.get() << " Pause"<<std::endl;
                    _currentImageStream->pause();
                }
                return true;
            }
            else if (ea.getKey()=='r')
            {
                std::cout<< _currentImageStream.get() << " Restart"<<std::endl;
                _currentImageStream->rewind();
                _currentImageStream->play();
                return true;
            }
            else if (ea.getKey()=='>')
            {
                std::cout << _currentImageStream.get() << " Seeking"<<std::endl;
                _currentImageStream->seek(_currentImageStream->getCurrentTime() + 1.0);

                return true;
            }
            else if (ea.getKey()=='L')
            {
                if ( _currentImageStream->getLoopingMode() == osg::ImageStream::LOOPING)
                {
                    std::cout<< _currentImageStream.get() << " Toggle Looping Off"<<std::endl;
                    _currentImageStream->setLoopingMode( osg::ImageStream::NO_LOOPING );
                }
                else
                {
                    std::cout<< _currentImageStream.get() << " Toggle Looping On"<<std::endl;
                    _currentImageStream->setLoopingMode( osg::ImageStream::LOOPING );
                }
                return true;
            }
            else if (ea.getKey()=='+')
            {
                double tm = _currentImageStream->getTimeMultiplier();
                tm += 0.1;
                _currentImageStream->setTimeMultiplier(tm);
                std::cout << _currentImageStream.get() << " Increase speed rate "<< _currentImageStream->getTimeMultiplier() << std::endl;

                return true;
            }
            else if (ea.getKey()=='-')
            {
                double tm = _currentImageStream->getTimeMultiplier();
                tm -= 0.1;
                _currentImageStream->setTimeMultiplier(tm);
                std::cout << _currentImageStream.get() << " Decrease speed rate "<< _currentImageStream->getTimeMultiplier() << std::endl;

                return true;
            }
            else if (ea.getKey()=='o')
            {
                std::cout<< _currentImageStream.get() << " Frame rate  "<< _currentImageStream->getFrameRate() <<std::endl;

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
    usage.addKeyboardMouseBinding("p","Play/Pause current movie");
    usage.addKeyboardMouseBinding("r","Restart current movie");
    usage.addKeyboardMouseBinding("l","Toggle looping of current movie");
    usage.addKeyboardMouseBinding("+","Increase speed of current movie");
    usage.addKeyboardMouseBinding("-","Decrease speed of current movie");
    usage.addKeyboardMouseBinding("o","Display frame rate of current movie");
    usage.addKeyboardMouseBinding(">","Advance the current movie using seek");
}


static osgDB::DirectoryContents getSuitableFiles(osg::ArgumentParser& arguments)
{
    osgDB::DirectoryContents files;
    for(int i=1; i<arguments.argc(); ++i)
    {
        if (arguments.isOption(i))
            continue;

        if (osgDB::fileType(arguments[i]) == osgDB::DIRECTORY)
        {
            const std::string& directory = arguments[i];
            osgDB::DirectoryContents dc = osgDB::getSortedDirectoryContents(directory);

            for(osgDB::DirectoryContents::iterator itr = dc.begin(); itr != dc.end(); ++itr)
            {
                std::string full_file_name = directory + "/" + (*itr);
                if (osgDB::fileType(full_file_name) != osgDB::DIRECTORY)
                {
                    files.push_back(full_file_name);
                }
            }
        }
        else {
            files.push_back(arguments[i]);
        }
    }
    return files;
}


class MyDimensionsChangedCallback : public osg::Image::DimensionsChangedCallback {
public:
    MyDimensionsChangedCallback(osg::Texture* tex, osg::Geometry* geo)
        : osg::Image::DimensionsChangedCallback()
        , _tex(tex)
        , _geo(geo)
    {
    }

    virtual void operator()(osg::Image* img)
    {
        if (img && _tex.valid() && _geo.valid())
        {
            float l(0), t(0);
            float r = (_tex->getTextureTarget() == GL_TEXTURE_2D) ? 1 : img->s();
            float b = (_tex->getTextureTarget() == GL_TEXTURE_2D) ? 1 : img->t();

            /*
            if (img->getOrigin() == osg::Image::TOP_LEFT)
                std::swap(t, b);
            */

            osg::Vec2Array* tex_coords = dynamic_cast<osg::Vec2Array*>(_geo->getTexCoordArray(0));
            if (tex_coords) {

                (*tex_coords)[0].set(l,t);
                (*tex_coords)[1].set(l,b);
                (*tex_coords)[2].set(r,b);
                (*tex_coords)[3].set(r,t);
                tex_coords->dirty();
                _geo->dirtyGLObjects();
            }
        }
    }

private:
    osg::observer_ptr<osg::Texture> _tex;
    osg::observer_ptr<osg::Geometry> _geo;
};

static osg::Node* readImageStream(const std::string& file_name, osg::Vec3& p, float desired_height, osgDB::Options* options)
{
    osg::ref_ptr<osg::Object> obj = osgDB::readRefObjectFile(file_name, options);
    osg::ref_ptr<osg::Texture> tex = dynamic_cast<osg::Texture*>(obj.get());
    osg::Geometry* geo(NULL);
    float w(0);

    if (!tex)
    {
        osg::ref_ptr<osg::ImageStream> img_stream = dynamic_cast<osg::ImageStream*>(obj.get());

        // try readImageFile if readObjectFile failed
        if (!img_stream)
        {
            img_stream = osgDB::readRefFile<osg::ImageStream>(file_name, options);
        }

        if (img_stream)
        {
            tex = new osg::Texture2D(img_stream.get());
            tex->setResizeNonPowerOfTwoHint(false);

        }
    }

    // create textured quad
    if(tex)
    {
        osg::Geode* geode = new osg::Geode();

        osg::ref_ptr<osg::ImageStream> img = dynamic_cast<osg::ImageStream*>(tex->getImage(0));
        if (img)
        {
            w = (img->t() > 0) ? img->s() * desired_height / img->t() : 0;

            osgText::Text* text = new osgText::Text();
            text->setFont("arial.ttf");
            text->setDataVariance(osg::Object::DYNAMIC);
            text->setUpdateCallback(new ImageStreamStateCallback(text, img.get()));
            text->setCharacterSize(24);
            text->setPosition(p + osg::Vec3(10,-10,10));
            text->setAxisAlignment(osgText::TextBase::XZ_PLANE);
            geode->addDrawable (text);
        }

        if (w == 0)
        {
            // hmm, imagestream with no width?
            w = desired_height * 16 / 9.0f;
        }
        float tex_s = (tex->getTextureTarget() == GL_TEXTURE_2D) ? 1 : img->s();
        float tex_t = (tex->getTextureTarget() == GL_TEXTURE_2D) ? 1 : img->t();

        if (img->getOrigin() == osg::Image::TOP_LEFT)
            geo = osg::createTexturedQuadGeometry(p, osg::Vec3(w, 0, 0), osg::Vec3(0, 0, desired_height), 0, tex_t, tex_s, 0);
        else
            geo = osg::createTexturedQuadGeometry(p, osg::Vec3(w, 0, 0), osg::Vec3(0, 0, desired_height), 0, 0, tex_s, tex_t);

        geode->addDrawable(geo);

        geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex.get());

        osg::Vec4Array* colors = new osg::Vec4Array();
        colors->push_back(osg::Vec4(0.7, 0.7, 0.7, 1));

        geo->setColorArray(colors, osg::Array::BIND_OVERALL);

        p[0] += w + 10;

        img->addDimensionsChangedCallback(new MyDimensionsChangedCallback(tex.get(), geo));

        return geode;
    }
    else
    {
        std::cout << "could not read file from " << file_name << std::endl;
        return NULL;
    }

    return NULL;
}


class ReplaceTextureVisitor : public osg::NodeVisitor {
public:
    ReplaceTextureVisitor(osg::Texture* tex)
        : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
        , _tex(tex)
    {
    }

    virtual void apply(osg::Geode& geode)
    {
        apply(geode.getStateSet());
        for(unsigned int i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Drawable* drawable = geode.getDrawable(i);

            apply(drawable->getStateSet());
            ImageStreamStateCallback* cb = dynamic_cast<ImageStreamStateCallback*>(drawable->getUpdateCallback());
            if (cb)
                cb->setImageStream(dynamic_cast<osg::ImageStream*>(_tex->getImage(0)));

        }

        osg::NodeVisitor::apply(geode);
    }

    void apply(osg::StateSet* ss)
    {
        if (ss && ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE))
            ss->setTextureAttribute(0, _tex.get());
    }
private:
    osg::ref_ptr<osg::Texture> _tex;
};


class SlideShowEventHandler : public osgGA::GUIEventHandler {
public:
    SlideShowEventHandler(osg::Node* node, const osgDB::DirectoryContents& files,osgDB::ReaderWriter::Options* options)
        : osgGA::GUIEventHandler()
        , _node(node)
        , _files(files)
        , _options(options)
        , _currentFile(-1)
    {
        loadSlide(_currentFile);
    }

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& /*aa*/, osg::Object*, osg::NodeVisitor* /*nv*/)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left)
                        loadSlide(_currentFile - 1);
                    else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right)
                        loadSlide(_currentFile + 1);
                    else
                        return false;

                    return true;
                }
                break;
            default:
                break;
        }

        return false;
    }

private:

    void loadSlide(int new_ndx)
    {
        if (new_ndx == _currentFile)
            return;

        _currentFile = new_ndx;
        if (_currentFile < 0)
            _currentFile = _files.size() - 1;
        else if (_currentFile >= static_cast<int>(_files.size()))
            _currentFile = 0;

        osg::ref_ptr<osg::Object> obj = osgDB::readRefObjectFile(_files[_currentFile], _options.get());
        osg::ref_ptr<osg::Texture> tex = dynamic_cast<osg::Texture*>(obj.get());
        if (!tex) {
            osg::ref_ptr<osg::ImageStream> stream = dynamic_cast<osg::ImageStream*>(obj.get());
            if (!stream)
            {
                stream = osgDB::readRefFile<osg::ImageStream>(_files[_currentFile], _options.get());
            }

            if (stream)
            {
                tex = new osg::Texture2D(stream.get());
                tex->setResizeNonPowerOfTwoHint(false);
            }
        }
        if (tex) {
            osg::ref_ptr<osg::ImageStream> stream = dynamic_cast<osg::ImageStream*>(tex->getImage(0));
            if (stream)
                stream->play();
            ReplaceTextureVisitor v(tex.get());
            _node->accept(v);
        }
    }

    osg::ref_ptr<osg::Node> _node;
    osgDB::DirectoryContents _files;
    osg::ref_ptr<osgDB::ReaderWriter::Options> _options;
    int _currentFile;
};


int main(int argc, char** argv)
{
    /*
    std::string plugin_to_use = "AVFoundation"; //  "QTKit";

    osgDB::Registry::instance()->addFileExtensionAlias("mov", plugin_to_use);
    osgDB::Registry::instance()->addFileExtensionAlias("mp4", plugin_to_use);
    osgDB::Registry::instance()->addFileExtensionAlias("m4v", plugin_to_use);
    */

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example demonstrates the use of ImageStream for rendering movies as textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--disableCoreVideo","disable CoreVideo (QTKit+AVFoundation plugin)");
    arguments.getApplicationUsage()->addCommandLineOption("--disableMultiThreadedFrameDispatching","disable frame dispatching via multiple threads (QTKit+AVFoundation plugin)");
    arguments.getApplicationUsage()->addCommandLineOption("--maxVideos [numVideos]","max videos to open from a folder");
    arguments.getApplicationUsage()->addCommandLineOption("--slideShow","present movies in a slide-show");

    unsigned int max_videos(10);
    bool slide_show = false;
    std::string options_str("");

    if (arguments.find("--slideShow") > 0) {
        slide_show = true;
    }

    if (arguments.find("--disableMultiThreadedFrameDispatching") > 0) {
        options_str += " disableMultiThreadedFrameDispatching";
    }

    if (arguments.find("--disableCoreVideo") > 0) {
        options_str += " disableCoreVideo";
    }

    if (int ndx = arguments.find("--numFrameDispatchThreads") > 0)
    {
        options_str += std::string(" numFrameDispatchThreads=") + arguments[ndx+1];
    }
    if (int ndx = arguments.find("--maxVideos") > 0)
    {
        if (arguments.isNumber(ndx+1)) max_videos =  atoi(arguments[ndx+1]);
    }


    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }


    osg::ref_ptr<osg::Group> group = new osg::Group;

    osg::StateSet* stateset = group->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);


    osg::Vec3 pos(0.0f,0.0f,0.0f);
    static const float desired_height = 768.0f;

    osgDB::DirectoryContents files = getSuitableFiles(arguments);
    osgGA::GUIEventHandler* movie_event_handler(NULL);

    osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options(options_str);

    if (slide_show)
    {
        osg::Node* node = readImageStream(files[0], pos, desired_height, options.get());
        group->addChild(node);
        movie_event_handler = new SlideShowEventHandler(node, files, options.get());
    }
    else
    {
        movie_event_handler = new MovieEventHandler();

        unsigned int num_files_per_row = std::max(osg::round(sqrt(static_cast<double>(std::min(max_videos, static_cast<unsigned int>(files.size()))))), 1.0);
        static const float new_row_at = num_files_per_row * desired_height * 16 / 9.0;

        unsigned int num_videos = 0;
        for(osgDB::DirectoryContents::iterator i = files.begin(); (i != files.end()) && (num_videos < max_videos); ++i)
        {
            osg::Node* node = readImageStream(*i, pos, desired_height, options.get());
            if (node)
                group->addChild(node);

            if (pos[0] > new_row_at)
            {
                pos[0] = 0;
                pos[2] += desired_height +10;
            }
            num_videos++;
        }
    }


    // set the scene to render
    viewer.setSceneData(group.get());

    if (viewer.getSceneData()==0)
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }


    viewer.addEventHandler( movie_event_handler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ToggleSyncToVBlankHandler());
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

    // create the windows and run the threads.
    return viewer.run();
}

