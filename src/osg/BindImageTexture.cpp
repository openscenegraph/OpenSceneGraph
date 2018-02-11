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
        unsigned int contextID = state.getContextID();
        const BufferData * bd = _target->getBufferData();
        if( (bd && bd->getModifiedCount() != _target->getModifiedCount(contextID)) )
        {
            // _target never been applied yet
            state.applyTextureAttribute(state.getActiveTextureUnit(), _target);
        }
        Texture::TextureObject *to = _target->getTextureObject(contextID);
        state.get<osg::GLExtensions>()->glBindImageTexture(_imageunit, to->id(), _level, _layered, _layer, _access, _format);
    }

}
