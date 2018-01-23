#include <osg/BindImageTexture>

using namespace osg;

int BindImageTexture::compare(const osg::StateAttribute &sa) const
{
    COMPARE_StateAttribute_Types(BindImageTexture,sa)
    // Compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_target)
    COMPARE_StateAttribute_Parameter(_imageunit)
    COMPARE_StateAttribute_Parameter(_access)
    COMPARE_StateAttribute_Parameter(_format)
    COMPARE_StateAttribute_Parameter(_layered)
    COMPARE_StateAttribute_Parameter(_level)
    return 0;
}

void BindImageTexture::apply(osg::State&state) const
{
    if(_target.valid())
    {
        Texture::TextureObject *to = _target->getTextureObject( state.getContextID() );
        GLBufferObject *globj = _target->getBufferData()->getBufferObject()->getGLBufferObject( state.getContextID() );
        if( !globj || globj->isDirty() )
        {
            // _target never been applied yet
            _target->apply(state);
            to = _target->getTextureObject( state.getContextID() );
        }
        state.get<osg::GLExtensions>()->glBindImageTexture(_imageunit, to->id(), _level, _layered, _layer, _access, _format);
    }

}
