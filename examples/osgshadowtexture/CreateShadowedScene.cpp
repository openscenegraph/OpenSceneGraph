#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/Material>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/ShapeDrawable>

#include <osgUtil/CullVisitor>
#include <osgUtil/RenderToTextureStage>

#include "CreateShadowedScene.h"

using namespace osg;

class CreateShadowTextureCullCallback : public osg::NodeCallback
{
    public:
    
        CreateShadowTextureCullCallback(osg::Node* shadower,const osg::Vec3& position, const osg::Vec4& ambientLightColor, unsigned int textureUnit):
            _shadower(shadower),
            _position(position),
            _ambientLightColor(ambientLightColor),
            _unit(textureUnit),
            _shadowState(new osg::StateSet),
            _shadowedState(new osg::StateSet)
        {
            _texture = new osg::Texture2D;
            _texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            _texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
            _texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
            _texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
            _texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        }
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {

            osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cullVisitor && (_texture.valid() && _shadower.valid()))
            {            
                doPreRender(*node,*cullVisitor);

            }
            else
            {
                // must traverse the shadower            
                traverse(node,nv);
            }
        }
        
    protected:
        
        void doPreRender(osg::Node& node, osgUtil::CullVisitor& cv);
        
        osg::ref_ptr<osg::Node>      _shadower;
        osg::ref_ptr<osg::Texture2D> _texture;
        osg::Vec3                    _position;
        osg::Vec4                    _ambientLightColor;
        unsigned int                 _unit;
        osg::ref_ptr<osg::StateSet>  _shadowState;
        osg::ref_ptr<osg::StateSet>  _shadowedState;

};

void CreateShadowTextureCullCallback::doPreRender(osg::Node& node, osgUtil::CullVisitor& cv)
{   

    const osg::BoundingSphere& bs = _shadower->getBound();
    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<_shadower.get()<<std::endl;
        return;
    }
    

    // create the render to texture stage.
    osg::ref_ptr<osgUtil::RenderToTextureStage> rtts = new osgUtil::RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->_stage;

    // set up the background color and clear mask.
    rtts->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    rtts->setClearMask(previous_stage->getClearMask());

    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


    // record the render bin, to be restored after creation
    // of the render to text
    osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

    // set the current renderbin to be the newly created stage.
    cv.setCurrentRenderBin(rtts.get());


    float centerDistance = (_position-bs.center()).length();

    float znear = centerDistance-bs.radius();
    float zfar  = centerDistance+bs.radius();
    float zNearRatio = 0.001f;
    if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
    
        
    // 2:1 aspect ratio as per flag geomtry below.
    float top   = (bs.radius()/centerDistance)*znear;
    float right = top;

    // set up projection.
    osg::RefMatrix* projection = new osg::RefMatrix;
    projection->makeFrustum(-right,right,-top,top,znear,zfar);

    cv.pushProjectionMatrix(projection);

    osg::RefMatrix* matrix = new osg::RefMatrix;
    matrix->makeLookAt(_position,bs.center(),osg::Vec3(0.0f,1.0f,0.0f));


    osg::Matrix MV = cv.getModelViewMatrix();

    // compute the matrix which takes a vertex from local coords into tex coords
    // will use this later to specify osg::TexGen..
    osg::Matrix MVPT = 
                       *matrix * 
                       *projection *
                       osg::Matrix::translate(1.0,1.0,1.0) *
                       osg::Matrix::scale(0.5f,0.5f,0.5f);

    cv.pushModelViewMatrix(matrix);

    // make the material black for a shadow.
    osg::Material* material = new osg::Material;
    material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    material->setEmission(osg::Material::FRONT_AND_BACK,_ambientLightColor);
    material->setShininess(osg::Material::FRONT_AND_BACK,0.0f);
    _shadowState->setAttribute(material,osg::StateAttribute::OVERRIDE);

    cv.pushStateSet(_shadowState.get());

    {

        // traverse the shadower
        _shadower->accept(cv);

    }

    cv.popStateSet();

    // restore the previous model view matrix.
    cv.popModelViewMatrix();


    // restore the previous model view matrix.
    cv.popProjectionMatrix();

    // restore the previous renderbin.
    cv.setCurrentRenderBin(previousRenderBin);

    if (rtts->_renderGraphList.size()==0 && rtts->_bins.size()==0)
    {
        // getting to this point means that all the shadower has been
        // culled by small feature culling or is beyond LOD ranges.
        return;
    }



    int height = 256;
    int width  = 256;


    const osg::Viewport& viewport = *cv.getViewport();

    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
    new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
    rtts->setViewport(new_viewport);
    
    _shadowState->setAttribute(new_viewport);

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->_stage->addToDependencyList(rtts.get());

    // if one exist attach texture to the RenderToTextureStage.
    if (_texture.valid()) rtts->setTexture(_texture.get());


    // set up the stateset to decorate the shadower with the shadow texture
    // with the appropriate tex gen coords.
    TexGen* texgen = new TexGen;
    //texgen->setMatrix(MV);
    texgen->setMode(osg::TexGen::EYE_LINEAR);
    texgen->setPlane(osg::TexGen::S,osg::Plane(MVPT(0,0),MVPT(1,0),MVPT(2,0),MVPT(3,0)));
    texgen->setPlane(osg::TexGen::T,osg::Plane(MVPT(0,1),MVPT(1,1),MVPT(2,1),MVPT(3,1)));
    texgen->setPlane(osg::TexGen::R,osg::Plane(MVPT(0,2),MVPT(1,2),MVPT(2,2),MVPT(3,2)));
    texgen->setPlane(osg::TexGen::Q,osg::Plane(MVPT(0,3),MVPT(1,3),MVPT(2,3),MVPT(3,3)));

    cv.getRenderStage()->addPositionedTextureAttribute(0,new osg::RefMatrix(MV),texgen);


    _shadowedState->setTextureAttributeAndModes(_unit,_texture.get(),osg::StateAttribute::ON);
    _shadowedState->setTextureAttribute(_unit,texgen);
    _shadowedState->setTextureMode(_unit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    _shadowedState->setTextureMode(_unit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    _shadowedState->setTextureMode(_unit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
    _shadowedState->setTextureMode(_unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

    cv.pushStateSet(_shadowedState.get());

        // must traverse the shadower            
        traverse(&node,&cv);

    cv.popStateSet();    

}


// set up a light source with the shadower and shodower subgraphs below it
// with the appropriate callbacks set up.
osg::Group* createShadowedScene(osg::Node* shadower,osg::Node* shadowed,const osg::Vec3& lightPosition,float radius,unsigned int textureUnit)
{
    osg::LightSource* lightgroup = new osg::LightSource;

    osg::Light* light = new osg::Light;
    light->setPosition(osg::Vec4(lightPosition,1.0f));
    light->setLightNum(0);

    lightgroup->setLight(light);
    
    osg::Vec4 ambientLightColor(0.2,0.2f,0.2f,1.0f);

    // add the shadower
    lightgroup->addChild(shadower);

    // add the shadowed with the callback to generate the shadow texture.
    shadowed->setCullCallback(new CreateShadowTextureCullCallback(shadower,lightPosition,ambientLightColor,textureUnit));
    lightgroup->addChild(shadowed);
    
    osg::Geode* lightgeode = new osg::Geode;
    lightgeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    lightgeode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(lightPosition,radius)));
    
    lightgroup->addChild(lightgeode);
    
    return lightgroup;
}

