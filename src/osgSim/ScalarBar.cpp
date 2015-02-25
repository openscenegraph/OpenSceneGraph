#include <osgSim/ScalarBar>
#include <osgText/Text>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <sstream>

using namespace osgSim;

ScalarBar::~ScalarBar()
{
}

std::string ScalarBar::ScalarPrinter::printScalar(float scalar)
{
    std::stringstream ostr;
    ostr<<scalar;
    return ostr.str();
}

void ScalarBar::setNumColors(int numColors)
{
    _numColors = numColors;
    createDrawables();
}

int ScalarBar::getNumColors() const
{
    return _numColors;
}

void ScalarBar::setNumLabels(int numLabels)
{
    _numLabels = numLabels;
    createDrawables();
}

int ScalarBar::getNumLabels() const
{
    return _numLabels;
}

void ScalarBar::setScalarsToColors(ScalarsToColors* stc)
{
    _stc = stc;
    createDrawables();
}

const ScalarsToColors* ScalarBar::getScalarsToColors() const
{
    return _stc.get();
}

void ScalarBar::setTitle(const std::string& title)
{
    _title = title;
    createDrawables();
}

const std::string& ScalarBar::getTitle() const
{
    return _title;
}

void ScalarBar::setPosition(const osg::Vec3& pos)
{
    _position = pos;
    createDrawables();
}

void ScalarBar::setWidth(float width)
{
    _width = width;
    createDrawables();
}

void ScalarBar::setOrientation(ScalarBar::Orientation orientation)
{
    _orientation = orientation;
    createDrawables();
}

ScalarBar::Orientation ScalarBar::getOrientation() const
{
    return _orientation;
}

void ScalarBar::setAspectRatio(float aspectRatio)
{
    _aspectRatio = aspectRatio;
    createDrawables();
}

float ScalarBar::getAspectRatio() const
{
    return _aspectRatio;
}

void ScalarBar::setScalarPrinter(ScalarPrinter* sp)
{
    _sp = sp;
    createDrawables();
}

const ScalarBar::ScalarPrinter* ScalarBar::getScalarPrinter() const
{
    return _sp.get();
}


void ScalarBar::setTextProperties(const TextProperties& tp)
{
    _textProperties = tp;
    createDrawables();
}

const ScalarBar::TextProperties& ScalarBar::getTextProperties() const
{
    return _textProperties;
}


void ScalarBar::createDrawables()
{
    // Remove any existing Drawables
    removeDrawables(0, getNumDrawables());

    if (_numColors==0) return;

    osg::Matrix matrix;
    if(_orientation==HORIZONTAL)
    {
        matrix = osg::Matrix::translate(_position);
    }
    else
    {
        matrix = osg::Matrix::rotate(osg::DegreesToRadians(90.0f),0.0f,0.0f,1.0f) * osg::Matrix::translate(_position);
    }

    // 1. First the bar
    // =================
    osg::ref_ptr<osg::Geometry> bar = new osg::Geometry();

    // Create the bar - created in 'real' coordinate space the moment,
    // with xyz values reflecting those of the actual scalar values in play.
    // FIXME: Consider positioning at origin! Should be easy enough to do.

    // Vertices
    osg::ref_ptr<osg::Vec3Array> vs(new osg::Vec3Array);
    vs->reserve(4*_numColors);

    float incr = (_stc->getMax() - _stc->getMin()) / _numColors;
    float xincr = (_width) / _numColors;
    float arOffset = _width * _aspectRatio;

    int i;
    for(i=1; i<=_numColors; ++i)
    {
        vs->push_back(osg::Vec3((i-1) * xincr, 0.0f,     0.0f)*matrix);
        vs->push_back(osg::Vec3((i-1) * xincr, arOffset, 0.0f)*matrix);
        vs->push_back(osg::Vec3(i     * xincr, arOffset, 0.0f)*matrix);
        vs->push_back(osg::Vec3(i     * xincr, 0.0f,     0.0f)*matrix);
    }
    bar->setVertexArray(vs.get());

    // Colours
    osg::ref_ptr<osg::Vec4Array> cs(new osg::Vec4Array);
    cs->reserve(4*_numColors);
    const float halfIncr = incr*0.5;
    // Check whether to use interpolated colors or retain the original
    // color values if _numColors equals the number of ColorRange colors defined
    ColorRange *cr = dynamic_cast<ColorRange*>(_stc.get());
    bool fixedColors = cr && (_numColors == static_cast<int>(cr->getColors().size()));
    for (i = 0; i<_numColors; ++i)
    {
        // We add half an increment to the color look-up to get the color
        // square in the middle of the 'block' unless using fixed colors.
        osg::Vec4 c = fixedColors ? cr->getColors()[i] : _stc->getColor(_stc->getMin() + (i*incr) + halfIncr);
        cs->push_back(c);
        cs->push_back(c);
        cs->push_back(c);
        cs->push_back(c);
    }

    bar->setColorArray(cs.get(), osg::Array::BIND_PER_VERTEX);

    // Normal
    osg::ref_ptr<osg::Vec3Array> ns(new osg::Vec3Array);
    ns->push_back(osg::Matrix::transform3x3(osg::Vec3(0.0f,0.0f,1.0f),matrix));
    bar->setNormalArray(ns.get(), osg::Array::BIND_OVERALL);

    // The Quad strip that represents the bar
    bar->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,vs->size()));
    bar->getOrCreateStateSet()->setAttributeAndModes( new osg::PolygonOffset(1,1),
                            osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON );

    addDrawable(bar.get());

#define CHARACTER_OFFSET_FACTOR (0.3f)

    // 2. Then the text labels
    // =======================

    // Check the character size, if it's 0, estimate a good character size
    float characterSize = _textProperties._characterSize;
    if(characterSize == 0) characterSize = _width * 0.03f;

    osgText::Font* font = osgText::readFontFile(_textProperties._fontFile.c_str());

    std::vector<osgText::Text*> texts(_numLabels);      // We'll need to collect pointers to these for later
    float labelIncr = (_numLabels>0) ? (_stc->getMax()-_stc->getMin())/(_numLabels-1) : 0.0f;
    float labelxIncr = (_numLabels>0) ? (_width)/(_numLabels-1) : 0.0f;
    const float labelStickStartY = _orientation==HORIZONTAL ? arOffset : 0;
    const float labelY = labelStickStartY +
        (_orientation==HORIZONTAL ?  characterSize : -characterSize) * CHARACTER_OFFSET_FACTOR;


    for(i=0; i<_numLabels; ++i)
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(_textProperties._color);
        text->setFontResolution(_textProperties._fontResolution.first,_textProperties._fontResolution.second);
        text->setCharacterSize(characterSize);
        text->setText(_sp->printScalar(_stc->getMin()+(i*labelIncr)));

        text->setPosition(osg::Vec3((i*labelxIncr), labelY, 0.0f)*matrix);
        text->setAlignment( (_orientation==HORIZONTAL) ? osgText::Text::CENTER_BASE_LINE : osgText::Text::LEFT_CENTER);

        addDrawable(text);

        texts[i] = text;
    }

    // 3. The title
    // ============

    if(_title != "")
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(_textProperties._color);
        text->setFontResolution(_textProperties._fontResolution.first,_textProperties._fontResolution.second);
        text->setCharacterSize(characterSize);
        text->setText(_title);

        osg::Vec3 titlePos;
        if ( _orientation==HORIZONTAL )
        {
            const float titleY = (_numLabels>0) ? labelY + characterSize : labelY;
            titlePos = osg::Vec3((_width/2.0f), titleY, 0.0f);
        }
        else
        {
            titlePos = osg::Vec3(_width+characterSize*0.5, arOffset*0.5, 0 );
        }

        // Position the title at the middle of the bar above any labels.
        text->setPosition(titlePos*matrix);
        text->setAlignment(
            _orientation==HORIZONTAL ? osgText::Text::CENTER_BASE_LINE : osgText::Text::CENTER_BOTTOM);

        addDrawable(text);
    }

    // 4. The rectangular border and sticks
    // ====================================

    osg::ref_ptr<osg::Vec3Array> annotVertices = new osg::Vec3Array;

    //Border
    annotVertices->push_back( osg::Vec3(0.0f,0.0f,0.0f) * matrix  );
    annotVertices->push_back( osg::Vec3(0.0f,arOffset,0.0f) * matrix  );

    annotVertices->push_back( osg::Vec3(0.0f,arOffset,0.0f) * matrix  );
    annotVertices->push_back( osg::Vec3(_width,arOffset,0.0f) * matrix  );

    annotVertices->push_back( osg::Vec3(_width,arOffset,0.0f) * matrix  );
    annotVertices->push_back( osg::Vec3(_width,0.0f,0.0f) * matrix  );

    annotVertices->push_back( osg::Vec3(_width,0.0f,0.0f) * matrix  );
    annotVertices->push_back( osg::Vec3(0.0f,0.0f,0.0f) * matrix  );

    //Sticks
    for(i=0; i<_numLabels; ++i)
    {
        const osg::Vec3 p1(osg::Vec3((i*labelxIncr), labelStickStartY, 0.0f)*matrix);
        const osg::Vec3 p2(osg::Vec3((i*labelxIncr), labelY, 0.0f)*matrix);
        annotVertices->push_back( p1 );
        annotVertices->push_back( p2 );
    }

    osg::ref_ptr<osg::Geometry> annotationGeom = new osg::Geometry();
    annotationGeom->addPrimitiveSet( new osg::DrawArrays(GL_LINES,0,annotVertices->size()) );
    annotationGeom->setVertexArray( annotVertices.get() );
    osg::ref_ptr<osg::Material> annotMaterial = new osg::Material;
    annotMaterial->setDiffuse( osg::Material::FRONT, _textProperties._color );
    annotationGeom->getOrCreateStateSet()->setAttribute( annotMaterial.get() );

    addDrawable( annotationGeom.get() );
}
