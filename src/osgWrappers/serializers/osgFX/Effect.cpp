#include <osgFX/Effect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkSelectedTechnique( const osgFX::Effect& effect )
{
    return effect.getSelectedTechnique()!=osgFX::Effect::AUTO_DETECT;
}

static bool readSelectedTechnique( osgDB::InputStream& is, osgFX::Effect& effect )
{
    int sel = 0; is >> sel;
    effect.selectTechnique( sel );
    return true;
}

static bool writeSelectedTechnique( osgDB::OutputStream& os, const osgFX::Effect& effect )
{
    os << effect.getSelectedTechnique() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgFX_Effect,
                         /*new osgFX::Effect*/NULL,
                         osgFX::Effect,
                         "osg::Object osg::Node osg::Group osgFX::Effect" )
{
    ADD_BOOL_SERIALIZER( Enabled, true );  // _enabled
    ADD_USER_SERIALIZER( SelectedTechnique );  // _global_sel_tech
}
