
#include <osg/Referenced>
#include <osg/Node>
#include <osg/Camera>
#include <osg/TextureRectangle>
#include <osg/Texture2D>
#include <osgGA/GUIEventHandler>

#include <limits>

#ifndef DEPTHPEELING_H
#define DEPTHPEELING_H

// Some choices for the kind of textures we can use ...
#define USE_TEXTURE_RECTANGLE
//#define USE_NON_POWER_OF_TWO_TEXTURE
#define USE_PACKED_DEPTH_STENCIL

template<typename T>
inline T
nextPowerOfTwo(T k)
{
    if (k == T(0))
        return 1;
    k--;
    for (int i = 1; i < std::numeric_limits<T>::digits; i <<= 1)
        k = k | k >> i;
    return k + 1;
}

class DepthPeeling : public osg::Referenced {
public:

    DepthPeeling(unsigned int width, unsigned int height);
    void setSolidScene(osg::Node* scene);
    void setTransparentScene(osg::Node* scene);
    osg::Node* getRoot();
    void resize(int width, int height);
    void setNumPasses(unsigned int numPasses);
    unsigned int getNumPasses() const;
    void setTexUnit(unsigned int texUnit);
    void setShowAllLayers(bool showAllLayers);
    bool getShowAllLayers() const;
    void setOffsetValue(unsigned int offsetValue);
    unsigned int getOffsetValue() const;

    static const char *PeelingShader; /* use this to support depth peeling in GLSL shaders in transparent objects */

    class EventHandler : public osgGA::GUIEventHandler {
    public:
        EventHandler(DepthPeeling* depthPeeling);

        /** Handle events, return true if handled, false otherwise. */
        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*);

    protected:
        osg::ref_ptr<DepthPeeling> _depthPeeling;
    };

protected:
    osg::Node *createQuad(unsigned int layerNumber, unsigned int numTiles);
    void createPeeling();

    class CullCallback : public osg::NodeCallback {
    public:
        CullCallback(unsigned int texUnit, unsigned int offsetValue);
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

    private:
        unsigned int _texUnit;
        unsigned int _offsetValue;
    };

    unsigned int _numPasses;
    unsigned int _texUnit;
    unsigned int _texWidth;
    unsigned int _texHeight;
    bool _showAllLayers;
    unsigned int _offsetValue;

    // The root node that is handed over to the viewer
    osg::ref_ptr<osg::Group> _root;

    // The scene that is displayed
    osg::ref_ptr<osg::Group> _solidscene;
    osg::ref_ptr<osg::Group> _transparentscene;

    // The final camera that composites the pre rendered textures to the final picture
    osg::ref_ptr<osg::Camera> _compositeCamera;

#ifdef USE_TEXTURE_RECTANGLE
    std::vector<osg::ref_ptr<osg::TextureRectangle> > _depthTextures;
    std::vector<osg::ref_ptr<osg::TextureRectangle> > _colorTextures;
#else
    std::vector<osg::ref_ptr<osg::Texture2D> > _depthTextures;
    std::vector<osg::ref_ptr<osg::Texture2D> > _colorTextures;
#endif
};

#endif // #ifndef DEPTHPEELING_H
