#ifndef SLIDESHOWCONSTUCTOR
#define SLIDESHOWCONSTRUCTOR

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Group>
#include <osg/ClearNode>
#include <osg/Switch>
#include <osgText/Text>

class SlideShowConstructor
{
public:

    SlideShowConstructor();
    
    struct HomePosition : public osg::Referenced
    {
        HomePosition() {}

        HomePosition(const osg::Vec3& in_eye, const osg::Vec3& in_center, const osg::Vec3& in_up):
            eye(in_eye),
            center(in_center),
            up(in_up) {}
    
        osg::Vec3   eye;
        osg::Vec3   center;
        osg::Vec3   up;
    };
    
    struct Duration : public osg::Referenced
    {
        Duration(double in_duration):duration(in_duration) {}
        
        double duration;
    };
    
    void createPresentation();
    
    void setBackgroundColor(const osg::Vec4& color);
    
    void setTextColor(const osg::Vec4& color);
    
    void setPresentationName(const std::string& name);
    
    void setPresentationAspectRatio(float aspectRatio);

    void setPresentationAspectRatio(const std::string& str);
    
    void setPresentationDuration(double duration);


    void addSlide();
    
    void setSlideTitle(const std::string& name) { _slideTitle = name; }

    void setSlideBackground(const std::string& name) { _slideBackgroundImageFileName = name; }
    
    void setSlideDuration(double duration);


    void addLayer();
    
    void setLayerDuration(double duration);

    void addBullet(const std::string& bullet);
    
    void addParagraph(const std::string& paragraph);
    
    void addImage(const std::string& filename,float height);
    
    void addStereoImagePair(const std::string& filenameLeft,const std::string& filenameRight,float height);
    
    enum CoordinateFrame { SLIDE, MODEL };

    void addModel(const std::string& filename, CoordinateFrame coordinate_frame, const osg::Vec3& position, float scale, const osg::Vec4& rotate, const osg::Vec4& rotation);
    void addModelWithPath(const std::string& filename, CoordinateFrame coordinate_frame, const osg::Vec3& position, float scale, const osg::Vec4& rotate, const std::string& animation_path);
    void addModelWithCameraPath(const std::string& filename, CoordinateFrame coordinate_frame, const osg::Vec3& position, float scale, const osg::Vec4& rotate, const std::string& animation_path);
    
    osg::ClearNode* takePresentation() { return _root.release(); }
    
    osg::ClearNode* getPresentation() { return _root.get(); }

    osg::Switch* getPresentationSwitch() { return _presentationSwitch.get(); }

    osg::Switch* getCurrentSlide() { return _slide.get(); }
    
    osg::Group* getCurrentLayer() { return _currentLayer.get(); }

protected:

    osg::Vec3   _slideOrigin;
    float       _slideWidth;
    float       _slideHeight;
    float       _slideDistance;

    osg::Vec4   _backgroundColor;
    osg::Vec4   _textColor;
    std::string _textFont;
    float       _titleHeight;
    float       _titleWidth;
    float       _textHeight;
    float       _textWidth;
    std::string _presentationName;
    double       _presentationDuration;

    osg::Vec3   _titlePositionRatios;
    osgText::Text::AlignmentType _titleAlignment;
    osg::Vec3   _titleOrigin;

    osg::Vec3   _textOrigin;
    osg::Vec3   _imageOrigin;
    
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
