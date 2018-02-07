#include <osg/BindImageTexture>

using namespace osg;

int BindImageTexture::compare(const osg::StateAttribute &sa) const
{
    COMPARE_StateAttribute_Types(BindImageTexture,sa)
    // Compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_target)
    COMPARE_StateAttribute_Parameter(_targetTextureUnit)
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
        osg::Texture::TextureObject *textureObject = _target->getTextureObject( state.getContextID() );
        const osg::BufferData * bd = _target->getBufferData();

        if( !textureObject || (bd && bd->getModifiedCount() != _target->getModifiedCount(state.getContextID())))
        {
            state.applyTextureAttribute(_targetTextureUnit,_target);
            textureObject = _target->getTextureObject( state.getContextID() );
        }
        ///TODO add this to state to avoid unecessary call to bindimagetexture
        /// (in practice we change _access all the time... so not a big deal)
        state.get<osg::GLExtensions>()->glBindImageTexture(_imageunit, textureObject->id(), _level, _layered, _layer, _access, _format);
   }

}
