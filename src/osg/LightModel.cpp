#include <osg/GL>
#include <osg/LightModel>

using namespace osg;


LightModel::LightModel():
          StateAttribute(),
          _ambient(0.2f,0.2f,0.2f,1.0f),
          _colorControl(LightModel::SINGLE_COLOR),
          _localViewer(false),
          _twoSided(false)
{
}


LightModel::~LightModel()
{
}

void LightModel::apply(State&) const
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,_ambient.ptr());
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,_colorControl);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,_localViewer);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,_twoSided);
}

