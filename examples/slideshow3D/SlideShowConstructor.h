#ifndef SLIDESHOWCONSTUCTOR
#define SLIDESHOWCONSTRUCTOR

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Group>
#include <osg/ClearNode>
#include <osg/Switch>

class SlideShowConstructor
{
public:

    SlideShowConstructor();
    
    void createPresentation();
    
    void setBackgroundColor(const osg::Vec4& color);
    
    void setTextColor(const osg::Vec4& color);
    
    void setPresentationName(const std::string& name);
    
    void addSlide();
    
    void setSlideTitle(const std::string& name) { _slideTitle = name; }

    void setSlideBackground(const std::string& name) { _slideBackgroundImageFileName = name; }
    
    void addLayer();
    
    void addBullet(const std::string& bullet);
    
    void addParagraph(const std::string& paragraph);
    
    void addImage(const std::string& filename,float height);
    
    void addModel(const std::string& filename,float scale,float rotation,float position);
    
    osg::ClearNode* takePresentation() { return _root.release(); }
    
    osg::ClearNode* getPresentation() { return _root.get(); }

    osg::Switch* getPresentationSwitch() { return _presentationSwitch.get(); }

    osg::Switch* getCurrentSlide() { return _slide.get(); }
    
    osg::Group* getCurrentLayer() { return _currentLayer.get(); }

protected:

    osg::Vec3   _slideOrigin;
    float       _slideWidth;
    float       _slideHeight;

    osg::Vec4   _backgroundColor;
    osg::Vec4   _textColor;
    std::string _textFont;
    float       _titleHeight;
    float       _textHeight;
    std::string _presentationName;


    osg::Vec3   _titleOrigin;
    osg::Vec3   _textOrigin;
    osg::Vec3   _imageOrigin;
    osg::Vec3   _modelLeft;
    osg::Vec3   _modelRight;
    
    osg::Vec3 _textCursor;
    osg::Vec3 _imageCursor;
    osg::Vec3 _modelCursor;
    
    osg::ref_ptr<osg::ClearNode>    _root;
    osg::ref_ptr<osg::Switch>       _presentationSwitch;
    
    osg::ref_ptr<osg::Switch>       _slide;
    std::string                     _slideTitle;
    std::string                     _slideBackgroundImageFileName;
    
    osg::ref_ptr<osg::Group>        _previousLayer;
    osg::ref_ptr<osg::Group>        _currentLayer;
    
};

#endif
