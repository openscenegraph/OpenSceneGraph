// Pool.cpp

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/Vec4>
#include <osg/Texture2D>
#include <osg/TexEnv>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include "MaterialPaletteRecord.h"
#include "OldMaterialPaletteRecord.h"
#include "Pool.h"
#include "Registry.h"

#include <stdio.h>


using namespace flt;


osg::Vec4 ColorPool::getColor(int nColorIntensity)
{
    // nColorIntensity:
    // bit 0-6:  intensity
    // bit 7-15  color index

    osg::Vec4 col(1,1,1,1);

    if (nColorIntensity >= 0)
    {
        ColorName* cn = getColorName(nColorIntensity >> 7);
        if (cn)
            col = cn->getColor();

        float intensity = (float)(nColorIntensity & 0x7f)/127.f;
        col[0] *= intensity;
        col[1] *= intensity;
        col[2] *= intensity;
    }

    return col;
}


// getColor for version 11, 12 & 13.
osg::Vec4 ColorPool::getOldColor(int nColorIntensity)
{
    // nColorIntensity:
    // bit 0-6:  intensity
    // bit 7-11  color index
    // bit 12    fixed intensity bit

    osg::Vec4 col(1,1,1,1);

    if (nColorIntensity >= 0)
    {
        int nIndex;
        bool bFixedIntensity = (nColorIntensity & 0x1000) ? true : false;

        if (bFixedIntensity)
            nIndex = (nColorIntensity & 0x0fff)+(4096>>7);
        else
            nIndex = nColorIntensity >> 7;

        ColorName* cn = getColorName(nIndex);
        if (cn)
            col = cn->getColor();

        // intensity
        if (!bFixedIntensity)
        {
            float intensity = (float)(nColorIntensity & 0x7f)/127.f;
            col[0] *= intensity;
            col[1] *= intensity;
            col[2] *= intensity;
        }
    }

    return col;
}


void ColorPool::addColor(int nIndex, const osg::Vec4& color)
{
    if (nIndex >= 0)
    {
        ColorName* colorname = new ColorName;
        colorname->setColor(color);

        _colorNameMap[nIndex] = colorname;
    }
}


ColorPool::ColorName* ColorPool::getColorName(int nIndex)
{
    ColorNameMap::iterator itr = _colorNameMap.find(nIndex);
    if (itr != _colorNameMap.end())
        return (*itr).second.get();

    return NULL;
}


////////////////////////////////////////////////////////////////////


osg::StateSet* TexturePool::getTexture(int nIndex, int fltVersion)
{
    TexturePaletteMap::iterator fitr = _textureMap.find(nIndex);
    if (fitr != _textureMap.end())
    {
        return (*fitr).second.get();
    }
    else
    {
        // no existing texture state set set up so lets look
        // for a file name for this nIndex..
        TextureNameMap::iterator nitr = _textureNameMap.find(nIndex);
        if (nitr != _textureNameMap.end())
        {
            const std::string& textureName = (*nitr).second;

            // Valid index, find the texture
            // Get StateSet containing texture from registry pool.
            osg::StateSet* textureStateSet = Registry::instance()->getTexture(textureName);

            if (textureStateSet)
            {
                // Add texture to local pool to be ab121le to get by index.
                addTexture(nIndex, textureStateSet);
            }
            else
            {
                CERR<<"setTexture attempting to load ("<<textureName<<")"<<std::endl;

                unsigned int unit = 0;

                // Read texture and attribute file
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                if (image.valid())
                {
                    std::string attrName(textureName);
                    attrName += ".attr";

                    // Read attribute file
                    char options[256];
                    sprintf(options,"FLT_VER %d",fltVersion);

                    osgDB::Registry::instance()->setOptions(new osgDB::ReaderWriter::Options(options));
                    textureStateSet = dynamic_cast<osg::StateSet*>(osgDB::readObjectFile(attrName));
                    osgDB::Registry::instance()->setOptions(NULL);      // Delete options

                    // if not found create default StateSet
                    if (textureStateSet == NULL)
                    {
                        textureStateSet = new osg::StateSet;

                        osg::Texture2D* osgTexture = new osg::Texture2D;
                        osgTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
                        osgTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
                        textureStateSet->setTextureAttributeAndModes( unit, osgTexture,osg::StateAttribute::ON);

                        osg::TexEnv* osgTexEnv = new osg::TexEnv;
                        osgTexEnv->setMode(osg::TexEnv::MODULATE);
                        textureStateSet->setTextureAttribute( unit, osgTexEnv );
                    }

                    osg::Texture2D *osgTexture = dynamic_cast<osg::Texture2D*>(textureStateSet->getTextureAttribute( unit, osg::StateAttribute::TEXTURE));
                    if (osgTexture == NULL)
                    {
                        osgTexture = new osg::Texture2D;
                        textureStateSet->setTextureAttributeAndModes( unit, osgTexture,osg::StateAttribute::ON);
                    }

                    osgTexture->setImage(image.get());

                }
                else
                {
                    // invalid image file, register an empty state set 
                    textureStateSet = new osg::StateSet;
                }

                // Add new texture to registry pool
                // ( umm... should this have reference to the texture unit? RO. July2002)
                Registry::instance()->addTexture(textureName, textureStateSet);

                // Also add to local pool to be able to get texture by index.
                // ( umm... should this have reference to the texture unit? RO. July2002)
                addTexture(nIndex, textureStateSet);

                CERR<<"Registry::instance()->addTexture("<<textureName<<", "<<textureStateSet<<")"<<std::endl;
                CERR<<"pTexturePool->addTexture("<<nIndex<<", "<<textureStateSet<<")"<<std::endl;
            }
            
            return textureStateSet;
        }
    }
    return NULL;
}

std::string* TexturePool::getTextureName(int nIndex)
{
    TextureNameMap::iterator fitr = _textureNameMap.find(nIndex);
    if (fitr != _textureNameMap.end())
        return &(*fitr).second;
    else
        return NULL;
}


void TexturePool::addTexture(int nIndex, osg::StateSet* stateset)
{
    _textureMap[nIndex] = stateset;
}

void TexturePool::addTextureName(int nIndex, const std::string& name)
{
    _textureNameMap[nIndex] = name;
}

////////////////////////////////////////////////////////////////////


MaterialPool::PoolMaterial* MaterialPool::getMaterial(int nIndex)
{
    if (nIndex < 0) return NULL;
    MaterialMap::iterator fitr = _MaterialMap.find(nIndex);
    if (fitr != _MaterialMap.end())
        return (*fitr).second.get();

    return NULL;
}


void MaterialPool::addMaterial(int nIndex, PoolMaterial* material)
{
    _MaterialMap[nIndex] = material;
}

osg::Group* InstancePool::getInstance(int nIndex)
{
    InstanceMap::iterator fitr = _instanceMap.find(nIndex);
    if (fitr != _instanceMap.end())
        return (*fitr).second.get();
    else
        return NULL;
}


void InstancePool::addInstance(int nIndex, osg::Group* instance)
{
    _instanceMap[nIndex] = instance;
}

