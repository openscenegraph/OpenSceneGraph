#include "osg/Material"
#include "osg/Input"
#include "osg/Output"
#include "osg/Notify"

using namespace osg;

Material::Material( void )
{
    _colorMode = OFF;

    _ambientFrontAndBack = true;
    _ambientFront.set(0.2f, 0.2f, 0.2f, 1.0f);
    _ambientBack.set(0.2f, 0.2f, 0.2f, 1.0f);

    _diffuseFrontAndBack = true;
    _diffuseFront.set(0.8f, 0.8f, 0.8f, 1.0f);
    _diffuseBack.set(0.8f, 0.8f, 0.8f, 1.0f);

    _specularFrontAndBack = true;
    _specularFront.set(0.0f, 0.0f, 0.0f, 1.0f);
    _specularBack.set(0.0f, 0.0f, 0.0f, 1.0f);

    _emissionFrontAndBack = true;
    _emissionFront.set(0.0f, 0.0f, 0.0f, 1.0f);
    _emissionBack.set(0.0f, 0.0f, 0.0f, 1.0f);

    _shininessFrontAndBack = true;
    _shininessFront = 0.0f;
    _shininessBack = 0.0f;
}


Material::~Material( void )
{
}


Material* Material::instance()
{
    static ref_ptr<Material> s_Material(new Material);
    return s_Material.get();
}

void Material::setAmbient( MaterialFace face, const Vec4& ambient )
{
    switch(face) {
    case(FACE_FRONT):
        _ambientFrontAndBack = false;
        _ambientFront = ambient;
        break;    
    case(FACE_BACK):
        _ambientFrontAndBack = false;
        _ambientBack = ambient;
        break;
    case(FACE_FRONT_AND_BACK):
        _ambientFrontAndBack = true;
        _ambientFront = ambient;
        _ambientBack = ambient;
        break;
    default:
        notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::setAmbient()."<<endl;
    }
}

const Vec4& Material::getAmbient(MaterialFace face) const
{
    switch(face) {
    case(FACE_FRONT):
        return _ambientFront;
    case(FACE_BACK):
        return _ambientBack;
    case(FACE_FRONT_AND_BACK):
        if (!_ambientFrontAndBack)
        {
             notify(NOTICE)<<"Notice: Material::getAmbient(FRONT_AND_BACK) called on material "<<endl;
             notify(NOTICE)<<"        with seperate FRONT and BACK ambient colors."<<endl;
        }
        return _ambientFront;
    }
    notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::getAmbient()."<<endl;
    return _ambientFront;
}

void Material::setDiffuse( MaterialFace face, const Vec4& diffuse )
{
    switch(face) {
    case(FACE_FRONT):
        _diffuseFrontAndBack = false;
        _diffuseFront = diffuse;
        break;    
    case(FACE_BACK):
        _diffuseFrontAndBack = false;
        _diffuseBack = diffuse;
        break;
    case(FACE_FRONT_AND_BACK):
        _diffuseFrontAndBack = true;
        _diffuseFront = diffuse;
        _diffuseBack = diffuse;
        break;
    default:
        notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::setDiffuse()."<<endl;
        break;
    }
}

const Vec4& Material::getDiffuse(MaterialFace face) const
{
    switch(face) {
    case(FACE_FRONT):
        return _diffuseFront;
    case(FACE_BACK):
        return _diffuseBack;
    case(FACE_FRONT_AND_BACK):
        if (!_diffuseFrontAndBack)
        {
             notify(NOTICE)<<"Notice: Material::getDiffuse(FRONT_AND_BACK) called on material "<<endl;
             notify(NOTICE)<<"        with seperate FRONT and BACK diffuse colors."<<endl;
        }
        return _diffuseFront;
    }
    notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::getDiffuse()."<<endl;
    return _diffuseFront;
}

void Material::setSpecular( MaterialFace face, const Vec4& specular )
{
    switch(face) {
    case(FACE_FRONT):
        _specularFrontAndBack = false;
        _specularFront = specular;
        break;    
    case(FACE_BACK):
        _specularFrontAndBack = false;
        _specularBack = specular;
        break;
    case(FACE_FRONT_AND_BACK):
        _specularFrontAndBack = true;
        _specularFront = specular;
        _specularBack = specular;
        break;
    default:
        notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::setSpecular()."<<endl;
        break;
    }
}

const Vec4& Material::getSpecular(MaterialFace face) const
{
    switch(face) {
    case(FACE_FRONT):
        return _specularFront;
    case(FACE_BACK):
        return _specularBack;
    case(FACE_FRONT_AND_BACK):
        if (!_specularFrontAndBack)
        {
             notify(NOTICE)<<"Notice: Material::getSpecular(FRONT_AND_BACK) called on material "<<endl;
             notify(NOTICE)<<"        with seperate FRONT and BACK specular colors."<<endl;
        }
        return _specularFront;
    }
    notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::getSpecular()."<<endl;
    return _specularFront;
}

void Material::setEmission( MaterialFace face, const Vec4& emission )
{
    switch(face) {
    case(FACE_FRONT):
        _emissionFrontAndBack = false;
        _emissionFront = emission;
        break;    
    case(FACE_BACK):
        _emissionFrontAndBack = false;
        _emissionBack = emission;
        break;
    case(FACE_FRONT_AND_BACK):
        _emissionFrontAndBack = true;
        _emissionFront = emission;
        _emissionBack = emission;
        break;
    default:
        notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::setEmission()."<<endl;
        break;
    }
}

const Vec4& Material::getEmission(MaterialFace face) const
{
    switch(face) {
    case(FACE_FRONT):
        return _emissionFront;
    case(FACE_BACK):
        return _emissionBack;
    case(FACE_FRONT_AND_BACK):
        if (!_emissionFrontAndBack)
        {
             notify(NOTICE)<<"Notice: Material::getEmission(FRONT_AND_BACK) called on material "<<endl;
             notify(NOTICE)<<"        with seperate FRONT and BACK emission colors."<<endl;
        }
        return _emissionFront;
    }
    notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::getEmission()."<<endl;
    return _emissionFront;
}

void Material::setShininess( MaterialFace face, float shininess )
{
    switch(face) {
    case(FACE_FRONT):
        _shininessFrontAndBack = false;
        _shininessFront = shininess;
        break;    
    case(FACE_BACK):
        _shininessFrontAndBack = false;
        _shininessBack = shininess;
        break;
    case(FACE_FRONT_AND_BACK):
        _shininessFrontAndBack = true;
        _shininessFront = shininess;
        _shininessBack = shininess;
        break;
    default:
        notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::setShininess()."<<endl;
        break;
    }
}

float Material::getShininess(MaterialFace face) const
{
    switch(face) {
    case(FACE_FRONT):
        return _shininessFront;
    case(FACE_BACK):
        return _shininessBack;
    case(FACE_FRONT_AND_BACK):
        if (!_shininessFrontAndBack)
        {
             notify(NOTICE)<<"Notice: Material::getShininess(FRONT_AND_BACK) called on material "<<endl;
             notify(NOTICE)<<"        with seperate FRONT and BACK shininess colors."<<endl;
        }
        return _shininessFront;
    }
    notify(NOTICE)<<"Notice: invalid MaterialFace passed to Material::getShininess()."<<endl;
    return _shininessFront;
}


bool Material::matchFaceAndColor(Input& fr,const char* name,MaterialFace& mf,Vec4& color)
{
    bool iteratorAdvanced = false;

    if (fr[0].matchWord(name))
    {
        int fr_inc = 1;
        if (fr[1].matchWord("FRONT"))
        {
            mf = FACE_FRONT;
            ++fr_inc;
        }
        else if (fr[1].matchWord("BACK"))
        {
            mf = FACE_BACK;
            ++fr_inc;
        }

        if (fr[fr_inc].getFloat(color[0]) && fr[fr_inc+1].getFloat(color[1]) && fr[fr_inc+2].getFloat(color[2]))
        {
            fr_inc += 3;

            if (fr[fr_inc].getFloat(color[3])) ++fr_inc;
            else color[3] = 1.0f;

            fr+=fr_inc;

            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool Material::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    Vec4 data(0.0f,  0.0f, 0.0f, 1.0f);
    MaterialFace mf = FACE_FRONT_AND_BACK;

    if (fr[0].matchWord("ColorMode"))
    {
        if (fr[1].matchWord("AMBIENT"))
        {
            setColorMode(AMBIENT);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("DIFFUSE"))
        {
            setColorMode(DIFFUSE);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("SPECULAR"))
        {
            setColorMode(SPECULAR);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("EMISSION"))
        {
            setColorMode(EMISSION);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("AMBIENT_AND_DIFFUSE"))
        {
            setColorMode(AMBIENT_AND_DIFFUSE);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("OFF"))
        {
            setColorMode(OFF);
            fr+=2;
            iteratorAdvanced = true;
        }

    }


    if (matchFaceAndColor(fr,"ambientColor",mf,data))
    {
        setAmbient(mf,data);
        iteratorAdvanced = true;
    }

    if (matchFaceAndColor(fr,"diffuseColor",mf,data))
    {
        setDiffuse(mf,data);
        iteratorAdvanced = true;
    }

    if (matchFaceAndColor(fr,"specularColor",mf,data))
    {
        setSpecular(mf,data);
        iteratorAdvanced = true;
    }

    if (matchFaceAndColor(fr,"emissionColor",mf,data))
    {
        setEmission(mf,data);
        iteratorAdvanced = true;
    }

    if (matchFaceAndColor(fr,"ambientColor",mf,data))
    {
        setAmbient(mf,data);
        iteratorAdvanced = true;
    }

    float shininess = 0.0f;
    if (fr[0].matchWord("shininess"))
    {

        mf = FACE_FRONT_AND_BACK;
        int fr_inc = 1;

        if (fr[1].matchWord("FRONT"))
        {
            mf = FACE_FRONT;
            ++fr_inc;
        }
        else if (fr[1].matchWord("BACK"))
        {
            mf = FACE_BACK;
            ++fr_inc;
        }

        if (fr[fr_inc].getFloat(shininess))
        {
            fr+=(fr_inc+1);
            setShininess(mf,shininess);
            iteratorAdvanced = true;
        }

    }

    float transparency = 0.0f;
    if (fr[0].matchWord("transparency") && fr[1].getFloat(transparency))
    {

        _ambientFront[3] = 1.0f-transparency;
        _diffuseFront[3] = 1.0f-transparency;
        _specularFront[3] = 1.0f-transparency;
        _emissionFront[3] = 1.0f-transparency;

        _ambientBack[3] = 1.0f-transparency;
        _diffuseBack[3] = 1.0f-transparency;
        _specularBack[3] = 1.0f-transparency;
        _emissionBack[3] = 1.0f-transparency;

        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;

}


bool Material::writeLocalData(Output& fw)
{
    switch(_colorMode)
    {
        case(AMBIENT): fw.indent() << "ColorMode AMBIENT" << endl; break;
        case(DIFFUSE): fw.indent() << "ColorMode DIFFUSE" << endl; break;
        case(SPECULAR): fw.indent() << "ColorMode SPECULAR" << endl; break;
        case(EMISSION): fw.indent() << "ColorMode EMISSION" << endl; break;
        case(AMBIENT_AND_DIFFUSE): fw.indent() << "ColorMode AMBIENT_AND_DIFFUSE" << endl; break;
        case(OFF): fw.indent() << "ColorMode OFF" << endl; break;
    }

    if (_ambientFrontAndBack)
    {
        fw.indent() << "ambientColor " << _ambientFront << endl;
    }
    else
    {
        fw.indent() << "ambientColor FRONT " << _ambientFront <<  endl;
        fw.indent() << "ambientColor BACK  " << _ambientBack  << endl;
    }

    if (_diffuseFrontAndBack)
    {
        fw.indent() << "diffuseColor " << _diffuseFront << endl;
    }
    else
    {
        fw.indent() << "diffuseColor FRONT " << _diffuseFront <<  endl;
        fw.indent() << "diffuseColor BACK  " << _diffuseBack  << endl;
    }

    if (_specularFrontAndBack)
    {
        fw.indent() << "specularColor " << _specularFront << endl;
    }
    else
    {
        fw.indent() << "specularColor FRONT " << _specularFront <<  endl;
        fw.indent() << "specularColor BACK  " << _specularBack  << endl;
    }

    if (_emissionFrontAndBack)
    {
        fw.indent() << "emissionColor " << _emissionFront << endl;
    }
    else
    {
        fw.indent() << "emissionColor FRONT " << _emissionFront <<  endl;
        fw.indent() << "emissionColor BACK  " << _emissionBack  << endl;
    }

    if (_shininessFrontAndBack)
    {
        fw.indent() << "shininess " << _shininessFront << endl;
    }
    else
    {
        fw.indent() << "shininess FRONT " << _shininessFront <<  endl;
        fw.indent() << "shininess BACK  " << _shininessBack  << endl;
    }

    return true;
}
void Material::apply( void )
{
    if (_colorMode==OFF)
    {
        glDisable(GL_COLOR_MATERIAL);
    }
    else
    {
        glEnable(GL_COLOR_MATERIAL);  
        glColorMaterial(GL_FRONT_AND_BACK,(GLenum)_colorMode);
        switch(_colorMode) {
        case(AMBIENT): glColor4fv(_ambientFront.ptr()); break;
        case(DIFFUSE): glColor4fv(_diffuseFront.ptr()); break;
        case(SPECULAR): glColor4fv(_specularFront.ptr()); break;
        case(EMISSION): glColor4fv(_emissionFront.ptr()); break;
        case(AMBIENT_AND_DIFFUSE): glColor4fv(_diffuseFront.ptr()); break;
        case(OFF): break;
        }
    }

    if (_colorMode!=AMBIENT && _colorMode!=AMBIENT_AND_DIFFUSE)
    {
        if (_ambientFrontAndBack)
        {
            glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, _ambientFront.ptr() );
        }
        else
        {
            glMaterialfv( GL_FRONT, GL_AMBIENT, _ambientFront.ptr() );
            glMaterialfv( GL_BACK, GL_AMBIENT, _ambientBack.ptr() );
        }
    }
    
    if (_colorMode!=DIFFUSE && _colorMode!=AMBIENT_AND_DIFFUSE)
    {
        if (_diffuseFrontAndBack)
        {
            glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuseFront.ptr() );
        }
        else
        {
            glMaterialfv( GL_FRONT, GL_DIFFUSE, _diffuseFront.ptr() );
            glMaterialfv( GL_BACK, GL_DIFFUSE, _diffuseBack.ptr() );
        }
    }
    
    if (_colorMode!=SPECULAR)
    {
        if (_specularFrontAndBack)
        {
            glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, _specularFront.ptr() );
        }
        else
        {
            glMaterialfv( GL_FRONT, GL_SPECULAR, _specularFront.ptr() );
            glMaterialfv( GL_BACK, GL_SPECULAR, _specularBack.ptr() );
        }
    }
    
    if (_colorMode!=EMISSION)
    {
        if (_emissionFrontAndBack)
        {
            glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, _emissionFront.ptr() );
        }
        else
        {
            glMaterialfv( GL_FRONT, GL_EMISSION, _emissionFront.ptr() );
            glMaterialfv( GL_BACK, GL_EMISSION, _emissionBack.ptr() );
        }
    }
    
    if (_shininessFrontAndBack)
    {
        glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, _shininessFront* 128.0f );
    }
    else
    {
        glMaterialf( GL_FRONT, GL_SHININESS, _shininessFront* 128.0f );
        glMaterialf( GL_BACK, GL_SHININESS, _shininessBack* 128.0f );
    }

}
