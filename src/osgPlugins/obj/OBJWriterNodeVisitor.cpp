// -*-c++-*-

/*
 * Wavefront OBJ loader for Open Scene Graph
 *
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * Modified by Robert Osfield to support per Drawable coord, normal and
 * texture coord arrays, bug fixes, and support for texture mapping.
 *
 * Writing support added 2007 by Stephan Huber, http://digitalmind.de,
 * some ideas taken from the dae-plugin
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#include <osg/io_utils>
#include "OBJWriterNodeVisitor.h"
#include <osgDB/WriteFile>


/** writes all values of an array out to a stream, applies a matrix beforehand if necessary */
class ValueVisitor : public osg::ValueVisitor {
    public:
        ValueVisitor(std::ostream& fout, const osg::Matrix& m = osg::Matrix::identity(), bool isNormal = false) :
            osg::ValueVisitor(),
            _fout(fout),
            _m(m),
            _isNormal(isNormal)
        {
            _applyMatrix = (_m != osg::Matrix::identity());
            if (_isNormal) _origin = osg::Vec3(0,0,0) * _m;
        }

        virtual void apply (osg::Vec2 & inv)
        {
            _fout << inv[0] << ' ' << inv[1];
        }

        virtual void apply (osg::Vec3 & inv)
        {
            osg::Vec3 v(inv);
            if (_applyMatrix)  v = (_isNormal) ? (v * _m) - _origin : v * _m;
            _fout << v[0] << ' ' << v[1] << ' ' << v[2];
        }

        virtual void apply (osg::Vec2b & inv)
        {
            _fout << inv[0] << ' ' << inv[1];
        }

        virtual void apply (osg::Vec3b & inv)
        {
            osg::Vec3 v(inv[0], inv[1], inv[2]);
            if (_applyMatrix)  v = (_isNormal) ? (v * _m) - _origin : v * _m;
            _fout << v[0] << ' ' << v[1] << ' ' << v[2];
        }

        virtual void apply (osg::Vec2s & inv)
        {
            _fout << inv[0] << ' ' << inv[1];
        }

        virtual void apply (osg::Vec3s & inv)
        {
            osg::Vec3 v(inv[0], inv[1], inv[2]);
            if (_applyMatrix)  v = (_isNormal) ? (v * _m) - _origin : v * _m;
            _fout << v[0] << ' ' << v[1] << ' ' << v[2];
        }

        //add Vec3dArray* vertex output to avoid inaccuracy
        virtual void apply(osg::Vec3d & inv)
        {
            osg::Vec3d v(inv[0], inv[1], inv[2]);
            osg::Vec3d orign_d((double)_origin.x(), (double)_origin.y(), (double)_origin.z());
            if (_applyMatrix)  v = (_isNormal) ? (v * _m) - orign_d : v * _m;

            //Setting 10-digit Significant Number
            _fout.precision(10);
            _fout << v[0] << ' ' << v[1] << ' ' << v[2];
        }

    private:

        ValueVisitor& operator = (const ValueVisitor&) { return *this; }

        std::ostream&    _fout;
        osg::Matrix        _m;
        bool            _applyMatrix, _isNormal;
        osg::Vec3        _origin;
};

/** writes all primitives of a primitive-set out to a stream, decomposes quads to triangles, line-strips to lines etc */
class ObjPrimitiveIndexWriter : public osg::PrimitiveIndexFunctor {

    public:
        ObjPrimitiveIndexWriter(std::ostream& fout,osg::Geometry* geo, unsigned int normalIndex, unsigned int lastVertexIndex, unsigned int  lastNormalIndex, unsigned int lastTexIndex) :
            osg::PrimitiveIndexFunctor(),
            _fout(fout),
            _modeCache(0),
            _lastVertexIndex(lastVertexIndex),
            _lastNormalIndex(lastNormalIndex),
            _lastTexIndex(lastTexIndex),
            _hasNormalCoords(geo->getNormalArray() != NULL),
            _hasTexCoords(geo->getTexCoordArray(0) != NULL),
            _geo(geo),
            _normalIndex(normalIndex)
        {
        }

        virtual void setVertexArray(unsigned int,const osg::Vec2*) {}

        virtual void setVertexArray(unsigned int ,const osg::Vec3* ) {}

        virtual void setVertexArray(unsigned int,const osg::Vec4* ) {}

        virtual void setVertexArray(unsigned int,const osg::Vec2d*) {}

        virtual void setVertexArray(unsigned int ,const osg::Vec3d* ) {}

        virtual void setVertexArray(unsigned int,const osg::Vec4d* ) {}

        void write(unsigned int i)
        {
            _fout << (i + _lastVertexIndex) << "/";

            if (_hasTexCoords || _hasNormalCoords)
            {
                if (_hasTexCoords)
                    _fout << (i + _lastTexIndex);
                _fout << "/";
                if (_hasNormalCoords)
                {
                    if (osg::getBinding(_geo->getNormalArray()) == osg::Array::BIND_PER_VERTEX)
                        _fout << (i+_lastNormalIndex);
                    else
                        _fout << (_normalIndex + _lastNormalIndex);
                }
            }
            _fout << " ";
        }

        // operator for triangles
        void writeTriangle(unsigned int i1, unsigned int i2, unsigned int i3)
         {
            _fout << "f ";
            write(i1);
            write(i2);
            write(i3);
            _fout << std::endl;
        }

        // operator for lines
        void writeLine(unsigned int i1, unsigned int i2)
        {
            _fout << "l ";
            write(i1);
            write(i2);
            _fout << std::endl;
        }

        // operator for points
        void writePoint(unsigned int i1)
        {
            _fout << "p ";
            write(i1);
            _fout << std::endl;
        }

        virtual void begin(GLenum mode)
        {
            _modeCache = mode;
            _indexCache.clear();
        }

        virtual void vertex(unsigned int vert)
        {
            _indexCache.push_back(vert);
        }

        virtual void end()
        {
            if (!_indexCache.empty())
            {
                drawElements(_modeCache,_indexCache.size(),&_indexCache.front());
            }
        }

        virtual void drawArrays(GLenum mode,GLint first,GLsizei count);

        virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
        {
            drawElementsImplementation<GLubyte>(mode, count, indices);
        }
        virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
        {
            drawElementsImplementation<GLushort>(mode, count, indices);
        }

        virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
        {
            drawElementsImplementation<GLuint>(mode, count, indices);
        }

    protected:

        template<typename T>void drawElementsImplementation(GLenum mode, GLsizei count, const T* indices)
        {
            if (indices==0 || count==0) return;

            typedef const T* IndexPointer;

            switch(mode)
            {
                case(GL_TRIANGLES):
                {
                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                        writeTriangle(*iptr,*(iptr+1),*(iptr+2));

                    break;
                }
                case(GL_TRIANGLE_STRIP):
                {
                    IndexPointer iptr = indices;
                    for(GLsizei i=2;i<count;++i,++iptr)
                    {
                        if ((i%2)) writeTriangle(*(iptr),*(iptr+2),*(iptr+1));
                        else       writeTriangle(*(iptr),*(iptr+1),*(iptr+2));
                    }
                    break;
                }
                case(GL_QUADS):
                {
                    IndexPointer iptr = indices;
                    for(GLsizei i=3;i<count;i+=4,iptr+=4)
                    {
                        writeTriangle(*(iptr),*(iptr+1),*(iptr+2));
                        writeTriangle(*(iptr),*(iptr+2),*(iptr+3));
                    }
                    break;
                }
                case(GL_QUAD_STRIP):
                {
                    IndexPointer iptr = indices;
                    for(GLsizei i=3;i<count;i+=2,iptr+=2)
                    {
                        writeTriangle(*(iptr),*(iptr+1),*(iptr+2));
                        writeTriangle(*(iptr+1),*(iptr+3),*(iptr+2));
                    }
                    break;
                }
                case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
                case(GL_TRIANGLE_FAN):
                {
                    IndexPointer iptr = indices;
                    unsigned int first = *iptr;
                    ++iptr;
                    for(GLsizei i=2;i<count;++i,++iptr)
                    {
                        writeTriangle(first,*(iptr),*(iptr+1));
                    }
                    break;
                }
                case(GL_POINTS):
                {
                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices;iptr<ilast;++iptr)

                    {
                        writePoint(*iptr);
                    }
                    break;
                }

                case(GL_LINES):
                {
                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices;iptr<ilast;iptr+=2)
                    {
                        writeLine(*iptr, *(iptr+1));
                    }
                    break;
                }
                case(GL_LINE_STRIP):
                {

                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices+1;iptr<ilast;iptr+=2)

                    {
                        writeLine(*(iptr-1), *iptr);
                    }
                    break;
                }
                case(GL_LINE_LOOP):
                {
                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices+1;iptr<ilast;iptr+=2)
                    {
                        writeLine(*(iptr-1), *iptr);
                    }
                    writeLine(*ilast, *indices);
                    break;
                }

                default:
                    // uhm should never come to this point :)
                    break;
            }
        }

    private:

        ObjPrimitiveIndexWriter& operator = (const ObjPrimitiveIndexWriter&) { return *this; }

        std::ostream&         _fout;
        GLenum               _modeCache;
        std::vector<GLuint>  _indexCache;
        unsigned int         _lastVertexIndex, _lastNormalIndex, _lastTexIndex;
        bool                 _hasNormalCoords, _hasTexCoords;
        osg::Geometry*         _geo;
        unsigned int         _normalIndex;
};


void ObjPrimitiveIndexWriter::drawArrays(GLenum mode,GLint first,GLsizei count)
{
    switch(mode)
    {
        case(GL_TRIANGLES):
        {
            unsigned int pos=first;
            for(GLsizei i=2;i<count;i+=3,pos+=3)
            {
                writeTriangle(pos,pos+1,pos+2);
            }
            break;
        }
        case(GL_TRIANGLE_STRIP):
         {
            unsigned int pos=first;
            for(GLsizei i=2;i<count;++i,++pos)
            {
                if ((i%2)) writeTriangle(pos,pos+2,pos+1);
                else       writeTriangle(pos,pos+1,pos+2);
            }
            break;
        }
        case(GL_QUADS):
        {
            unsigned int pos=first;
            for(GLsizei i=3;i<count;i+=4,pos+=4)
            {
                writeTriangle(pos,pos+1,pos+2);
                writeTriangle(pos,pos+2,pos+3);
            }
            break;
        }
        case(GL_QUAD_STRIP):
        {
            unsigned int pos=first;
            for(GLsizei i=3;i<count;i+=2,pos+=2)
            {
                writeTriangle(pos,pos+1,pos+2);
                writeTriangle(pos+1,pos+3,pos+2);
            }
            break;
        }
        case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
        case(GL_TRIANGLE_FAN):
        {
            unsigned int pos=first+1;
            for(GLsizei i=2;i<count;++i,++pos)
            {
                writeTriangle(first,pos,pos+1);
            }
            break;
        }
        case(GL_POINTS):
        {

            for(GLsizei i=0;i<count;++i)
            {
                writePoint(first + i);
            }
            break;
        }

        case(GL_LINES):
        {
            for(GLsizei i=0;i<count;i+=2)
            {
                writeLine(first + i, first + i+1);
            }
            break;
        }
        case(GL_LINE_STRIP):
        {
            for(GLsizei i=1;i<count;++i)
            {
                writeLine(first + i-1, first + i);
            }
            break;
        }
        case(GL_LINE_LOOP):
        {
            for(GLsizei i=1;i<count;++i)
            {
                writeLine(first + i-1, first + i);
            }
            writeLine(first + count-1, first + 0);
            break;
        }
        default:
            OSG_WARN << "OBJWriterNodeVisitor :: can't handle mode " << mode << std::endl;
            break;
    }
}


OBJWriterNodeVisitor::OBJMaterial::OBJMaterial(osg::Material* mat, osg::Texture* tex, bool outputTextureFiles, const osgDB::Options* options) :
    diffuse(1,1,1,1),
    ambient(0.2,0.2,0.2,1),
    specular(0,0,0,1),
    shininess(-1),
    image("")
{
    static unsigned int s_objmaterial_id = 0;
    ++s_objmaterial_id;
    std::stringstream ss;
    ss << "material_" << s_objmaterial_id;
    name = ss.str();

    if (mat) {
        diffuse = mat->getDiffuse(osg::Material::FRONT);
        ambient = mat->getAmbient(osg::Material::FRONT);
        specular = mat->getSpecular(osg::Material::FRONT);
        shininess = mat->getShininess(osg::Material::FRONT)*1000.0f/128.0f;
    }

    if (tex) {
        osg::Image* img = tex->getImage(0);
        if ((img) && (!img->getFileName().empty()))
        {
            image = img->getFileName();
            if (outputTextureFiles)
            {
                std::string imagePath = osgDB::getFilePath(image);
                if (!imagePath.empty() && !osgDB::fileExists(imagePath))
                {
                    osgDB::makeDirectory(imagePath);
                }

                osgDB::writeImageFile(*img, image, options);
            }
        }
    }

}

std::ostream& operator<<(std::ostream& fout, const OBJWriterNodeVisitor::OBJMaterial& mat) {

    fout << "newmtl " << mat.name << std::endl;
    fout << "       " << "Ka " << mat.ambient << std::endl;
    fout << "       " << "Kd " << mat.diffuse << std::endl;
    fout << "       " << "Ks " << mat.specular << std::endl;
    if (mat.shininess != -1)
        fout << "       " << "Ns " << mat.shininess<< std::endl;

    if(!mat.image.empty())
        fout << "       " << "map_Kd " << mat.image << std::endl;

    return fout;

}

void OBJWriterNodeVisitor::writeMaterials(std::ostream& fout)
{
    for(MaterialMap::iterator i = _materialMap.begin(); i != _materialMap.end(); ++i)
    {
        fout << (*i).second << std::endl;
    }
}


std::string OBJWriterNodeVisitor::getUniqueName(const std::string& defaultvalue) {

    std::string name = "";
    for(std::list<std::string>::iterator i = _nameStack.begin(); i != _nameStack.end(); ++i) {
        if (!name.empty()) name+="_";
        name += (*i);
    }

    if (!defaultvalue.empty())
        name += "_" +defaultvalue;

    if (_nameMap.find(name) == _nameMap.end())
        _nameMap.insert(std::make_pair(name, 0u));

    std::stringstream ss;
    ss << name << "_" << _nameMap[name];
    ++(_nameMap[name]);
    return ss.str();

}

void OBJWriterNodeVisitor::processArray(const std::string& key, osg::Array* array, const osg::Matrix& m, bool isNormal)
{
    if (array == NULL)
        return;

    ValueVisitor vv(_fout, m, isNormal);
    _fout << std::endl;
    for(unsigned int i = 0; i < array->getNumElements(); ++i) {
        _fout << key << ' ';
        array->accept(i, vv);
        _fout << std::endl;
    }

    _fout << "# " << array->getNumElements() << " elements written" << std::endl;

}

void OBJWriterNodeVisitor::processStateSet(osg::StateSet* ss)
{
    if (_materialMap.find(ss) != _materialMap.end()) {
        _fout << "usemtl " << _materialMap[ss].name << std::endl;
        return;
    }

    osg::Material* mat = dynamic_cast<osg::Material*>(ss->getAttribute(osg::StateAttribute::MATERIAL));
    osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));

    if (mat || tex)
    {
        _materialMap.insert(std::make_pair(osg::ref_ptr<osg::StateSet>(ss), OBJMaterial(mat, tex, _outputTextureFiles, _options.get())));
        _fout << "usemtl " << _materialMap[ss].name << std::endl;
    }

}


void OBJWriterNodeVisitor::processGeometry(osg::Geometry* geo, osg::Matrix& m) {
    _fout << std::endl;
    _fout << "o " << getUniqueName( geo->getName().empty() ? geo->className() : geo->getName() ) << std::endl;

    if (geo->containsDeprecatedData()) geo->fixDeprecatedData();

    processStateSet(_currentStateSet.get());

    processArray("v", geo->getVertexArray(), m, false);
    processArray("vn", geo->getNormalArray(), m, true);
    processArray("vt", geo->getTexCoordArray(0)); // we support only tex-unit 0
    unsigned int normalIndex = 0;
    for(unsigned int i = 0; i < geo->getNumPrimitiveSets(); ++i)
    {
        osg::PrimitiveSet* ps = geo->getPrimitiveSet(i);

        ObjPrimitiveIndexWriter pif(_fout, geo, normalIndex, _lastVertexIndex, _lastNormalIndex, _lastTexIndex);
        ps->accept(pif);

        if(geo->getNormalArray() && geo->getNormalArray()->getBinding() == osg::Array::BIND_PER_PRIMITIVE_SET)
            ++normalIndex;
    }
    if (geo->getVertexArray())
        _lastVertexIndex += geo->getVertexArray()->getNumElements();
    if (geo->getNormalArray())
        _lastNormalIndex += geo->getNormalArray()->getNumElements();
    if(geo->getTexCoordArray(0))
        _lastTexIndex += geo->getTexCoordArray(0)->getNumElements();

}

void OBJWriterNodeVisitor::apply(osg::Geometry& geometry)
{
    osg::Matrix m = osg::computeLocalToWorld(getNodePath());

    pushStateSet(geometry.getStateSet());

    processGeometry(&geometry,m);

    popStateSet(geometry.getStateSet());
}

void OBJWriterNodeVisitor::apply( osg::Geode &node )
{
    pushStateSet(node.getStateSet());
    _nameStack.push_back(node.getName());
    unsigned int count = node.getNumDrawables();
    for ( unsigned int i = 0; i < count; i++ )
    {
        node.getDrawable( i )->accept(*this);
    }

    popStateSet(node.getStateSet());
    _nameStack.pop_back();
}



