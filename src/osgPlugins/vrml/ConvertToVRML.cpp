/*
 *
 * OSG to VRML2 converter for OpenSceneGraph.
 *
 * authors :
 *           Johan Nouvel (johan_nouvel@yahoo.com)
 *
 *
 */

#include "ConvertToVRML.h"
#include <iostream>
#include <string.h>
#include <limits>
#include <osg/Quat>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/BoundingSphere>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osg/Image>

using namespace std;

/////////////////////////////////////////////////////////////////////////
//
// transformNode
//
/////////////////////////////////////////////////////////////////////////
osg::Node* transformNode(const osg::Node& root) {

  // create a zup to yup OSG Matrix
  osg::MatrixTransform* ret = new osg::MatrixTransform();
  osg::Matrix osgToVRMLMat(osg::Matrix(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0));

  ret->setDataVariance(osg::Object::STATIC);
  ret->setMatrix(osgToVRMLMat);

  if (strcmp(root.className(), "MatrixTransform") == 0) {
    const osg::MatrixTransform& aMat = static_cast<const osg::MatrixTransform&> (root);
    osg::MatrixTransform* node = new osg::MatrixTransform(aMat);
    ret->addChild(node);
  } else if (strcmp(root.className(), "Group") == 0) {
    const osg::Group& aGroup = static_cast<const osg::Group&> (root);
    osg::Group* node = new osg::Group(aGroup);
    ret->addChild(node);
  } else if (strcmp(root.className(), "PositionAttitudeTransform") == 0) {
    const osg::PositionAttitudeTransform& aPAT = static_cast<const osg::PositionAttitudeTransform&> (root);
    osg::PositionAttitudeTransform* node = new osg::PositionAttitudeTransform(aPAT);
    ret->addChild(node);
  } else if (strcmp(root.className(), "Geode") == 0) {
    const osg::Geode& aGeode = static_cast<const osg::Geode&> (root);
    osg::Geode* node = new osg::Geode(aGeode);
    ret->addChild(node);
  } else {
    osg::notify(osg::ALWAYS) << root.className() << " unsupported" << endl;
  }
  if (ret->getNumChildren() > 0) {
    return ret;
  }
  return (NULL);
}

/////////////////////////////////////////////////////////////////////////
//
// convertToVRML
//
/////////////////////////////////////////////////////////////////////////
osgDB::ReaderWriter::WriteResult convertToVRML(const osg::Node& root, const std::string& filename, const osgDB::ReaderWriter::Options* options) {

  ToVRML toVrml(filename, options);

  //cout << root.className() << endl;
  osg::Node* aRoot = transformNode(root);

  if (aRoot == NULL) {
    return (osgDB::ReaderWriter::WriteResult(osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED));
  }
  aRoot->accept(toVrml);

  return (toVrml.result());
}

/////////////////////////////////////////////////////////////////////////
//
// ToVRML
//
/////////////////////////////////////////////////////////////////////////
ToVRML::ToVRML(const std::string& fileName, const osgDB::ReaderWriter::Options* options) :
  osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {

  _fout.open(fileName.c_str(), ios::out);

  if (!_fout) {
    _res = osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED;
    return;
  }
  _res = osgDB::ReaderWriter::WriteResult::FILE_SAVED;

  _pathToOutput = osgDB::getFilePath(fileName.c_str());
  if (_pathToOutput == "") {
    _pathToOutput = ".";
  }
  //std::cout<<" path -"<<_pathToOutput<<"-"<<std::endl;

  _fout.precision(6);
  _fout.setf(ios::fixed, ios::floatfield);

  _strIndent = (char*) malloc(33);
  for (int i = 0; i < 33; i++) {
    _strIndent[i] = ' ';
  }
  _indent = 0;
  _strIndent[0] = '\0';

  //initialize
  _fout << "#VRML V2.0 utf8\n\n";

  _textureMode = CONVERT_TEXTURE;
  _txtUnit = 0;

  //_convertTextures = true;
  _pathRelativeToOutput = ".";
  if (options != NULL) {
    std::string opt = options->getOptionString();
    if (opt.find("convertTextures=0") != std::string::npos) {
      //std::cout << "Read XML stream" << std::endl;
      _textureMode = COPY_TEXTURE;

    } else if (opt.find("convertTextures=-1") != std::string::npos) {
      _textureMode = KEEP_ORIGINAL_TEXTURE;

    } else if (opt.find("convertTextures=-2") != std::string::npos) {
      _textureMode = NO_TEXTURE;

    } else if (opt.find("convertTextures=-3") != std::string::npos) {
      _textureMode = CONVERT_TEXTURE;
    }

    if (opt.find("directoryTextures=") != std::string::npos) {
      std::string dirOpt = opt;
      dirOpt.erase(0, dirOpt.find("directoryTextures=") + 18);
      _pathRelativeToOutput = dirOpt.substr(0, dirOpt.find(' '));
    }

    if (opt.find("textureUnit=") != std::string::npos) {
      std::string dirOpt = opt;
      dirOpt.erase(0, dirOpt.find("textureUnit=") + 12);
      _txtUnit = atoi(dirOpt.substr(0, dirOpt.find(' ')).c_str());
    }
  }

  if (_textureMode != NO_TEXTURE) {
    _defaultImage = new osg::Image();
    _defaultImage->setFileName(_pathToOutput + "/" + _pathRelativeToOutput + "/default_texture.png");
    int s = 1;
    int t = 1;
    unsigned char *data = new unsigned char[s * t * 3];
    data[0] = 255;
    data[1] = 255;
    data[2] = 255;
    _defaultImage->setImage(s, t, 0, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
    osgDB::writeImageFile(*(_defaultImage.get()), _defaultImage->getFileName());
  }
}

/////////////////////////////////////////////////////////////////////////
//
// ~ToVRML
//
/////////////////////////////////////////////////////////////////////////
ToVRML::~ToVRML() {
  if (_fout) {
    _fout.close();
  }
  free( _strIndent);

}

/////////////////////////////////////////////////////////////////////////
//
// indent
//
/////////////////////////////////////////////////////////////////////////
char* ToVRML::indent() {
  return (_strIndent);
}

/////////////////////////////////////////////////////////////////////////
//
// indentM
//
/////////////////////////////////////////////////////////////////////////
char* ToVRML::indentM() {
  _strIndent[_indent] = ' ';
  _indent += 2;

  if (_indent > 32) {
    _indent = 32;
  }
  _strIndent[_indent] = '\0';

  return (_strIndent);
}

/////////////////////////////////////////////////////////////////////////
//
// indentL
//
/////////////////////////////////////////////////////////////////////////
char* ToVRML::indentL() {
  _strIndent[_indent] = ' ';
  _indent -= 2;

  if (_indent < 0) {
    _indent = 0;
  }
  _strIndent[_indent] = '\0';

  return (_strIndent);
}

/////////////////////////////////////////////////////////////////////////
//
// apply(osg::Geode)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::Geode& node) {
  //cout << "Geode" << endl;

  pushStateSetStack(node.getStateSet());

  for (unsigned int i = 0; i < node.getNumDrawables(); i++) {
    apply(node.getDrawable(i));
  }
  popStateSetStack();
}

/////////////////////////////////////////////////////////////////////////
//
// apply
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::Drawable* drawable) {
  // get the drawable type
  //cout<<"ICI "<<drawable->className()<<endl;
  pushStateSetStack(drawable->getStateSet());

  if (strcmp(drawable->className(), "Geometry") == 0) {
    apply(drawable->asGeometry());
  } else {
    osg::notify(osg::ALWAYS) << "Drawable " << drawable->className() << " unsupported" << endl;
  }

  popStateSetStack();
}

/////////////////////////////////////////////////////////////////////////
//
// apply
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::Geometry* geom) {

  if (geom->containsDeprecatedData()) geom->fixDeprecatedData();

  // are all primitives faces or line ?
  GLenum mode;
  osg::PrimitiveSet::Type type;

  _fout << indent() << "Shape {\n";
  indentM();
  writeAppearance( getCurrentStateSet());
  indentL();

  int modePoints = 0;
  int modeLines = 0;
  int modeFaces = 0;

  for (unsigned int p = 0; p < geom->getNumPrimitiveSets(); p++) {
    mode = geom->getPrimitiveSet(p)->getMode();
    if (mode == osg::PrimitiveSet::POINTS) {
      modePoints++;

    } else if ((mode == osg::PrimitiveSet::LINES) || (mode == osg::PrimitiveSet::LINE_STRIP) || (mode == osg::PrimitiveSet::LINE_LOOP)) {
      modeLines++;

    } else if ((mode == osg::PrimitiveSet::TRIANGLES) || (mode == osg::PrimitiveSet::TRIANGLE_STRIP) || (mode == osg::PrimitiveSet::TRIANGLE_FAN) || (mode
    == osg::PrimitiveSet::QUADS) || (mode == osg::PrimitiveSet::QUAD_STRIP) || (mode == osg::PrimitiveSet::POLYGON)) {
      modeFaces++;
    }
  }

  //std::cout << "primitives type : Points " << modePoints << " Line " << modeLines << " Face " << modeFaces << std::endl;

  if (modePoints > 0) {
    _fout << indentM() << "geometry PointSet {\n";
    for (unsigned int p = 0; p < geom->getNumPrimitiveSets(); p++) {
      mode = geom->getPrimitiveSet(p)->getMode();
      type = geom->getPrimitiveSet(p)->getType();
      if (mode == osg::PrimitiveSet::POINTS) {
        osg::notify(osg::WARN) << " osg::PrimitiveSetMode = POINTS not supported by VRML Writer" << std::endl;
      }
    }
  }

  if (modeLines > 0) {
    _fout << indentM() << "geometry IndexedLineSet {\n";
    for (unsigned int p = 0; p < geom->getNumPrimitiveSets(); p++) {
      mode = geom->getPrimitiveSet(p)->getMode();
      type = geom->getPrimitiveSet(p)->getType();
      if ((mode == osg::PrimitiveSet::LINES) || (mode == osg::PrimitiveSet::LINE_STRIP) || (mode == osg::PrimitiveSet::LINE_LOOP)) {
        osg::notify(osg::WARN) << " osg::PrimitiveSetMode = LINES, LINE_STRIP or LINE_LOOP not supported by VRML Writer" << std::endl;
      }
    }
  }

  if (modeFaces > 0) {
    _fout << indentM() << "geometry IndexedFaceSet {\n";
    _fout << indentM() << "solid FALSE\n";
    indentL();
    _fout << indentM() << "coordIndex [\n";
    indentM();

    std::vector<int> primitiveSetFaces;
    int primitiveFaces = 0;

    for (unsigned int p = 0; p < geom->getNumPrimitiveSets(); p++) {
      mode = geom->getPrimitiveSet(p)->getMode();
      type = geom->getPrimitiveSet(p)->getType();
      if ((mode == osg::PrimitiveSet::TRIANGLES) || (mode == osg::PrimitiveSet::TRIANGLE_STRIP) || (mode == osg::PrimitiveSet::TRIANGLE_FAN) || (mode == osg::PrimitiveSet::QUADS)
      || (mode == osg::PrimitiveSet::QUAD_STRIP) || (mode == osg::PrimitiveSet::POLYGON)) {
        if (type == osg::PrimitiveSet::PrimitiveType) {
          osg::notify(osg::WARN) << "osg::PrimitiveSet::PrimitiveType not supported by VRML Writer" << std::endl;

        } else if (type == osg::PrimitiveSet::DrawArraysPrimitiveType) {
          //std::cout << "osg::PrimitiveSet::DrawArraysPrimitiveType" << std::endl;
          osg::ref_ptr < osg::DrawArrays > dra = dynamic_cast<osg::DrawArrays*> (geom->getPrimitiveSet(p));

          unsigned int* indices = new unsigned int[dra->getCount()];
          for (int j = 0; j < dra->getCount(); j++) {
            indices[j] = dra->getFirst() + j;
          }
          writeCoordIndex(mode, indices, dra->getCount(), primitiveSetFaces, primitiveFaces);
          delete[] indices;

        } else if (type == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType) {
          //osg::notify(osg::WARN) << " osg::PrimitiveSet::DrawArrayLengthsPrimitiveType not supported by VRML Writer" << std::endl;
          osg::ref_ptr < osg::DrawArrayLengths > dal = dynamic_cast<osg::DrawArrayLengths*> (geom->getPrimitiveSet(p));

          //std::cout<<dal->getFirst()<<" "<<dal->getNumPrimitives()<<" "<<   dal->getNumIndices ()<<" "<< dal->getNumInstances()<<std::endl;
          int first = 0;
          for (unsigned int prim = dal->getFirst(); prim < dal->getNumPrimitives(); prim++) {
            //std::cout<<(*dal)[prim]<<std::endl;
            unsigned int numIndices = (*dal)[prim];
            unsigned int * indices = new unsigned int[numIndices];
            for (unsigned int i = 0; i < numIndices; i++) {
              indices[i] = first + i;
            }

            first += (*dal)[prim];
            // write coorIndex as if wa have an indexed geometry
            writeCoordIndex(mode, indices, numIndices, primitiveSetFaces, primitiveFaces);
            delete[] indices;
          }

        } else if (type == osg::PrimitiveSet::DrawElementsUBytePrimitiveType) {
          //std::cout << "osg::PrimitiveSet::DrawElementsUBytePrimitiveType" << std::endl;

          osg::ref_ptr < osg::DrawElementsUByte > drui = dynamic_cast<osg::DrawElementsUByte*> (geom->getPrimitiveSet(p));
          const unsigned char * indices = (const unsigned char*) (drui->getDataPointer());
          writeCoordIndex(mode, indices, drui->getNumIndices(), primitiveSetFaces, primitiveFaces);

        } else if (type == osg::PrimitiveSet::DrawElementsUShortPrimitiveType) {
          //std::cout << "osg::PrimitiveSet::DrawElementsUShortPrimitiveType" << std::endl;

          osg::ref_ptr < osg::DrawElementsUShort > drui = dynamic_cast<osg::DrawElementsUShort*> (geom->getPrimitiveSet(p));
          const unsigned short * indices = (const unsigned short*) (drui->getDataPointer());
          writeCoordIndex(mode, indices, drui->getNumIndices(), primitiveSetFaces, primitiveFaces);

        } else if (type == osg::PrimitiveSet::DrawElementsUIntPrimitiveType) {
          //std::cout << "osg::PrimitiveSet::DrawElementsUIntPrimitiveType" << std::endl;

          osg::ref_ptr < osg::DrawElementsUInt > drui = dynamic_cast<osg::DrawElementsUInt*> (geom->getPrimitiveSet(p));
          const unsigned int * indices = (const unsigned int*) (drui->getDataPointer());
          writeCoordIndex(mode, indices, drui->getNumIndices(), primitiveSetFaces, primitiveFaces);
        }

      }

    }
    _fout << indentL() << "]\n";
    indentL();

    //write vertex
    writeCoord((osg::Vec3Array*) (geom->getVertexArray()));
    //write texture coordinate
    if (_textureMode != NO_TEXTURE) {
      writeTexCoord((osg::Vec2Array*) (geom->getTexCoordArray(_txtUnit)), (osg::Vec3Array*) (geom->getVertexArray()));
    }
    //write normals
    writeNormal(geom, primitiveSetFaces, primitiveFaces);
    //write colors
    writeColor(geom, primitiveSetFaces, primitiveFaces);
  }

  _fout << indent() << "}\n";
  _fout << indentL() << "}\n";

}

/////////////////////////////////////////////////////////////////////////
//
// writeAppearance
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::writeAppearance(osg::StateSet* stateset) {

  if (stateset == NULL) {
    return;
  }

  osg::ref_ptr < osg::StateSet > ss = stateset;

  _fout << indent() << "appearance Appearance {\n";
  osg::Material* mat = (osg::Material*) (ss->getAttribute(osg::StateAttribute::MATERIAL));
  osg::Texture2D* tex;
  if (_textureMode == NO_TEXTURE) {
    tex = NULL;
  } else {
    tex = (osg::Texture2D*) (ss->getTextureAttribute(_txtUnit, osg::StateAttribute::TEXTURE));
  }

  if (mat) {

    bool alreadyLoaded;
    std::string name;
    osg::Vec4 ambient, diffuse, specular, emission;
    float shininess;

    ambient = mat->getAmbient(osg::Material::FRONT);
    diffuse = mat->getDiffuse(osg::Material::FRONT);
    specular = mat->getSpecular(osg::Material::FRONT);
    shininess = mat->getShininess(osg::Material::FRONT);
    emission = mat->getEmission(osg::Material::FRONT);

    float transp = 1 - (ambient[3] + diffuse[3] + specular[3] + emission[3]) / 4.0;
    if (transp < 0)
      transp = 0;

    //if (transp == 0 && tex) {
    //  if (tex->getImage()->isImageTranslucent()) {
    //    transp = 0.01;
    //    mat->setTransparency(osg::Material::FRONT,transp);
    //  }
    //}

    findMaterialName(mat, name, alreadyLoaded);
    if (alreadyLoaded) {
      _fout << indentM() << "material USE " << name << "\n";
      indentL();
    } else {

      _fout << indentM() << "material DEF " << name << " Material {\n";
      //_fout << indentM() << "material Material {\n";
      _fout << indentM() << "diffuseColor " << diffuse[0] << " " << diffuse[1] << " " << diffuse[2] << "\n";
      _fout << indent() << "emissiveColor " << emission[0] << " " << emission[1] << " " << emission[2] << "\n";

      _fout << indent() << "specularColor " << specular[0] << " " << specular[1] << " " << specular[2] << "\n";
      _fout << indent() << "shininess " << shininess << "\n";
      float ambientI = (ambient[0] + ambient[1] + ambient[2]) / 3.0;
      if (ambientI > 1)
        ambientI = 1;
      _fout << indent() << "ambientIntensity " << ambientI << "\n";

      _fout << indent() << "transparency " << transp << "\n";
      _fout << indentL() << "}\n";
      indentL();
    }
  }

  if (tex) {
    bool alreadyLoaded;
    std::string name;
    findTextureName(tex, name, alreadyLoaded);
    if (alreadyLoaded) {
      _fout << indentM() << "texture USE " << name << "\n";
      indentL();
    } else {
      _fout << indentM() << "texture DEF " << name << " ImageTexture {\n";
      std::string texName = tex->getImage()->getFileName();
      if (_textureMode == COPY_TEXTURE || _textureMode == CONVERT_TEXTURE) {
        texName = _pathRelativeToOutput + "/" + osgDB::getSimpleFileName(texName);

      } else if (_textureMode == KEEP_ORIGINAL_TEXTURE) {
        texName = _pathRelativeToOutput + "/" + texName;
      }
      _fout << indentM() << "url [ \"" << texName << "\" ]\n";
      //std::cout<<"WRAP "<<tex->getWrap(osg::Texture::WRAP_S)<<std::endl;
      if (tex->getWrap(osg::Texture::WRAP_S) == osg::Texture::REPEAT) {
        _fout << indent() << "repeatS TRUE\n";
      } else {
        _fout << indent() << "repeatS FALSE\n";
      }
      if (tex->getWrap(osg::Texture::WRAP_T) == osg::Texture::REPEAT) {
        _fout << indent() << "repeatT TRUE\n";
      } else {
        _fout << indent() << "repeatT FALSE\n";
      }
      _fout << indentL() << "}\n";
      indentL();
    }
  }

  _fout << indent() << "}\n";

}

/////////////////////////////////////////////////////////////////////////
//
// apply(osg::Group)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::Group& node) {
  //cout << "Group" << endl;
  _fout << indent() << "Group {\n";
  _fout << indentM() << "children [\n";

  pushStateSetStack(node.getStateSet());

  indentM();
  traverse(node);
  indentL();

  popStateSetStack();

  _fout << indent() << "]\n";
  _fout << indentL() << "}\n";
}

/////////////////////////////////////////////////////////////////////////
//
// apply(osg::MatrixTransform)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::MatrixTransform& node) {
  //cout << "MatrixTransform" << endl;
  osg::Matrixf mat = node.getMatrix();
  //   for(int i=0;i<16;i++){
  //     cout <<mat.ptr()[i]<<" ";
  //   }
  //   cout<<endl;


  osg::Vec3 trans = mat.getTrans();
  osg::Vec3 scale = mat.getScale();
  osg::Quat quat;
  mat.get(quat);
  double angle;
  osg::Vec3 axe;
  quat.getRotate(angle, axe);

  _fout << indent() << "Transform {\n";
  _fout << indentM() << "scale " << scale[0] << " " << scale[1] << " " << scale[2] << "\n";
  //_fout << indentM() << "scale 1 1 1\n";
  _fout << indent() << "translation " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
  _fout << indent() << "rotation " << axe[0] << " " << axe[1] << " " << axe[2] << " " << angle << "\n";
  _fout << indent() << "children [\n";

  pushStateSetStack(node.getStateSet());

  indentM();
  traverse(node);
  indentL();

  popStateSetStack();

  _fout << indent() << "]\n";

  _fout << indentL() << "}\n";

}

/////////////////////////////////////////////////////////////////////////
//
// apply(osg::PositionAttitudeTransform)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::PositionAttitudeTransform& node) {

  osg::Vec3 trans = node.getPosition();
  osg::Vec3 scale = node.getScale();
  osg::Quat quat = node.getAttitude();
  double angle;
  osg::Vec3 axe;
  quat.getRotate(angle, axe);

  _fout << indent() << "Transform {\n";
  _fout << indentM() << "scale " << scale[0] << " " << scale[1] << " " << scale[2] << "\n";
  //_fout << indentM() << "scale 1 1 1\n";
  _fout << indent() << "translation " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
  _fout << indent() << "rotation " << axe[0] << " " << axe[1] << " " << axe[2] << " " << angle << "\n";
  _fout << indent() << "children [\n";

  pushStateSetStack(node.getStateSet());

  indentM();
  traverse(node);
  indentL();

  popStateSetStack();

  _fout << indent() << "]\n";

  _fout << indentL() << "}\n";

}

////////////////////////////////////////////////////////////////////////
//
// apply(osg::Billboard)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::Billboard& node) {
  osg::notify(osg::ALWAYS) << "WARNING : Billboard is changed to Group" << endl;
  _fout << indent() << "Group {\n";
  indentM();

  //osg::Billboard::Mode mode = node.getMode();
  //const osg::Vec3 axe = node.getAxis();
  //const osg::Vec3 normal = node.getNormal();

  //if (mode == osg::Billboard::AXIAL_ROT) {
  //  _fout << indent() << "axisOfRotation " << axe[0] << " " << axe[1] << " " << axe[2] << "\n";
  //}
  //osg::BoundingSphere sph=node.getBound();
  //_fout << indent() << "bboxCenter "<<sph.center()[0]<<" "<<sph.center()[1]<<" "<<sph.center()[2]<<"\n";
  //_fout << indent() << "bboxSize "<<sph.radius()<<" "<<sph.radius()<<" "<<sph.radius()<<"\n";

  _fout << indent() << "children [\n";

  pushStateSetStack(node.getStateSet());

  indentM();
  for (unsigned int i = 0; i < node.getNumDrawables(); i++) {
    osg::Vec3 trans = node.getPosition(i);
    _fout << indent() << "Transform {\n";
    _fout << indentM() << "translation " << trans[0] << " " << trans[1] << " " << trans[2] << "\n";
    _fout << indent() << "children [\n";
    indentM();
    apply(node.getDrawable(i));
    _fout << indentL() << "]\n";
    _fout << indentL() << "}\n";
  }

  popStateSetStack();

  _fout << indentL() << "]\n";
  _fout << indentL() << "}\n";
}

////////////////////////////////////////////////////////////////////////
//
// apply(osg::LOD)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::LOD& node) {
  //std::cout << "LOD" << endl;
  _fout << indent() << "LOD {\n";

  bool fakeFirst = false;
  bool fakeLast = false;
  if (node.getMinRange(0) > 0) {
    fakeFirst = true;
  }
  if (node.getMaxRange(node.getNumRanges() - 1) < std::numeric_limits<float>::max()) {
    fakeLast = true;
  }

  _fout << indentM() << "level [\n";
  if (fakeFirst) {
    _fout << indentM() << "Shape {\n";
    _fout << indent() << "}\n";
    indentL();
  }
  pushStateSetStack(node.getStateSet());
  indentM();
  traverse(node);
  indentL();
  popStateSetStack();

  if (fakeLast) {
    _fout << indentM() << "Shape {\n";
    _fout << indent() << "}\n";
    indentL();
  }

  _fout << indent() << "]\n";

  osg::Vec3 center;
  if (node.getCenterMode() == osg::LOD::USER_DEFINED_CENTER) {
    center = node.getCenter();
  } else {
    center = node.getBound().center();
  }
  _fout << indent() << "center " << center[0] << " " << center[1] << " " << center[2] << "\n";

  _fout << indentM() << "range [";
  if (fakeFirst) {
    _fout << indentM() << node.getMinRange(0) << ",";
  }
  for (unsigned int i = 0; i < node.getNumRanges(); i++) {

    _fout << indentM() << node.getMaxRange(i);
    if (i < node.getNumRanges() - 1) {
      _fout << ",";
    }
  }
  _fout << "]\n";

  _fout << indentL() << "}\n";
}

/////////////////////////////////////////////////////////////////////////
//
// apply(osg::Node)
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::apply(osg::Node& node) {
  osg::notify(osg::ALWAYS) << node.className() << " not supported" << endl;
  pushStateSetStack(node.getStateSet());
  traverse(node);
  popStateSetStack();
}

/////////////////////////////////////////////////////////////////////////
//
// pushStateSetStack
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::pushStateSetStack(osg::StateSet* ss) {
  _stack.push_back(osg::ref_ptr<osg::StateSet>(ss));
}

/////////////////////////////////////////////////////////////////////////
//
// popStateSetStack
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::popStateSetStack() {
  if (!_stack.empty()) {
    _stack.pop_back();
  }
}

/////////////////////////////////////////////////////////////////////////
//
// getCurrentStateSet
//
/////////////////////////////////////////////////////////////////////////
osg::StateSet* ToVRML::getCurrentStateSet() {
  // return current StateSet by flatten StateSet stack.
  // until flatten is done, just return top StateSet
  if (!_stack.empty()) {
    osg::StateSet* ssFinal = new osg::StateSet();
    ssFinal->setGlobalDefaults();
    //std::cout << "StateSet Stack Size " << _stack.size() << std::endl;
    for (unsigned int i = 0; i < _stack.size(); i++) {
      osg::StateSet* ssCur = _stack[i].get();
      if (ssCur != NULL) {
        ssFinal->merge(*ssCur);
      }
    }
    return (ssFinal);
  } else
    return (NULL);
}

/////////////////////////////////////////////////////////////////////////
//
// findMaterialName
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::findMaterialName(osg::Material* mat, std::string& name, bool& alreadyLoaded) {

  osg::Vec4 ambient, diffuse, specular, emission;
  //float shininess;
  std::map<osg::ref_ptr<osg::Material>, std::string>::iterator it;// = _matMap.find(obj);

  for (it = _matMap.begin(); it != _matMap.end(); it++) {
    osg::ref_ptr < osg::Material > matLoaded = it->first;
    ambient = mat->getAmbient(osg::Material::FRONT);
    //diffuse = mat->getDiffuse(osg::Material::FRONT);
    //specular = mat->getSpecular(osg::Material::FRONT);
    //shininess = mat->getShininess(osg::Material::FRONT);
    //emission = mat->getEmission(osg::Material::FRONT);
    //std::cout << "MATERIAL " << ambient[3] << " " << matLoaded->getAmbient(osg::Material::FRONT)[3] << std::endl;

    if ((matLoaded->getAmbient(osg::Material::FRONT) == mat->getAmbient(osg::Material::FRONT)) && (matLoaded->getDiffuse(osg::Material::FRONT)
    == mat->getDiffuse(osg::Material::FRONT)) && (matLoaded->getSpecular(osg::Material::FRONT) == mat->getSpecular(osg::Material::FRONT))
    && (matLoaded->getShininess(osg::Material::FRONT) == mat->getShininess(osg::Material::FRONT)) && (matLoaded->getEmission(osg::Material::FRONT)
    == mat->getEmission(osg::Material::FRONT))) {

      name = it->second;
      alreadyLoaded = true;
      return;
    }
  }
  std::stringstream ss;
  ss << "material_" << _matMap.size();
  name = ss.str();
  _matMap[mat] = name;
  alreadyLoaded = false;

}

/////////////////////////////////////////////////////////////////////////
//
// findTextureName
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::findTextureName(osg::Texture2D* tex, std::string& name, bool& alreadyLoaded) {

  //std::cout << "ORI -" << tex->getImage() << "- " << std::endl;
  if (tex->getImage() == NULL) {
    tex->setImage(_defaultImage.get());
  }

  std::string nameOri = osgDB::convertFileNameToNativeStyle(tex->getImage()->getFileName());
  std::string nameInput = osgDB::findDataFile(nameOri, NULL);
  if (nameInput == "") {
    osg::notify(osg::ALWAYS) << "WARNING: couldn't find texture named: -" << nameOri << "- use default one instead -" << _defaultImage->getFileName() << "-" << std::endl;
    tex->setImage(_defaultImage.get());
    nameOri = osgDB::convertFileNameToNativeStyle(tex->getImage()->getFileName());
    nameInput = osgDB::findDataFile(nameOri, NULL);
  }

  std::string nameOutput;
  std::string::size_type pos = nameInput.find_last_of(".");
  std::string ext = nameInput.substr(pos, nameInput.length() - pos);

  osg::ref_ptr < osg::Image > imgIn = osgDB::readImageFile(nameInput);
  if (!imgIn.valid()) {
    osg::notify(osg::ALWAYS) << "WARNING: couldn't read texture named: -" << nameOri << "- use default one instead -" << _defaultImage->getFileName() << "-" << std::endl;
    tex->setImage(_defaultImage.get());
    nameOri = osgDB::convertFileNameToNativeStyle(tex->getImage()->getFileName());
    nameInput = osgDB::findDataFile(nameOri, NULL);
    pos = nameInput.find_last_of(".");
    nameInput.substr(pos, nameInput.length() - pos);
    imgIn = _defaultImage.get();
  }

  //int nbBand = poSrcDS->GetRasterCount();
  if (_textureMode == KEEP_ORIGINAL_TEXTURE) {
    nameOutput = nameInput;

  } else if (_textureMode == COPY_TEXTURE) {
    nameOutput = _pathToOutput + "/" + _pathRelativeToOutput + "/" + osgDB::getSimpleFileName(nameInput);

  } else if (_textureMode == CONVERT_TEXTURE) {
    nameOutput = _pathToOutput + "/" + _pathRelativeToOutput + "/" + osgDB::getSimpleFileName(nameInput);
    if (ext != ".png" && ext != ".jpg") {
      if (imgIn->isImageTranslucent()) {//(nbBand != 1 && nbBand != 3) {
        nameOutput = _pathToOutput + "/" + _pathRelativeToOutput + "/" + osgDB::getSimpleFileName(nameInput.substr(0, pos) + ".png");
      } else {
        nameOutput = _pathToOutput + "/" + _pathRelativeToOutput + "/" + osgDB::getSimpleFileName(nameInput.substr(0, pos) + ".jpg");
      }
    }
  }

  std::map<osg::ref_ptr<osg::Texture2D>, std::string>::iterator it;
  for (it = _texMap.begin(); it != _texMap.end(); it++) {
    osg::ref_ptr < osg::Texture2D > texLoaded = it->first;

    if ((nameOutput == texLoaded->getImage()->getFileName()) && (tex->getWrap(osg::Texture::WRAP_S) == texLoaded->getWrap(osg::Texture::WRAP_S))
    && (tex->getWrap(osg::Texture::WRAP_T) == texLoaded->getWrap(osg::Texture::WRAP_T))) {
      name = it->second;
      alreadyLoaded = true;
      return;
    }
  }

  // first time we see these texture
  // add it to textureMap
  std::stringstream ss;
  ss << "texture2D_" << _texMap.size();
  name = ss.str();
  _texMap[tex] = name;
  alreadyLoaded = false;

  tex->getImage()->setFileName(nameOutput);

  if (nameInput == nameOutput) {
    return;
  }
  // else write image file

  if (_textureMode == CONVERT_TEXTURE) {
    // convert format (from dds to rgb or rgba, or from rgba to rgb)
    std::string newName;
    GLenum type = GL_UNSIGNED_BYTE;
    GLenum pixelFormat;
    GLint internalFormat;
    int nbC;
    if (imgIn->isImageTranslucent()) {
      // convert to png if transparency
      pixelFormat = GL_RGBA;
      internalFormat = GL_RGBA;
      nbC = 4;
    } else {
      // convert to jpeg if no transparency
      pixelFormat = GL_RGB;
      internalFormat = GL_RGB;
      nbC = 3;
    }
    osg::ref_ptr < osg::Image > curImg = tex->getImage();
    osg::ref_ptr < osg::Image > mmImg = new osg::Image();
    unsigned char* data = new unsigned char[curImg->s() * curImg->t() * nbC];
    for (int j = 0; j < curImg->t(); j++) {
      for (int i = 0; i < curImg->s(); i++) {
        osg::Vec4 c = curImg->getColor(i, j);
        data[(i + j * curImg->s()) * nbC] = c[0] * 255;
        data[(i + j * curImg->s()) * nbC + 1] = c[1] * 255;
        data[(i + j * curImg->s()) * nbC + 2] = c[2] * 255;
        if (nbC == 4) {
          data[(i + j * curImg->s()) * nbC + 3] = c[3] * 255;
        }
      }
    }
    mmImg->setImage(curImg->s(), curImg->t(), 1, internalFormat, pixelFormat, type, data, osg::Image::USE_NEW_DELETE, 1);
    osgDB::writeImageFile(*(mmImg.get()), nameOutput);

  } else {
    // no conversion needed, input and output have the same format, but path to image has changed
    osgDB::writeImageFile(*(tex->getImage()), nameOutput);
  }

}

/////////////////////////////////////////////////////////////////////////
//
// writeCoordIndex
//
/////////////////////////////////////////////////////////////////////////
template<typename T> void ToVRML::writeCoordIndex(GLenum mode, T* indices, unsigned int number, std::vector<int>& primitiveSetFaces, int& primitiveFaces) {

  int currentFaces = 0;
  if (mode == osg::PrimitiveSet::TRIANGLES) {
    //std::cout << "TRIANGLES" << std::endl;
    for (unsigned int j = 0; j < number; j += 3) {
      _fout << indent() << indices[j] << "," << indices[j + 1] << "," << indices[j + 2] << ",-1,\n";
      currentFaces++;
    }
    primitiveSetFaces.push_back(currentFaces);
    primitiveFaces += currentFaces;

  } else if (mode == osg::PrimitiveSet::TRIANGLE_FAN) {
    //std::cout << "TRIANGLE FAN" << std::endl;
    int firstPoint = indices[0];
    int secondPoint = indices[1];
    for (unsigned int j = 2; j < number; j++) {
      _fout << indent() << firstPoint << "," << secondPoint << "," << indices[j] << ",-1,\n";
      secondPoint = indices[j];
      currentFaces++;
    }
    primitiveSetFaces.push_back(currentFaces);
    primitiveFaces += currentFaces;

  } else if (mode == osg::PrimitiveSet::TRIANGLE_STRIP) {
    //std::cout << "TRIANGLE STRIP" << std::endl;
    int firstPoint = indices[0];
    int secondPoint = indices[1];
    bool changeSecond = false;
    for (unsigned int j = 2; j < number; j++) {
      _fout << indent() << firstPoint << "," << secondPoint << "," << indices[j] << ",-1,\n";
      if (!changeSecond) {
        firstPoint = indices[j];
        changeSecond = true;
      } else {
        secondPoint = indices[j];
        changeSecond = false;
      }
      currentFaces++;
    }
    primitiveSetFaces.push_back(currentFaces);
    primitiveFaces += currentFaces;

  } else if (mode == osg::PrimitiveSet::QUADS) {
    //std::cout << "QUADS" << std::endl;
    for (unsigned int j = 0; j < number; j += 4) {
      _fout << indent() << indices[j] << "," << indices[j + 1] << "," << indices[j + 2] << "," << indices[j + 3] << ",-1,\n";
      currentFaces++;
    }
    primitiveSetFaces.push_back(currentFaces);
    primitiveFaces += currentFaces;

  } else if (mode == osg::PrimitiveSet::QUAD_STRIP) {
    //std::cout << "QUADS STRIP" << std::endl;
    int firstPoint = indices[0];
    int secondPoint = indices[1];
    for (unsigned int j = 2; j < number; j += 2) {
      _fout << indent() << firstPoint << "," << secondPoint << "," << indices[j + 1] << " " << indices[j] << ",-1,\n";
      firstPoint = indices[j];
      secondPoint = indices[j + 1];
      currentFaces++;
    }
    primitiveSetFaces.push_back(currentFaces);
    primitiveFaces += currentFaces;

  } else if (mode == osg::PrimitiveSet::POLYGON) {
    //std::cout << "POLYGON" << std::endl;
    _fout << indent();
    for (unsigned int j = 0; j < number; j++) {
      _fout << indices[j] << ",";
    }
    currentFaces++;
    _fout << "-1,\n";
    primitiveSetFaces.push_back(currentFaces);
    primitiveFaces += currentFaces;

  } else {
    osg::notify(osg::ALWAYS) << "Unknown Primitive Set Type : " << mode << std::endl;
  }

}

/////////////////////////////////////////////////////////////////////////
//
// writeCoord
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::writeCoord(osg::Vec3Array* array) {

  if (array == NULL) {
    std::cerr << "Vec3Array is NULL" << std::endl;
    return;
  }
  osg::ref_ptr < osg::Vec3Array > vArray = array;

  _fout << indentM() << "coord Coordinate {\n";
  _fout << indentM() << "point [\n";
  indentM();
  osg::Vec3 xyz;
  for (unsigned int j = 0; j < vArray->size(); j++) {
    xyz = (*vArray)[j];
    _fout << indent() << xyz[0] << " " << xyz[1] << " " << xyz[2] << ",\n";
  }
  _fout << indentL() << "]\n";
  _fout << indentL() << "}\n";
  indentL();
}

/////////////////////////////////////////////////////////////////////////
//
// writeTexCoord
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::writeTexCoord(osg::Vec2Array* array, osg::Vec3Array* array2) {

  osg::ref_ptr < osg::StateSet > ss = getCurrentStateSet();
  osg::Texture2D* tex = (osg::Texture2D*) (ss->getTextureAttribute(_txtUnit, osg::StateAttribute::TEXTURE));
  if (tex == NULL) {
    return;
  }

  osg::ref_ptr < osg::TexGen > texGen = dynamic_cast<osg::TexGen*> (ss->getTextureAttribute(_txtUnit, osg::StateAttribute::TEXGEN));
  osg::ref_ptr < osg::TexEnv > texEnv = dynamic_cast<osg::TexEnv*> (ss->getTextureAttribute(_txtUnit, osg::StateAttribute::TEXENV));

  osg::Texture::WrapMode wrap_s = tex->getWrap(osg::Texture::WRAP_S);//osg::Texture::REPEAT;
  osg::Texture::WrapMode wrap_t = tex->getWrap(osg::Texture::WRAP_T);//osg::Texture::REPEAT;

  if (array != NULL) {
    writeUVArray(array, wrap_s, wrap_t);

  } else if (texGen.valid()) {
    //std::cout << "TEXGEN" << std::endl;
    osg::ref_ptr < osg::Vec2Array > uvArray = buildUVArray(texGen.get(), array2);
    if (uvArray.valid()) {
      writeUVArray(uvArray.get(), wrap_s, wrap_t);
    }

  } else if (texEnv.valid()) {
    //std::cout << "TEXENV" << std::endl;
    osg::ref_ptr < osg::Vec2Array > uvArray = buildUVArray(texEnv.get(), array2);
    if (uvArray.valid()) {
      writeUVArray(uvArray.get(), wrap_s, wrap_t);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
//
// writeNormal
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::writeNormal(osg::Geometry* geom, std::vector<int>& primitiveSetFaces, int nbFaces) {

  osg::ref_ptr < osg::Vec3Array > nArray = (osg::Vec3Array*) (geom->getNormalArray());
  if (!nArray.valid()) {
    return;
  }

  osg::Array::Binding normalBinding = osg::getBinding(geom->getNormalArray());
  if (normalBinding == osg::Array::BIND_PER_VERTEX || normalBinding == osg::Array::BIND_OVERALL) {
    _fout << indentM() << "normalPerVertex TRUE \n";
  } else {
    _fout << indentM() << "normalPerVertex FALSE \n";
  }

  _fout << indent() << "normal Normal {\n";
  _fout << indentM() << "vector [\n";
  indentM();
  osg::Vec3 n;

  if (normalBinding == osg::Array::BIND_PER_VERTEX) {
    for (unsigned int j = 0; j < (*nArray).size(); j++) {
      n = (*nArray)[j];
      _fout << indent() << n[0] << " " << n[1] << " " << n[2] << ",\n";
    }

  } else if (normalBinding == osg::Array::BIND_OVERALL) {
    n = (*nArray)[0];
    int size = ((osg::Vec3Array*) (geom->getVertexArray()))->size();
    for (int j = 0; j < size; j++) {
      _fout << indent() << n[0] << " " << n[1] << " " << n[2] << ",\n";
    }

  } else if (normalBinding == osg::Array::BIND_PER_PRIMITIVE_SET) {
    for (unsigned int j = 0; j < (*nArray).size(); j++) {
      n = (*nArray)[j];
      for (int k = 0; k < primitiveSetFaces[j]; k++) {
        _fout << indent() << n[0] << " " << n[1] << " " << n[2] << ",\n";
      }
    }
  }

  _fout << indentL() << "]\n";
  _fout << indentL() << "}\n";
  indentL();
}

/////////////////////////////////////////////////////////////////////////
//
// writeUVArray
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::writeUVArray(osg::Vec2Array* uvArray, osg::Texture::WrapMode wrap_s, osg::Texture::WrapMode wrap_t) {

  _fout << indentM() << "texCoord TextureCoordinate {\n";
  _fout << indentM() << "point [\n";
  indentM();
  osg::Vec2 uv;

  for (unsigned int j = 0; j < (*uvArray).size(); j++) {
    uv = (*uvArray)[j];
    if (wrap_s != osg::Texture::REPEAT) {
      // clamp to 0-1
      if (uv[0] < 0) {
        uv[0] = 0;
      } else if (uv[0] > 1) {
        uv[0] = 1;
      }
    }
    if (wrap_t != osg::Texture::REPEAT) {
      // clamp to 0-1
      if (uv[1] < 0) {
        uv[1] = 0;
      } else if (uv[1] > 1) {
        uv[1] = 1;
      }
    }
    _fout << indent() << uv[0] << " " << uv[1] << ",\n";
  }
  _fout << indentL() << "]\n";
  _fout << indentL() << "}\n";
  indentL();
}

/////////////////////////////////////////////////////////////////////////
//
// buildUVArray
//
/////////////////////////////////////////////////////////////////////////
osg::Vec2Array* ToVRML::buildUVArray(osg::TexGen* tGen, osg::Vec3Array* array) {
  osg::ref_ptr < osg::TexGen > texGen = tGen;
  osg::ref_ptr < osg::Vec3Array > vArray = array;
  osg::Vec2Array* uvRet = NULL;

  osg::TexGen::Mode mode = texGen->getMode();
  if (mode == osg::TexGen::OBJECT_LINEAR) {
    //std::cout << "I know" << std::endl;

    uvRet = new osg::Vec2Array();
    osg::Plane planS = texGen->getPlane(osg::TexGen::S);//, osg::Vec4(1 / rangeS, 0, 0, -(adfGeoTransform[0] / rangeS)));
    osg::Plane planT = texGen->getPlane(osg::TexGen::T);//, osg::Vec4(0, 1 / rangeT, 0, -(adfGeoTransform[3] + sy * adfGeoTransform[5]) / rangeT));
    osg::Vec4 pS = planS.asVec4();
    osg::Vec4 pT = planT.asVec4();
    double width, height, xmin, ymin;
    width = 1.0 / pS[0];
    height = 1.0 / pT[1];
    xmin = -width * pS[3];
    ymin = -height * pT[3];

    for (unsigned int j = 0; j < vArray->size(); j++) {
      osg::Vec3d p = (*vArray)[j];
      double x, y;
      osg::Vec2 uv;
      x = p[0];
      y = p[1];
      uv[0] = (x - xmin) / width;
      uv[1] = (y - ymin) / height;
      (*uvRet).push_back(uv);

    }
  } else {
    osg::notify(osg::ALWAYS) << "Unknown TexGen mode" << std::endl;
  }

  return (uvRet);
}

/////////////////////////////////////////////////////////////////////////
//
// buildUVArray
//
/////////////////////////////////////////////////////////////////////////
osg::Vec2Array* ToVRML::buildUVArray(osg::TexEnv* tEnv, osg::Vec3Array* array) {
  osg::ref_ptr < osg::TexEnv > texEnv = tEnv;
  osg::Vec2Array* uvRet = NULL;

 // osg::TexEnv::Mode mode = texEnv->getMode();
  //if (mode == osg::TexEnv::MODULATE) {
  //  std::cout << "I know" << std::endl;
  //} else {
  osg::notify(osg::ALWAYS) << "Unknown TexEnv mode" << std::endl;
  //}
  return (uvRet);
}

/////////////////////////////////////////////////////////////////////////
//
// writeColor
//
/////////////////////////////////////////////////////////////////////////
void ToVRML::writeColor(osg::Geometry* geom, std::vector<int>& primitiveSetFaces, int nbFaces) {

  osg::ref_ptr < osg::Vec4Array > cArray = (osg::Vec4Array*) (geom->getColorArray());
  if (!cArray.valid()) {
    return;
  }

  osg::Array::Binding colorBinding = osg::getBinding(geom->getColorArray());
  if (colorBinding == osg::Array::BIND_PER_VERTEX || colorBinding == osg::Array::BIND_OVERALL) {
    _fout << indentM() << "colorPerVertex TRUE \n";
  } else {
    _fout << indentM() << "colorPerVertex FALSE \n";
  }

  _fout << indent() << "color Color {\n";
  _fout << indentM() << "color [\n";
  indentM();
  osg::Vec4 c;

  if (colorBinding == osg::Array::BIND_PER_VERTEX) {
    for (unsigned int j = 0; j < (*cArray).size(); j++) {
      c = (*cArray)[j];
      _fout << indent() << c[0] << " " << c[1] << " " << c[2] << ",\n";
    }

  } else if (colorBinding == osg::Array::BIND_OVERALL) {
    c = (*cArray)[0];
    int size = ((osg::Vec3Array*) (geom->getVertexArray()))->size();
    for (int j = 0; j < size; j++) {
      _fout << indent() << c[0] << " " << c[1] << " " << c[2] << ",\n";
    }

  } else if (colorBinding == osg::Array::BIND_PER_PRIMITIVE_SET) {
    for (unsigned int j = 0; j < (*cArray).size(); j++) {
      c = (*cArray)[j];
      for (int k = 0; k < primitiveSetFaces[j]; k++) {
        _fout << indent() << c[0] << " " << c[1] << " " << c[2] << ",\n";
      }
    }
  }

  _fout << indentL() << "]\n";
  _fout << indentL() << "}\n";
  indentL();
}
