/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


#include <osgText/Text>

#include <osg/Math>
#include <osg/GL>
#include <osg/Notify>
#include <osg/PolygonOffset>
#include <osg/TexEnv>
#include <osg/io_utils>

#include <osgUtil/CullVisitor>

#include <osgDB/ReadFile>

#include <sstream>
#include <iomanip>

#define DEBUG_MESSAGE_LEVEL osg::INFO
#define DEBUG_MESSAGE osg::notify(DEBUG_MESSAGE_LEVEL)

using namespace osg;
using namespace osgText;

Text::Text():
    _shaderTechnique(GREYSCALE),
    _enableDepthWrites(true),
    _backdropType(NONE),
    _backdropHorizontalOffset(0.07f),
    _backdropVerticalOffset(0.07f),
    _backdropColor(0.0f, 0.0f, 0.0f, 1.0f),
    _colorGradientMode(SOLID),
    _colorGradientTopLeft(1.0f, 0.0f, 0.0f, 1.0f),
    _colorGradientBottomLeft(0.0f, 1.0f, 0.0f, 1.0f),
    _colorGradientBottomRight(0.0f, 0.0f, 1.0f, 1.0f),
    _colorGradientTopRight(1.0f, 1.0f, 1.0f, 1.0f)
{
    _supportsVertexBufferObjects = true;

    const std::string& str = osg::DisplaySettings::instance()->getTextShaderTechnique();
    if (!str.empty())
    {
        if (str=="ALL_FEATURES" || str=="ALL") _shaderTechnique = ALL_FEATURES;
        else if (str=="GREYSCALE") _shaderTechnique = GREYSCALE;
        else if (str=="SIGNED_DISTANCE_FIELD" || str=="SDF") _shaderTechnique = SIGNED_DISTANCE_FIELD;
        else if (str=="NO_TEXT_SHADER" || str=="NONE") _shaderTechnique = NO_TEXT_SHADER;
    }

    assignStateSet();
}

Text::Text(const Text& text,const osg::CopyOp& copyop):
    osgText::TextBase(text,copyop),
    _shaderTechnique(text._shaderTechnique),
    _enableDepthWrites(text._enableDepthWrites),
    _backdropType(text._backdropType),
    _backdropHorizontalOffset(text._backdropHorizontalOffset),
    _backdropVerticalOffset(text._backdropVerticalOffset),
    _backdropColor(text._backdropColor),
    _colorGradientMode(text._colorGradientMode),
    _colorGradientTopLeft(text._colorGradientTopLeft),
    _colorGradientBottomLeft(text._colorGradientBottomLeft),
    _colorGradientBottomRight(text._colorGradientBottomRight),
    _colorGradientTopRight(text._colorGradientTopRight)
{
    computeGlyphRepresentation();
}

Text::~Text()
{
}


void Text::setShaderTechnique(ShaderTechnique technique)
{
    if (_shaderTechnique==technique) return;

    _shaderTechnique = technique;

    assignStateSet();

    computeGlyphRepresentation();
}

osg::StateSet* Text::createStateSet()
{
    Font* activeFont = getActiveFont();
    if (!activeFont) return 0;

    Font::StateSets& statesets = activeFont->getCachedStateSets();

    std::stringstream ss;
    ss.imbue(std::locale::classic());
    ss<<std::fixed<<std::setprecision(3);

    osg::StateSet::DefineList defineList;
    if (_backdropType!=NONE)
    {
        ss.str("");
        ss << "vec4("<<_backdropColor.r()<<", "<<_backdropColor.g()<<", "<<_backdropColor.b()<<", "<<_backdropColor.a()<<")";

        defineList["BACKDROP_COLOR"] = osg::StateSet::DefinePair(ss.str(), osg::StateAttribute::ON);


        if (_backdropType==OUTLINE)
        {
            ss.str("");
            ss <<_backdropHorizontalOffset;
            defineList["OUTLINE"] = osg::StateSet::DefinePair(ss.str(), osg::StateAttribute::ON);
        }
        else
        {
            osg::Vec2 offset(_backdropHorizontalOffset, _backdropVerticalOffset);
            switch(_backdropType)
            {
                case(DROP_SHADOW_BOTTOM_RIGHT) :    offset.set(_backdropHorizontalOffset, -_backdropVerticalOffset); break;
                case(DROP_SHADOW_CENTER_RIGHT) :    offset.set(_backdropHorizontalOffset, 0.0f); break;
                case(DROP_SHADOW_TOP_RIGHT) :       offset.set(_backdropHorizontalOffset, _backdropVerticalOffset); break;
                case(DROP_SHADOW_BOTTOM_CENTER) :   offset.set(0.0f, -_backdropVerticalOffset); break;
                case(DROP_SHADOW_TOP_CENTER) :      offset.set(0.0f, _backdropVerticalOffset); break;
                case(DROP_SHADOW_BOTTOM_LEFT) :     offset.set(-_backdropHorizontalOffset, -_backdropVerticalOffset); break;
                case(DROP_SHADOW_CENTER_LEFT) :     offset.set(-_backdropHorizontalOffset, 0.0f); break;
                case(DROP_SHADOW_TOP_LEFT) :        offset.set(-_backdropHorizontalOffset, _backdropVerticalOffset); break;
                default : break;
            }

            ss.str("");
            ss << "vec2("<<offset.x()<<", "<<offset.y()<<")";

            defineList["SHADOW"] = osg::StateSet::DefinePair(ss.str(), osg::StateAttribute::ON);
        }
    }

    {
        ss<<std::fixed<<std::setprecision(1);

        ss.str("");
        ss << float(_fontSize.second);

        defineList["GLYPH_DIMENSION"] = osg::StateSet::DefinePair(ss.str(), osg::StateAttribute::ON);

        ss.str("");
        ss << float(activeFont->getTextureWidthHint());
        defineList["TEXTURE_DIMENSION"] = osg::StateSet::DefinePair(ss.str(), osg::StateAttribute::ON);
    }

    if (_shaderTechnique>GREYSCALE)
    {
        defineList["SIGNED_DISTANCE_FIELD"] = osg::StateSet::DefinePair("1", osg::StateAttribute::ON);
    }

#if 0
    OSG_NOTICE<<"Text::createStateSet() defines:"<<defineList.size()<<std::endl;
    for(osg::StateSet::DefineList::iterator itr = defineList.begin();
        itr != defineList.end();
        ++itr)
    {
        OSG_NOTICE<<"   define["<<itr->first<<"] = "<<itr->second.first<<std::endl;
    }
#endif

    if (!statesets.empty())
    {
        for(Font::StateSets::iterator itr = statesets.begin();
            itr != statesets.end();
            ++itr)
        {
            if ((*itr)->getDefineList()==defineList)
            {
                // OSG_NOTICE<<"Text::createStateSet() : Matched DefineList, return StateSet "<<itr->get()<<std::endl;
                return itr->get();
            }
            else
            {
            }
        }
    }


    if (osg::isNotifyEnabled(DEBUG_MESSAGE_LEVEL))
    {
        DEBUG_MESSAGE<<"Text::createStateSet() ShaderTechnique ";
        switch(_shaderTechnique)
        {
            case(NO_TEXT_SHADER) : DEBUG_MESSAGE<<"NO_TEXT_SHADER"<<std::endl; break;
            case(GREYSCALE) : DEBUG_MESSAGE<<"GREYSCALE"<<std::endl; break;
            case(SIGNED_DISTANCE_FIELD) : DEBUG_MESSAGE<<"SIGNED_DISTANCE_FIELD"<<std::endl; break;
            case(ALL_FEATURES) : DEBUG_MESSAGE<<"ALL_FEATURES"<<std::endl; break;
        }
    }

    DEBUG_MESSAGE<<"Text::createStateSet() : Not Matched DefineList, creating new StateSet"<<std::endl;

    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

    stateset->setDefineList(defineList);

    statesets.push_back(stateset.get());

    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);


    #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
    osg::DisplaySettings::ShaderHint shaderHint = osg::DisplaySettings::instance()->getShaderHint();
    if (_shaderTechnique==NO_TEXT_SHADER && shaderHint==osg::DisplaySettings::SHADER_NONE)
    {
        DEBUG_MESSAGE<<"Font::Font() Fixed function pipeline"<<std::endl;

        stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::ON);
        return stateset.release();
    }
    #endif

    // set up the StateSet to use shaders
    stateset->addUniform(new osg::Uniform("glyphTexture", 0));

    osg::ref_ptr<osg::Program> program = new osg::Program;
    stateset->setAttributeAndModes(program.get());

    {
        DEBUG_MESSAGE<<"Using shaders/osgText_Text.vert"<<std::endl;

        #include "shaders/osgText_Text_vert.cpp"
        program->addShader(osgDB::readRefShaderFileWithFallback(osg::Shader::VERTEX, "shaders/osgText_Text.vert", osgText_Text_vert));
    }

    {
        DEBUG_MESSAGE<<"Using shaders/osgText_Text.frag"<<std::endl;

        #include "shaders/osgText_Text_frag.cpp"
        program->addShader(osgDB::readRefShaderFileWithFallback(osg::Shader::FRAGMENT, "shaders/osgText_Text.frag", osgText_Text_frag));
    }

    return stateset.release();
}

Font* Text::getActiveFont()
{
    if (_font.valid()) return _font.get();

    if (!_fontFallback) _fontFallback = Font::getDefaultFont();

    return _fontFallback.get();
}

String::iterator Text::computeLastCharacterOnLine(osg::Vec2& cursor, String::iterator first,String::iterator last)
{
    Font* activefont = getActiveFont();
    if (!activefont) return last;

    float hr = _characterHeight;
    float wr = hr/getCharacterAspectRatio();

    bool kerning = true;
    unsigned int previous_charcode = 0;

    String::iterator lastChar = first;

    for(bool outOfSpace=false;lastChar!=last;++lastChar)
    {
        unsigned int charcode = *lastChar;

        if (charcode=='\n')
        {
            return lastChar;
        }

        Glyph* glyph = activefont->getGlyph(_fontSize, charcode);
        if (glyph)
        {

           float width = (float)(glyph->getWidth()) * wr;

            if (_layout==RIGHT_TO_LEFT)
            {
                cursor.x() -= glyph->getHorizontalAdvance() * wr;
            }

            // adjust cursor position w.r.t any kerning.
            if (kerning && previous_charcode)
            {
                switch(_layout)
                {
                  case LEFT_TO_RIGHT:
                  {
                    osg::Vec2 delta(activefont->getKerning(_fontSize, previous_charcode, charcode, _kerningType));
                    cursor.x() += delta.x() * wr;
                    cursor.y() += delta.y() * hr;
                    break;
                  }
                  case RIGHT_TO_LEFT:
                  {
                    osg::Vec2 delta(activefont->getKerning(_fontSize, charcode, previous_charcode, _kerningType));
                    cursor.x() -= delta.x() * wr;
                    cursor.y() -= delta.y() * hr;
                    break;
                  }
                  case VERTICAL:
                    break; // no kerning when vertical.
                }            // check to see if we are still within line if not move to next line.
            }

            switch(_layout)
            {
              case LEFT_TO_RIGHT:
              {
                if (_maximumWidth>0.0f && cursor.x()+width>_maximumWidth) outOfSpace=true;
                if(_maximumHeight>0.0f && cursor.y()<-_maximumHeight) outOfSpace=true;
                break;
              }
              case RIGHT_TO_LEFT:
              {
                if (_maximumWidth>0.0f && cursor.x()<-_maximumWidth) outOfSpace=true;
                if(_maximumHeight>0.0f && cursor.y()<-_maximumHeight) outOfSpace=true;
                break;
              }
              case VERTICAL:
                if (_maximumHeight>0.0f && cursor.y()<-_maximumHeight) outOfSpace=true;
                break;
            }

            // => word boundary detection & wrapping
            if (outOfSpace) break;

            // move the cursor onto the next character.
            switch(_layout)
            {
              case LEFT_TO_RIGHT: cursor.x() += glyph->getHorizontalAdvance() * wr; break;
              case VERTICAL:      cursor.y() -= glyph->getVerticalAdvance() *hr; break;
              case RIGHT_TO_LEFT: break; // nop.
            }

            previous_charcode = charcode;

        }

    }

    // word boundary detection & wrapping
    if (lastChar!=last)
    {
        String::iterator lastValidChar = lastChar;
        String::iterator prevChar;
        while (lastValidChar != first){
            prevChar = lastValidChar - 1;

            // last char is after a hyphen
                if(*lastValidChar == '-')
                return lastValidChar + 1;

            // last char is start of whitespace
            if((*lastValidChar == ' ' || *lastValidChar == '\n') && (*prevChar != ' ' && *prevChar != '\n'))
                return lastValidChar;

            // Subtract off glyphs from the cursor position (to correctly center text)
                if(*prevChar != '-')
            {
                Glyph* glyph = activefont->getGlyph(_fontSize, *prevChar);
                if (glyph)
                {
                    switch(_layout)
                    {
                    case LEFT_TO_RIGHT: cursor.x() -= glyph->getHorizontalAdvance() * wr; break;
                    case VERTICAL:      cursor.y() += glyph->getVerticalAdvance() * hr; break;
                    case RIGHT_TO_LEFT: break; // nop.
                    }
                }
            }

            lastValidChar = prevChar;
          }
    }

    return lastChar;
}

void Text::addGlyphQuad(Glyph* glyph, const osg::Vec2& minc, const osg::Vec2& maxc, const osg::Vec2& mintc, const osg::Vec2& maxtc)
{
    // set up the coords of the quad
    const Glyph::TextureInfo* info = glyph->getOrCreateTextureInfo(_shaderTechnique);
    GlyphTexture* glyphTexture = info ? info->texture : 0;
    GlyphQuads& glyphquad = _textureGlyphQuadMap[glyphTexture];

    glyphquad._glyphs.push_back(glyph);

    osg::DrawElements* primitives = glyphquad._primitives.get();
    if (!primitives)
    {
        unsigned int maxIndices = _text.size()*4;
        if (maxIndices>=16384) primitives = new osg::DrawElementsUInt(GL_TRIANGLES);
        else primitives = new osg::DrawElementsUShort(GL_TRIANGLES);
        primitives->setBufferObject(_ebo.get());
        glyphquad._primitives = primitives;
    }


    unsigned int lt = addCoord(osg::Vec2(minc.x(), maxc.y()));
    unsigned int lb = addCoord(osg::Vec2(minc.x(), minc.y()));
    unsigned int rb = addCoord(osg::Vec2(maxc.x(), minc.y()));
    unsigned int rt = addCoord(osg::Vec2(maxc.x(), maxc.y()));

    // set up the tex coords of the quad
    addTexCoord(osg::Vec2(mintc.x(), maxtc.y()));
    addTexCoord(osg::Vec2(mintc.x(), mintc.y()));
    addTexCoord(osg::Vec2(maxtc.x(), mintc.y()));
    addTexCoord(osg::Vec2(maxtc.x(), maxtc.y()));

    primitives->addElement(lt);
    primitives->addElement(lb);
    primitives->addElement(rb);

    primitives->addElement(lt);
    primitives->addElement(rb);
    primitives->addElement(rt);

    primitives->dirty();
}

void Text::computeGlyphRepresentation()
{
    Font* activefont = getActiveFont();
    if (!activefont) return;

    if (!_coords) { _coords = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX); _coords->setBufferObject(_vbo.get()); }
    else _coords->clear();

    if (!_colorCoords) { _colorCoords = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX); _colorCoords->setBufferObject(_vbo.get()); }
    else _colorCoords->clear();

    if (!_texcoords) { _texcoords = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX); _texcoords->setBufferObject(_vbo.get()); }
    else _texcoords->clear();

    for(TextureGlyphQuadMap::iterator itr = _textureGlyphQuadMap.begin();
        itr != _textureGlyphQuadMap.end();
        ++itr)
    {
        GlyphQuads& glyphquads = itr->second;
        glyphquads._glyphs.clear();
        if (glyphquads._primitives.valid())
        {
            glyphquads._primitives->resizeElements(0);
            glyphquads._primitives->dirty();
        }
    }


    _lineCount = 0;

    if (_text.empty())
    {
        _textBB.set(0,0,0,0,0,0);//no size text
        computePositions(); //to reset the origin
        return;
    }

    //OpenThreads::ScopedLock<Font::FontMutex> lock(*(activefont->getSerializeFontCallsMutex()));

    // initialize bounding box, it will be expanded during glyph position calculation
    _textBB.init();

    _textBB.set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    osg::Vec2 startOfLine_coords(0.0f,0.0f);
    osg::Vec2 cursor(startOfLine_coords);
    osg::Vec2 local(0.0f,0.0f);

    unsigned int previous_charcode = 0;
    unsigned int linelength = 0;
    bool horizontal = _layout!=VERTICAL;
    bool kerning = true;

    float hr = _characterHeight;
    float wr = hr/getCharacterAspectRatio();

    for(String::iterator itr=_text.begin();
        itr!=_text.end();
        )
    {
        // record the start of the current line
        String::iterator startOfLine_itr = itr;

        // find the end of the current line.
        osg::Vec2 endOfLine_coords(cursor);
        String::iterator endOfLine_itr = computeLastCharacterOnLine(endOfLine_coords, itr,_text.end());

        linelength = endOfLine_itr - startOfLine_itr;

        // Set line position to correct alignment.
        switch(_layout)
        {
          case LEFT_TO_RIGHT:
          {
            switch(_alignment)
            {
              // nothing to be done for these
              //case LEFT_TOP:
              //case LEFT_CENTER:
              //case LEFT_BOTTOM:
              //case LEFT_BASE_LINE:
              //case LEFT_BOTTOM_BASE_LINE:
              //  break;
              case CENTER_TOP:
              case CENTER_CENTER:
              case CENTER_BOTTOM:
              case CENTER_BASE_LINE:
              case CENTER_BOTTOM_BASE_LINE:
                cursor.x() = (cursor.x() - endOfLine_coords.x()) * 0.5f;
                break;
              case RIGHT_TOP:
              case RIGHT_CENTER:
              case RIGHT_BOTTOM:
              case RIGHT_BASE_LINE:
              case RIGHT_BOTTOM_BASE_LINE:
                cursor.x() = cursor.x() - endOfLine_coords.x();
                break;
              default:
                break;
              }
            break;
            }
            case RIGHT_TO_LEFT:
            {
            switch(_alignment)
            {
              case LEFT_TOP:
              case LEFT_CENTER:
              case LEFT_BOTTOM:
              case LEFT_BASE_LINE:
              case LEFT_BOTTOM_BASE_LINE:
                cursor.x() = 2*cursor.x() - endOfLine_coords.x();
                break;
              case CENTER_TOP:
              case CENTER_CENTER:
              case CENTER_BOTTOM:
              case CENTER_BASE_LINE:
              case CENTER_BOTTOM_BASE_LINE:
                cursor.x() = cursor.x() + (cursor.x() - endOfLine_coords.x()) * 0.5f;
                break;
              // nothing to be done for these
              //case RIGHT_TOP:
              //case RIGHT_CENTER:
              //case RIGHT_BOTTOM:
              //case RIGHT_BASE_LINE:
              //case RIGHT_BOTTOM_BASE_LINE:
              //  break;
              default:
                break;
            }
            break;
            }
            case VERTICAL:
            {
            switch(_alignment)
            {
              // TODO: current behaviour top baselines lined up in both cases - need to implement
              //       top of characters alignment - Question is this necessary?
              // ... otherwise, nothing to be done for these 6 cases
              //case LEFT_TOP:
              //case CENTER_TOP:
              //case RIGHT_TOP:
              //  break;
              //case LEFT_BASE_LINE:
              //case CENTER_BASE_LINE:
              //case RIGHT_BASE_LINE:
              //  break;
              case LEFT_CENTER:
              case CENTER_CENTER:
              case RIGHT_CENTER:
                cursor.y() = cursor.y() + (cursor.y() - endOfLine_coords.y()) * 0.5f;
                break;
              case LEFT_BOTTOM_BASE_LINE:
              case CENTER_BOTTOM_BASE_LINE:
              case RIGHT_BOTTOM_BASE_LINE:
                cursor.y() = cursor.y() - (linelength * _characterHeight);
                break;
              case LEFT_BOTTOM:
              case CENTER_BOTTOM:
              case RIGHT_BOTTOM:
                cursor.y() = 2*cursor.y() - endOfLine_coords.y();
                break;
              default:
                break;
            }
            break;
          }
        }

        if (itr!=endOfLine_itr)
        {

            for(;itr!=endOfLine_itr;++itr)
            {
                unsigned int charcode = *itr;

                Glyph* glyph = activefont->getGlyph(_fontSize, charcode);
                if (glyph)
                {
                    float width = (float)(glyph->getWidth()) * wr;
                    float height = (float)(glyph->getHeight()) * hr;

                    if (_layout==RIGHT_TO_LEFT)
                    {
                        cursor.x() -= glyph->getHorizontalAdvance() * wr;
                    }

                    // adjust cursor position w.r.t any kerning.
                    if (kerning && previous_charcode)
                    {
                        switch(_layout)
                        {
                          case LEFT_TO_RIGHT:
                          {
                            osg::Vec2 delta(activefont->getKerning(_fontSize, previous_charcode, charcode, _kerningType));
                            cursor.x() += delta.x() * wr;
                            cursor.y() += delta.y() * hr;
                            break;
                          }
                          case RIGHT_TO_LEFT:
                          {
                            osg::Vec2 delta(activefont->getKerning(_fontSize, charcode, previous_charcode, _kerningType));
                            cursor.x() -= delta.x() * wr;
                            cursor.y() -= delta.y() * hr;
                            break;
                          }
                          case VERTICAL:
                            break; // no kerning when vertical.
                        }
                    }

                    local = cursor;
                    osg::Vec2 bearing(horizontal?glyph->getHorizontalBearing():glyph->getVerticalBearing());
                    local.x() += bearing.x() * wr;
                    local.y() += bearing.y() * hr;

                    const Glyph::TextureInfo* info = glyph->getOrCreateTextureInfo(_shaderTechnique);
                    if (info)
                    {
                        // Adjust coordinates and texture coordinates to avoid
                        // clipping the edges of antialiased characters.
                        osg::Vec2 mintc = info->minTexCoord;
                        osg::Vec2 maxtc = info->maxTexCoord;
                        osg::Vec2 vDiff = maxtc - mintc;
                        float texelMargin = info->texelMargin;

                        float fHorizTCMargin = texelMargin / info->texture->getTextureWidth();
                        float fVertTCMargin = texelMargin / info->texture->getTextureHeight();
                        float fHorizQuadMargin = vDiff.x() == 0.0f ? 0.0f : width * fHorizTCMargin / vDiff.x();
                        float fVertQuadMargin = vDiff.y() == 0.0f ? 0.0f : height * fVertTCMargin / vDiff.y();

                        mintc.x() -= fHorizTCMargin;
                        mintc.y() -= fVertTCMargin;
                        maxtc.x() += fHorizTCMargin;
                        maxtc.y() += fVertTCMargin;
                        osg::Vec2 minc = local+osg::Vec2(0.0f-fHorizQuadMargin,0.0f-fVertQuadMargin);
                        osg::Vec2 maxc = local+osg::Vec2(width+fHorizQuadMargin,height+fVertQuadMargin);

                        addGlyphQuad(glyph, minc, maxc, mintc, maxtc);

                        // move the cursor onto the next character.
                        // also expand bounding box
                        switch(_layout)
                        {
                            case LEFT_TO_RIGHT:
                                cursor.x() += glyph->getHorizontalAdvance() * wr;
                                _textBB.expandBy(osg::Vec3(local.x(), local.y(), 0.0f)); //lower left corner
                                _textBB.expandBy(osg::Vec3(local.x()+width, local.y()+height, 0.0f)); //upper right corner
                                break;
                            case VERTICAL:
                                cursor.y() -= glyph->getVerticalAdvance() * hr;
                                _textBB.expandBy(osg::Vec3(local.x(), local.y()+height, 0.0f)); //upper left corner
                                _textBB.expandBy(osg::Vec3(local.x()+width, local.y(), 0.0f)); //lower right corner
                                break;
                            case RIGHT_TO_LEFT:
                                _textBB.expandBy(osg::Vec3(local.x()+width, local.y(), 0.0f)); //lower right corner
                                _textBB.expandBy(osg::Vec3(local.x(), local.y()+height, 0.0f)); //upper left corner
                                break;
                        }
                    }
                    else
                    {
                        OSG_NOTICE<<"No TextureInfo for "<<charcode<<std::endl;
                    }

                    previous_charcode = charcode;
                }
            }

            // skip over spaces and return.
            while (itr != _text.end() && *itr==' ') ++itr;
            if (itr != _text.end() && *itr=='\n') ++itr;
        }
        else
        {
            ++itr;
        }


        // move to new line.
        switch(_layout)
        {
          case LEFT_TO_RIGHT:
          {
            startOfLine_coords.y() -= _characterHeight * (1.0 + _lineSpacing);
            cursor = startOfLine_coords;
            previous_charcode = 0;
            _lineCount++;
            break;
          }
          case RIGHT_TO_LEFT:
          {
            startOfLine_coords.y() -= _characterHeight * (1.0 + _lineSpacing);
            cursor = startOfLine_coords;
            previous_charcode = 0;
            _lineCount++;
            break;
          }
          case VERTICAL:
          {
            startOfLine_coords.x() += _characterHeight/getCharacterAspectRatio() * (1.0 + _lineSpacing);
            cursor = startOfLine_coords;
            previous_charcode = 0;
            // because _lineCount is the max vertical no. of characters....
            _lineCount = (_lineCount >linelength)?_lineCount:linelength;
          }
          break;
        }
    }

    computePositions();
    computeColorGradients();

    // set up the vertices for any boundinbox or alignment decoration
    setupDecoration();
}

// Returns false if there are no glyphs and the width/height values are invalid.
// Also sets avg_width and avg_height to 0.0f if the value is invalid.
// This method is used several times in a loop for the same object which will produce the same values.
// Further optimization may try saving these values instead of recomputing them.
bool Text::computeAverageGlyphWidthAndHeight(float& avg_width, float& avg_height) const
{
    float width = 0.0f;
    float height = 0.0f;
    float running_width = 0.0f;
    float running_height = 0.0f;
    avg_width = 0.0f;
    avg_height = 0.0f;
    int counter = 0;
    unsigned int i;
    bool is_valid_size = true;

    for (i = 0; i < _coords->size(); i += 4)
    {
        width = (*_coords)[i + 2].x() - (*_coords)[i].x();
        height = (*_coords)[i].y() - (*_coords)[i + 1].y();

        running_width += width;
        running_height += height;
        counter++;
    }

    if(0 == counter)
    {
        is_valid_size = false;
    }
    else
    {
        avg_width = running_width/counter;
        avg_height = running_height/counter;
    }
    return is_valid_size;
}

void Text::computePositionsImplementation()
{
    // TextBase::computePositionsImplementation(); computes basic positions and maps the _textBB to _textBBWithMargin
    TextBase::computePositionsImplementation();

    if (!_textBBWithMargin.valid()) return;

    if (_drawMode & (BOUNDINGBOX | FILLEDBOUNDINGBOX))
    {
        _textBBWithMargin.set(
            _textBBWithMargin.xMin() - _textBBMargin,
            _textBBWithMargin.yMin() - _textBBMargin,
            _textBBWithMargin.zMin(),
            _textBBWithMargin.xMax() + _textBBMargin,
            _textBBWithMargin.yMax() + _textBBMargin,
            _textBBWithMargin.zMax()
        );
    }

    if (_backdropType != NONE)
    {

        float height = _characterHeight;
        float width = height/getCharacterAspectRatio();

        // Finally, we have one more issue to deal with.
        // Now that the text takes more space, we need
        // to adjust the size of the bounding box.
        switch(_backdropType)
        {
            case DROP_SHADOW_BOTTOM_RIGHT:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin(),
                    _textBBWithMargin.yMin() - height * _backdropVerticalOffset,
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax() + width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMax(),
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case DROP_SHADOW_CENTER_RIGHT:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin(),
                    _textBBWithMargin.yMin(),
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax() + width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMax(),
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case DROP_SHADOW_TOP_RIGHT:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin(),
                    _textBBWithMargin.yMin(),
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax() + width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMax() + height * _backdropVerticalOffset,
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case DROP_SHADOW_BOTTOM_CENTER:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin(),
                    _textBBWithMargin.yMin() - height * _backdropVerticalOffset,
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax(),
                    _textBBWithMargin.yMax(),
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case DROP_SHADOW_TOP_CENTER:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin(),
                    _textBBWithMargin.yMin(),
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax(),
                    _textBBWithMargin.yMax() + height * _backdropVerticalOffset,
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case DROP_SHADOW_BOTTOM_LEFT:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin() - width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMin() - height * _backdropVerticalOffset,
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax(),
                    _textBBWithMargin.yMax(),
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case DROP_SHADOW_CENTER_LEFT:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin() - width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMin(),
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax(),
                    _textBBWithMargin.yMax(),
                    _textBBWithMargin.zMax()
                );            break;
            }
            case DROP_SHADOW_TOP_LEFT:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin() - width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMin(),
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax(),
                    _textBBWithMargin.yMax() + height * _backdropVerticalOffset,
                    _textBBWithMargin.zMax()
                );
                break;
            }
            case OUTLINE:
            {
                _textBBWithMargin.set(
                    _textBBWithMargin.xMin() - width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMin() - height * _backdropVerticalOffset,
                    _textBBWithMargin.zMin(),
                    _textBBWithMargin.xMax() + width * _backdropHorizontalOffset,
                    _textBBWithMargin.yMax() + height * _backdropVerticalOffset,
                    _textBBWithMargin.zMax()
                );
                break;
            }
            default: // error
            {
                break;
            }
        }
    }
}

void Text::computeColorGradients()
{
    switch(_colorGradientMode)
    {
        case SOLID:
            return;
            break;
        case PER_CHARACTER:
            computeColorGradientsPerCharacter();
            break;
        case OVERALL:
            computeColorGradientsOverall();
            break;
        default:
            break;
    }
}

void Text::computeColorGradientsOverall()
{

    float min_x = FLT_MAX;
    float min_y = FLT_MAX;
    float max_x = FLT_MIN;
    float max_y = FLT_MIN;

    unsigned int i;

    const Coords& coords = _coords;
    for(i=0;i<coords->size();++i)
    {
        // Min and Max are needed for color gradients
        if((*coords)[i].x() > max_x)
        {
            max_x = (*coords)[i].x();
        }
        if ((*coords)[i].x() < min_x)
        {
            min_x = (*coords)[i].x();
        }
        if ((*coords)[i].y() > max_y)
        {
            max_y = (*coords)[i].y();
        }
        if ((*coords)[i].y() < min_y)
        {
            min_y = (*coords)[i].y();
        }

    }

    const ColorCoords& colorCoords = _colorCoords;

    unsigned int numCoords = coords->size();
    if (numCoords!=colorCoords->size())
    {
        colorCoords->resize(numCoords);
    }

    for(i=0;i<numCoords;++i)
    {
        float red = bilinearInterpolate(
            min_x,
            max_x,
            min_y,
            max_y,
            (*coords)[i].x(),
            (*coords)[i].y(),
            _colorGradientBottomLeft[0],
            _colorGradientTopLeft[0],
            _colorGradientBottomRight[0],
            _colorGradientTopRight[0]
        );

        float green = bilinearInterpolate(
            min_x,
            max_x,
            min_y,
            max_y,
            (*coords)[i].x(),
            (*coords)[i].y(),
            _colorGradientBottomLeft[1],
            _colorGradientTopLeft[1],
            _colorGradientBottomRight[1],
            _colorGradientTopRight[1]
        );

        float blue = bilinearInterpolate(
            min_x,
            max_x,
            min_y,
            max_y,
            (*coords)[i].x(),
            (*coords)[i].y(),
            _colorGradientBottomLeft[2],
            _colorGradientTopLeft[2],
            _colorGradientBottomRight[2],
            _colorGradientTopRight[2]
        );
        // Alpha does not convert to HSV
        float alpha = bilinearInterpolate(
            min_x,
            max_x,
            min_y,
            max_y,
            (*coords)[i].x(),
            (*coords)[i].y(),
            _colorGradientBottomLeft[3],
            _colorGradientTopLeft[3],
            _colorGradientBottomRight[3],
            _colorGradientTopRight[3]
        );

        (*colorCoords)[i] = osg::Vec4(red,green,blue,alpha);
    }
}

void Text::computeColorGradientsPerCharacter()
{
    Coords& coords = _coords;
    ColorCoords& colorCoords = _colorCoords;

    unsigned int numCoords = coords->size();
    if (numCoords!=colorCoords->size())
    {
        colorCoords->resize(numCoords);
    }

    for(unsigned int i=0;i<numCoords;++i)
    {
        switch(i%4)
        {
            case 0: // top-left
                {
                    (*colorCoords)[i] = _colorGradientTopLeft;
                    break;
                }
            case 1: // bottom-left
                {
                    (*colorCoords)[i] = _colorGradientBottomLeft;
                    break;
                }
            case 2: // bottom-right
                {
                    (*colorCoords)[i] = _colorGradientBottomRight;
                    break;
                }
            case 3: // top-right
                {
                    (*colorCoords)[i] = _colorGradientTopRight;
                    break;
                }
            default: // error
                {
                    (*colorCoords)[i] = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
                }
        }
    }
}

void Text::drawImplementation(osg::RenderInfo& renderInfo) const
{
    drawImplementation(*renderInfo.getState(), osg::Vec4(1.0f,1.0f,1.0f,1.0f));
}

void Text::drawImplementationSinglePass(osg::State& state, const osg::Vec4& colorMultiplier) const
{
    if (colorMultiplier.a()==0.0f || _color.a()==0.0f) return;

    osg::VertexArrayState* vas = state.getCurrentVertexArrayState();
    bool usingVertexBufferObjects = state.useVertexBufferObject(_supportsVertexBufferObjects && _useVertexBufferObjects);
    bool usingVertexArrayObjects = usingVertexBufferObjects && state.useVertexArrayObject(_useVertexArrayObject);
    bool requiresSetArrays = !usingVertexBufferObjects || !usingVertexArrayObjects || vas->getRequiresSetArrays();

    if ((_drawMode&(~TEXT))!=0 && !_decorationPrimitives.empty())
    {
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
#endif
        vas->disableColorArray(state);
        for(Primitives::const_iterator itr = _decorationPrimitives.begin();
            itr != _decorationPrimitives.end();
            ++itr)
        {
            if ((*itr)->getMode()==GL_TRIANGLES) state.Color(colorMultiplier.r()*_textBBColor.r(), colorMultiplier.g()*_textBBColor.g(), colorMultiplier.b()*_textBBColor.b(), colorMultiplier.a()*_textBBColor.a());
            else state.Color(colorMultiplier.r(), colorMultiplier.g(), colorMultiplier.b(), colorMultiplier.a());

            (*itr)->draw(state, usingVertexBufferObjects);
        }
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
#endif
    }

    if (_drawMode & TEXT)
    {
        for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
            titr!=_textureGlyphQuadMap.end();
            ++titr)
        {
            // need to set the texture here...
            state.applyTextureAttribute(0,titr->first.get());

            const GlyphQuads& glyphquad = titr->second;

            if(_colorGradientMode == SOLID)
            {
                vas->disableColorArray(state);
                state.Color(colorMultiplier.r()*_color.r(),colorMultiplier.g()*_color.g(),colorMultiplier.b()*_color.b(),colorMultiplier.a()*_color.a());
            }
            else
            {
                if (requiresSetArrays)
                {
                    vas->setColorArray(state, _colorCoords.get());
                }
            }

            glyphquad._primitives->draw(state, usingVertexBufferObjects);
        }
    }
}


void Text::drawImplementation(osg::State& state, const osg::Vec4& colorMultiplier) const
{
    // save the previous modelview matrix
    osg::Matrix previous_modelview = state.getModelViewMatrix();

    // set up the new modelview matrix
    osg::Matrix modelview;
    bool needToApplyMatrix = computeMatrix(modelview, &state);

    if (needToApplyMatrix)
    {
        // ** mult previous by the modelview for this context
        modelview.postMult(previous_modelview);

        // ** apply this new modelview matrix
        state.applyModelViewMatrix(modelview);

        // workaround for GL3/GL2
        if (state.getUseModelViewAndProjectionUniforms()) state.applyModelViewAndProjectionUniformsIfRequired();

        // OSG_NOTICE<<"New state.applyModelViewMatrix() "<<modelview<<std::endl;
    }
    else
    {
        // OSG_NOTICE<<"No need to apply matrix "<<std::endl;
    }

    state.Normal(_normal.x(), _normal.y(), _normal.z());

    osg::VertexArrayState* vas = state.getCurrentVertexArrayState();
    bool usingVertexBufferObjects = state.useVertexBufferObject(_supportsVertexBufferObjects && _useVertexBufferObjects);
    bool usingVertexArrayObjects = usingVertexBufferObjects && state.useVertexArrayObject(_useVertexArrayObject);
    bool requiresSetArrays = !usingVertexBufferObjects || !usingVertexArrayObjects || vas->getRequiresSetArrays();

    if (requiresSetArrays)
    {
        vas->lazyDisablingOfVertexAttributes();
        vas->setVertexArray(state, _coords.get());
        vas->setTexCoordArray(state, 0, _texcoords.get());
        vas->applyDisablingOfVertexAttributes(state);
    }

    glDepthMask(GL_FALSE);

    drawImplementationSinglePass(state, colorMultiplier);

    if (_enableDepthWrites)
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_TRUE);

        drawImplementationSinglePass(state, colorMultiplier);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        state.haveAppliedAttribute(osg::StateAttribute::COLORMASK);
    }

    state.haveAppliedAttribute(osg::StateAttribute::DEPTH);

    if (usingVertexBufferObjects && !usingVertexArrayObjects)
    {
        // unbind the VBO's if any are used.
        vas->unbindVertexBufferObject();
        vas->unbindElementBufferObject();
    }

    if (needToApplyMatrix)
    {
        // apply this new modelview matrix
        state.applyModelViewMatrix(previous_modelview);

        // workaround for GL3/GL2
        if (state.getUseModelViewAndProjectionUniforms()) state.applyModelViewAndProjectionUniformsIfRequired();
    }
}



void Text::accept(osg::Drawable::ConstAttributeFunctor& af) const
{
    if (_coords.valid() && !_coords->empty())
    {
        af.apply(osg::Drawable::VERTICES, _coords->size(), &(_coords->front()));
    }

    if (_texcoords.valid() && !_texcoords->empty())
    {
        af.apply(osg::Drawable::TEXTURE_COORDS_0, _texcoords->size(), &(_texcoords->front()));
    }
}

void Text::accept(osg::PrimitiveFunctor& pf) const
{
    if (!_coords || _coords->empty()) return;

    // short term fix/workaround for _coords being transformed by a local matrix before rendering, so we need to replicate this was doing tasks like intersection testing.
    osg::ref_ptr<osg::Vec3Array> vertices = _coords;
    if (!_matrix.isIdentity())
    {
        vertices = new osg::Vec3Array;
        vertices->resize(_coords->size());
        for(Vec3Array::iterator sitr = _coords->begin(), ditr = vertices->begin();
            sitr != _coords->end();
            ++sitr, ++ditr)
        {
            *ditr = *sitr * _matrix;
        }
    }

    pf.setVertexArray(vertices->size(), &(vertices->front()));

    for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        const GlyphQuads& glyphquad = titr->second;
        if (glyphquad._primitives.valid())
        {
            const osg::DrawElementsUShort* drawElementsUShort = dynamic_cast<const osg::DrawElementsUShort*>(glyphquad._primitives.get());
            if (drawElementsUShort && drawElementsUShort->size() > 0)
            {
                pf.drawElements(GL_TRIANGLES, drawElementsUShort->size(), &(drawElementsUShort->front()));
            }
            else
            {
                const osg::DrawElementsUInt* drawElementsUInt = dynamic_cast<const osg::DrawElementsUInt*>(glyphquad._primitives.get());
                if (drawElementsUInt && drawElementsUInt->size() > 0)
                {
                    pf.drawElements(GL_TRIANGLES, drawElementsUInt->size(), &(drawElementsUInt->front()));
                }
            }
        }
    }
}

bool Text::getCharacterCorners(unsigned int index, osg::Vec3& bottomLeft, osg::Vec3& bottomRight, osg::Vec3& topLeft, osg::Vec3& topRight) const
{
    if (!_coords.valid()) return false;

    if ((index*4+4)>static_cast<unsigned int>(_coords->size())) return false;

    unsigned int base = index*4;
    topLeft = (*_coords)[base];
    bottomLeft = (*_coords)[base+1];
    bottomRight = (*_coords)[base+2];
    topRight = (*_coords)[base+3];

    return true;
}

void Text::resizeGLObjectBuffers(unsigned int maxSize)
{
    TextBase::resizeGLObjectBuffers(maxSize);

    for(TextureGlyphQuadMap::iterator itr = _textureGlyphQuadMap.begin();
        itr != _textureGlyphQuadMap.end();
        ++itr)
    {
        itr->second.resizeGLObjectBuffers(maxSize);
    }
}

void Text::releaseGLObjects(osg::State* state) const
{
    TextBase::releaseGLObjects(state);

    for(TextureGlyphQuadMap::const_iterator itr = _textureGlyphQuadMap.begin();
        itr != _textureGlyphQuadMap.end();
        ++itr)
    {
        itr->second.releaseGLObjects(state);
    }
}


void Text::setBackdropType(BackdropType type)
{
    if (_backdropType==type) return;

    _backdropType = type;

    assignStateSet();

    computeGlyphRepresentation();
}

void Text::setBackdropOffset(float offset)
{
    _backdropHorizontalOffset = offset;
    _backdropVerticalOffset = offset;

    assignStateSet();

    computeGlyphRepresentation();
}

void Text::setBackdropOffset(float horizontal, float vertical)
{
    _backdropHorizontalOffset = horizontal;
    _backdropVerticalOffset = vertical;

    assignStateSet();

    computeGlyphRepresentation();
}

void Text::setBackdropColor(const osg::Vec4& color)
{
    _backdropColor = color;

    assignStateSet();
}

void Text::setColorGradientMode(ColorGradientMode mode)
{
    if (_colorGradientMode==mode) return;

    _colorGradientMode = mode;
    computeGlyphRepresentation();
}

void Text::setColorGradientCorners(const osg::Vec4& topLeft, const osg::Vec4& bottomLeft, const osg::Vec4& bottomRight, const osg::Vec4& topRight)
{
    _colorGradientTopLeft = topLeft;
    _colorGradientBottomLeft = bottomLeft;
    _colorGradientBottomRight = bottomRight;
    _colorGradientTopRight = topRight;
    computeGlyphRepresentation();
}

// Formula for f(x,y) from Wikipedia "Bilinear interpolation", 2006-06-18
float Text::bilinearInterpolate(float x1, float x2, float y1, float y2, float x, float y, float q11, float q12, float q21, float q22) const
{
    return (
        ((q11 / ((x2-x1)*(y2-y1))) * (x2-x)*(y2-y))
        + ((q21 / ((x2-x1)*(y2-y1))) * (x-x1)*(y2-y))
        + ((q12 / ((x2-x1)*(y2-y1))) * (x2-x)*(y-y1))
        + ((q22 / ((x2-x1)*(y2-y1))) * (x-x1)*(y-y1))
    );
}

Text::GlyphQuads::GlyphQuads()
{
}

Text::GlyphQuads::GlyphQuads(const GlyphQuads&)
{
}

void Text::GlyphQuads::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_primitives.valid()) _primitives->resizeGLObjectBuffers(maxSize);
}

void Text::GlyphQuads::releaseGLObjects(osg::State* state) const
{
    if (_primitives.valid()) _primitives->releaseGLObjects(state);
}
