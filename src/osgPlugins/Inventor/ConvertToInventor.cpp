//
//  ConvertToInventor converts OSG scene graph to Inventor or VRML 1 scene graph
//
//  It requires OSG and Inventor compatible library, such as Coin,
//  SGI Inventor , or TGS Inventor.
//
//
//  Autor: PCJohn (peciva _at fit.vutbr.cz)
//
//  License: public domain
//
//
//  THIS SOFTWARE IS NOT COPYRIGHTED
//
//  This source code is offered for use in the public domain.
//  You may use, modify or distribute it freely.
//
//  This source code is distributed in the hope that it will be useful but
//  WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
//  DISCLAIMED.  This includes but is not limited to warranties of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  If you find the source code useful, authors will kindly welcome
//  if you give them credit and keep their names with their source code,
//  but you are not forced to do so.
//

#include <osg/BlendFunc>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LightModel>
#include <osg/LOD>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/PositionAttitudeTransform>
#include <osg/PrimitiveSet>
#include <osg/ShapeDrawable>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCoordinate4.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoIndexedTriangleStripSet.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoLOD.h>
#include <Inventor/nodes/SoLevelOfDetail.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoTextureCoordinateEnvironment.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoTriangleStripSet.h>
#include <Inventor/fields/SoFields.h>
#ifdef __COIN__
#include <Inventor/nodes/SoTextureCoordinate3.h>
#include <Inventor/nodes/SoTransparencyType.h>
#include <Inventor/VRMLnodes/SoVRMLBillboard.h>
#endif
#include "ConvertToInventor.h"

#include <assert.h>


#define DEBUG_IV_WRITER 1


// Considerations for VRML1 support:
//
// SoSeparator:                supported (only renderCulling field)
// SoCoordinate3:              supported
// SoCoordinate4:              no -> conversion to Coordinate3 required <---- missing
// SoTextureCoordinate2:       supported
// SoTextureCoordinate3:       no - 3D textures not supported by VRML1
// SoTextureCoordinateEnvironment no <- should texturing be disabled in this case?
// SoTextureCoordinateBinding: no -> indexing is probably mandatory
// SoNormal:                   supported
// SoNormalBinding:            supported (all modes supported, including non-indexed modes)
// SoMaterial:                 supported
// SoMaterialBinding:          supported (all modes supported, including non-indexed modes)
// SoBaseColor:                no -> using SoMaterial instead
// SoPointSet:                 supported
// SoLineSet:                  no -> using SoIndexedListSet instead
// SoIndexedLineSet:           supported
// SoIndexedTriangleStripSet:  no -> using SoIndexedFaceSet instead
// SoTriangleStripSet:         no -> using SoIndexedFaceSet instead
// SoMatrixTransform:          supported
// SoTransform:                supported
// SoTransparencyType:         no   <---- missing
// SoShapeHints:               supported
// SoCone,SoCube,SoCylinder,SoSphere supported
// SoTranslation               supported
// SoLightModel:               ?
// SoLOD:                      supported
// SoLevelOfDetail:            no


// Node list from VRML1 documentation:
// shape nodes: AsciiText, Cone, Cube, Cylinder, IndexedFaceSet, IndexedLineSet, PointSet, Sphere
// properties nodes: Coordinate3, FontStyle, Info, Material, MaterialBinding, Normal, NormalBinding,
//                   Texture2, Texture2Transform, TextureCoordinate2, ShapeHints
// transformantion nodes: MatrixTransform, Rotation, Scale, Transform, Translation
// group nodes: Separator, Switch, WWWAnchor, LOD
// cameras: OrthographicCamera, PerspectiveCamera
// lights: DirectionalLight, PointLight, SpotLight
// others: WWWInline



ConvertToInventor::ConvertToInventor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
  // Enable/disable using of some extensions introduced by TGS and Coin.
  //
  // For instance, GL_REPLACE texturing mode is extension
  // and it will not be used if extensions are not enabled.
#ifdef __COIN__
  useIvExtensions = true;
#else
  useIvExtensions = false;
#endif
  vrml1Conversion = false;
  uniqueIdGenerator = 1;

  // Scene root
  ivRoot = new SoSeparator;
  ivRoot->ref();

  // OSG -> Inventor coordinate system conversion
  SoMatrixTransform *mt = new SoMatrixTransform;
  mt->matrix.setValue(1.f, 0.f,  0.f, 0.f,
                      0.f, 0.f, -1.f, 0.f,
                      0.f, 1.f,  0.f, 0.f,
                      0.f, 0.f,  0.f, 1.f);
  ivRoot->addChild(mt);

#if 0 // Setting everything to defaults may not be correct option
      // because the user may want to have model whose material,etc.
      // will be specified up in the scene graph and he may, for instance,
      // reuse the model with different materials.
      // However, I am not sure anyway. PCJohn-2007-08-27

  //              OSG defaults to:    Inventor defaults to:
  // GL_LIGHTING   enabled             enabled
  SoLightModel *lm = new SoLightModel;
  lm->model = SoLightModel::PHONG;
  ivRoot->addChild(lm);

  //              OSG defaults to:    Inventor defaults to:
  // lighting:     one side            one side lighting
  // frontFace:    COUNTER_CLOCKWISE   COUNTER_CLOCKWISE
  // culling:      DISABLED            DISABLED
  // faceCulling:  GL_BACK             GL_BACK
  SoShapeHints *sh = new SoShapeHints;
  sh->vertexOrdering = SoShapeHints::UNKNOWN_ORDERING;
  ivRoot->addChild(sh);

  // Make sure texturing is OFF
  ivRoot->addChild(new SoTexture2D);

  // Put default material
  ivRoot->addChild(new SoMaterial);
#endif

  // initialize Inventor state stack
  ivStack.push(InventorState::createTopLevelState(ivRoot));
}


ConvertToInventor::~ConvertToInventor()
{
  assert(ivStack.size() == 1 && "Not all InventorStates were popped from ivStack.");
  if (ivRoot)  ivRoot->unref();
}


SoNode* ConvertToInventor::getIvSceneGraph() const
{
  return ivRoot;
}


void ConvertToInventor::apply(osg::Node &node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: node traversed" << std::endl;
#endif

  traverse(node);
}


template<typename fieldClass, typename ivType, typename osgType>
void osgArray2ivMField_template(const osg::Array *array, fieldClass &field, int startIndex = 0, int stopIndex = 0, int numItemsUntilMinusOne = 0)
{
  int i,num = array->getNumElements();
  if (startIndex!=0 || stopIndex!=0) {
    num = stopIndex - startIndex;
    assert(stopIndex >= startIndex);
    assert(startIndex >= 0 && stopIndex >= 0);
    assert(stopIndex <= int(array->getNumElements()));
  }

  // update num if numItemsUntilMinusOne > 0
  if (numItemsUntilMinusOne > 0 && num >= 1)
    num += (num-1) / numItemsUntilMinusOne;

  field.setNum(num);
  ivType *a = field.startEditing();

  osgType *ptr = (osgType*)array->getDataPointer() + startIndex;

  if (numItemsUntilMinusOne <= 0)
    for (i=0; i<num; i++, ptr++)
      a[i] = ivType(*ptr);
  else {
    int z;
    for (i=0, z=0; i<num; i++)
      if (z == numItemsUntilMinusOne) {
        a[i] = ivType(-1);
        z = 0;
      } else {
        a[i] = ivType(*ptr);
        ptr++;
        z++;
      }
  }

  field.finishEditing();
}


template<typename ivType, typename osgType, int shift>
void osgArray2ivMField_composite_template_worker(ivType *dest, osgType *src, int num, int numItemsUntilMinusOne = 0)
{
  for (int i=0; i<num; i++, src+=shift)
    dest[i] = ivType(src);
}


template<>
void osgArray2ivMField_composite_template_worker<SbColor, GLubyte, 4>(SbColor *dest, GLubyte *src, int num, int numItemsUntilMinusOne)
{
  for (int i=0; i<num; i++, src+=4)
    dest[i].setValue(src[0]/255.f, src[1]/255.f, src[2]/255.f);
}


template<>
void osgArray2ivMField_composite_template_worker<SbVec3f, float, 2>(SbVec3f *dest, float *src, int num, int numItemsUntilMinusOne)
{
  for (int i=0; i<num; i++, src+=2)
    dest[i].setValue(src[0], src[1], 0.f);
}


template<typename fieldClass, typename ivType, typename osgType, int shift>
void osgArray2ivMField_composite_template(const osg::Array *array, fieldClass &field, int startIndex = 0, int stopIndex = 0, int numItemsUntilMinusOne = 0)
{
  int num = array->getNumElements();
  if (startIndex!=0 || stopIndex!=0) {
    num = stopIndex - startIndex;
    assert(stopIndex >= startIndex);
    assert(startIndex >= 0 && stopIndex >= 0);
    assert(stopIndex <= int(array->getNumElements()));
  }
  assert(numItemsUntilMinusOne <= 0 && "Composite template must have numItemsUntilMinusOne set to 0.");
  field.setNum(num);
  ivType *a = field.startEditing();

  osgType *ptr = (osgType*)array->getDataPointer() + startIndex;
  osgArray2ivMField_composite_template_worker<ivType, osgType, shift>(a, ptr, num, 0);

  field.finishEditing();
}


template<typename fieldClass, typename ivType, typename osgType, int numComponents>
void osgArray2ivMField_pack_template(const osg::Array *array, fieldClass &field,
                                            osgType mul, osgType max, osgType min,
                                            int startIndex = 0, int stopIndex = 0, int numItemsUntilMinusOne = 0)
{
  int i,num = array->getNumElements();
  if (startIndex!=0 || stopIndex!=0) {
    num = stopIndex - startIndex;
    assert(stopIndex >= startIndex);
    assert(startIndex >= 0 && stopIndex >= 0);
    assert(stopIndex <= int(array->getNumElements()));
  }
  assert(numItemsUntilMinusOne <= 0 && "Pack template must have numItemsUntilMinusOne set to 0.");
  field.setNum(num);
  ivType *a = field.startEditing();

  osgType *ptr = (osgType*)array->getDataPointer() + startIndex;
  for (i=0; i<num; i++, ptr++) {
    a[i] = ivType(0);
    for (int j=0; j<numComponents; j++) {
      osgType tmp = ptr[j]*mul;
      if (tmp > max)  tmp = max;
      if (tmp < min)  tmp = min;
      a[i] |= ivType(tmp) << (((numComponents-1)*8)-(j*8));
    }
  }

  field.finishEditing();
}


template<typename fieldClass, typename fieldItemType>
bool ivApplicateIntType(const osg::Array *array, fieldClass &field, int startIndex, int stopIndex, int numItemsUntilMinusOne)
{
  if (field.isOfType(fieldClass::getClassTypeId()))
  {
    switch (array->getType())
    {
      case osg::Array::ByteArrayType:   osgArray2ivMField_template<fieldClass, fieldItemType, int8_t>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::UByteArrayType:  osgArray2ivMField_template<fieldClass, fieldItemType, uint8_t>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::ShortArrayType:  osgArray2ivMField_template<fieldClass, fieldItemType, int16_t>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::UShortArrayType: osgArray2ivMField_template<fieldClass, fieldItemType, uint16_t>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::IntArrayType:    osgArray2ivMField_template<fieldClass, fieldItemType, int32_t>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::UIntArrayType:   osgArray2ivMField_template<fieldClass, fieldItemType, uint32_t>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::FloatArrayType:  osgArray2ivMField_template<fieldClass, fieldItemType, float>
                                          (array, field, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::Vec4bArrayType:  // FIXME: should signed values be handled differently? Like -128..127?
      case osg::Array::Vec4ubArrayType: osgArray2ivMField_pack_template<fieldClass, fieldItemType, GLubyte, 4>
                                          (array, field, 1, 255, 0, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      case osg::Array::Vec4ArrayType:   osgArray2ivMField_pack_template<fieldClass, fieldItemType, float, 4>
                                          (array, field, 255.f, 255.f, 0.f, startIndex, stopIndex, numItemsUntilMinusOne); return true;
      default: break;
    }
  }
  return false;
}


static void osgArray2ivMField(const osg::Array *array, SoMField &field, int startIndex = 0, int stopIndex = 0, int numItemsUntilMinusOne = 0)
{
  if (field.isOfType(SoMFFloat::getClassTypeId()))
  {
    switch (array->getType())
    {
      case osg::Array::FloatArrayType:  osgArray2ivMField_template<SoMFFloat, float, float>
                                          (array, (SoMFFloat&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      default: break;
    }
  }
  else if (ivApplicateIntType<SoMFInt32,  int32_t>(array, (SoMFInt32&) field, startIndex, stopIndex, numItemsUntilMinusOne)) return;
  else if (ivApplicateIntType<SoMFUInt32,uint32_t>(array, (SoMFUInt32&)field, startIndex, stopIndex, numItemsUntilMinusOne)) return;
  else if (ivApplicateIntType<SoMFShort,  int16_t>(array, (SoMFShort&) field, startIndex, stopIndex, numItemsUntilMinusOne)) return;
  else if (ivApplicateIntType<SoMFUShort,uint16_t>(array, (SoMFUShort&)field, startIndex, stopIndex, numItemsUntilMinusOne)) return;
  else if (field.isOfType(SoMFVec2f::getClassTypeId()))
  {
    switch (array->getType())
    {
      case osg::Array::Vec2ArrayType:   osgArray2ivMField_composite_template<SoMFVec2f, SbVec2f, float, 2>
                                          (array, (SoMFVec2f&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      default: break;
    }
  }
  else if (field.isOfType(SoMFVec3f::getClassTypeId()))
  {
    switch (array->getType())
    {
      case osg::Array::Vec3ArrayType:   osgArray2ivMField_composite_template<SoMFVec3f, SbVec3f, float, 3>
                                          (array, (SoMFVec3f&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      case osg::Array::Vec2ArrayType:   osgArray2ivMField_composite_template<SoMFVec3f, SbVec3f, float, 2>
                                          (array, (SoMFVec3f&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      default: break;
    }
  }
  else if (field.isOfType(SoMFVec4f::getClassTypeId()))
  {
    switch (array->getType()) {
      case osg::Array::Vec4ArrayType:   osgArray2ivMField_composite_template<SoMFVec4f, SbVec4f, float, 4>
                                          (array, (SoMFVec4f&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      default: break;
    }
  }
  else if (field.isOfType(SoMFColor::getClassTypeId()))
  {
    switch (array->getType())
    {
      case osg::Array::Vec3ArrayType:   osgArray2ivMField_composite_template<SoMFColor, SbColor, float, 3>
                                          (array, (SoMFColor&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      case osg::Array::Vec4ArrayType:   osgArray2ivMField_composite_template<SoMFColor, SbColor, float, 4>
                                          (array, (SoMFColor&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      case osg::Array::Vec4ubArrayType: osgArray2ivMField_composite_template<SoMFColor, SbColor, GLubyte, 4>
                                          (array, (SoMFColor&)field, startIndex, stopIndex, numItemsUntilMinusOne); return;
      default: break;
    }
  };

  OSG_WARN << "IvWriter: No direct conversion for array. "
    << "The file may be broken." << std::endl;
}


template<typename variableType, typename indexType>
bool ivDeindex(variableType *dest, const variableType *src, const int srcNum,
                    const indexType *indices, const int numToProcess)
{
  for (int i=0; i<numToProcess; i++) {
    int index = indices[i];
    if (index<0 || index>=srcNum)  return false;
    dest[i] = src[index];
  }
  return true;
}


template<typename variableType>
bool ivDeindex(variableType *dest, const variableType *src, const int srcNum,
                    const osg::Array *indices, const int numToProcess)
{
  if (int(indices->getNumElements()) < numToProcess) {
    assert(0 && "Something is wrong: indices array is shorter than numToProcess.");
    return false;
  }

  switch (indices->getType()) {
  case osg::Array::ByteArrayType:
  case osg::Array::UByteArrayType:
      return ivDeindex<variableType, GLbyte>(dest, src, srcNum,
                                           (GLbyte*)indices->getDataPointer(), numToProcess);
      break;
  case osg::Array::ShortArrayType:
  case osg::Array::UShortArrayType:
      return ivDeindex<variableType, GLshort>(dest, src, srcNum,
                                            (GLshort*)indices->getDataPointer(), numToProcess);
      break;
  case osg::Array::IntArrayType:
  case osg::Array::UIntArrayType:
      return ivDeindex<variableType, GLint>(dest, src, srcNum,
                                          (GLint*)indices->getDataPointer(), numToProcess);
      break;
  default:
      assert(0 && "Index of strange type.");
      return false;
  }
}


template<typename variableType, typename fieldType>
bool ivProcessArray(const osg::Array *drawElemIndices,
                         fieldType *destField, const fieldType *srcField,
                         int startIndex, int numToProcess)
{
  bool ok = true;

  if (drawElemIndices) {

    // "deindex" original data
    ok = ivDeindex<variableType>(destField->startEditing(),
                                srcField->getValues(startIndex),
                                srcField->getNum(), drawElemIndices, numToProcess);

    destField->finishEditing();
    if (!ok)
      OSG_WARN << "IvWriter: Can not deindex - bug in model: index out of range." << std::endl;

  } else {

    // copy required part of original data
    const variableType *src = srcField->getValues(startIndex);
    assert(startIndex+numToProcess <= srcField->getNum() && "Index out of bounds.");
    variableType *dest = destField->startEditing();
    for (int i=0; i<numToProcess; i++)
      *(dest++) = *(src++);
    destField->finishEditing();
  }

  return ok;
}


static void processIndices(const osg::Array *drawElemIndices,
                           SoMFInt32 &ivIndices,
                           int startIndex, int stopIndex, int numItemsUntilMinusOne)
{
  if (drawElemIndices) {

      osgArray2ivMField(drawElemIndices, ivIndices, startIndex, stopIndex, numItemsUntilMinusOne);

  } else {

    int num = stopIndex-startIndex;
    if (numItemsUntilMinusOne != 0 && num >= 1)
      num += (num-1)/numItemsUntilMinusOne;
    ivIndices.setNum(num);
    int32_t *a = ivIndices.startEditing();
    if (numItemsUntilMinusOne <= 0)
      for (int i=0,j=startIndex; j<stopIndex; i++,j++)
        a[i] = j;
    else
      for (int i=0,j=startIndex,k=0; j<stopIndex; i++)
        if (k==numItemsUntilMinusOne) {
          a[i]=-1;
          k=0;
        } else {
          a[i] = j;
          j++;
          k++;
        }
    ivIndices.finishEditing();
  }
}


static void postProcessDrawArrayLengths(const osg::DrawArrayLengths *drawArrayLengths, SoMFInt32 *field)
{
  int origNum = field->getNum();
  int newNum = origNum + drawArrayLengths->size()-1;
  field->setNum(newNum);
  int32_t *a = field->startEditing();
  int32_t *src = a + origNum;
  int32_t *dst = a + newNum;
  for (osg::DrawArrayLengths::const_reverse_iterator primItr = drawArrayLengths->rbegin();
      primItr!=drawArrayLengths->rend()-1;
      ++primItr) {
    int c = *primItr;
    src -= c;
    dst -= c;
    memmove(dst, src, sizeof(int32_t)*c);
    dst--;
    *dst = -1;
  }
  field->finishEditing();
}


static void postProcessField(const SbIntList &runLengths, osg::PrimitiveSet::Mode primType,
                             SoMFInt32 *field, deprecated_osg::Geometry::AttributeBinding binding)
{
  if (binding==deprecated_osg::Geometry::BIND_OFF || binding==deprecated_osg::Geometry::BIND_OVERALL ||
      binding==deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET)
    return;

  // make copy of array
  const int32_t *fieldArray = field->getValues(0);
  int origNum = field->getNum();
  int32_t *tmpArray = new int32_t[origNum];
  memcpy(tmpArray, fieldArray, origNum*sizeof(int32_t));

  // compute new number of indices
  int newNum = origNum;
  const int l = runLengths.getLength();
  switch (binding) {
    case deprecated_osg::Geometry::BIND_PER_VERTEX:
      for (int i=0; i<l; i++)
        newNum += (runLengths[i]-3)*3;
      break;
    case deprecated_osg::Geometry::BIND_PER_PRIMITIVE:
      for (int i=0; i<l; i++)
        newNum += runLengths[i]-3;
      break;
    default:
      assert(0);
  }

  // process indices
  field->setNum(newNum);
  int32_t *src = tmpArray;
  int32_t *dst = field->startEditing();
  // int32_t *dst2 = dst;
  switch (binding) {
    case deprecated_osg::Geometry::BIND_PER_VERTEX:
      for (int i=0; i<l; i++) {
        int c = runLengths[i];
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        bool even = true;
        int32_t first = *(src-3);
        for (int j=3; j<c; j++) {
          *(dst++) = -1;
          if (primType==osg::PrimitiveSet::TRIANGLE_STRIP) {
            if (even) {
              *(dst++) = *(src-1);
              *(dst++) = *(src-2);
            } else {
              *(dst++) = *(src-2);
              *(dst++) = *(src-1);
            }
            even = !even;
          } else
          if (primType==osg::PrimitiveSet::TRIANGLE_FAN) {
            *(dst++) = first;
            *(dst++) = *(src-1);
          } // FIXME: are QUADS, QUAD_STRIP, and POLYGON requiring some handling here as well? PCJohn-2007-08-25
          else {
            *(dst++) = *(src-2);
            *(dst++) = *(src-1);
          }
          *(dst++) = *(src++);
        }
        src++; // skip -1
        if (i != l-1)
          *(dst++) = -1;
      }
      break;

    case deprecated_osg::Geometry::BIND_PER_PRIMITIVE:
      for (int i=0; i<l; i++,src++) {
        int c = runLengths[i];
        *(dst++) = *(src);
        for (int j=3; j<c; j++)
          *(dst++) = *(src);
      }
      break;

    default:
      assert(0);
  }
  field->finishEditing();

  // free resources
  delete [] tmpArray;
}


static void postProcessTriangleSeparation(SoIndexedShape *shape, osg::PrimitiveSet::Mode primType,
                                          deprecated_osg::Geometry::AttributeBinding normalBinding,
                                          deprecated_osg::Geometry::AttributeBinding colorBinding)
{
  // compute runLengths
  SbIntList runLengths;
  const int32_t *a = shape->coordIndex.getValues(0);
  int origNum = shape->coordIndex.getNum();
  int l = 0;
  for (int i=0; i<origNum; i++,a++) {
    if (*a == -1) {
      runLengths.append(l);
      l = 0;
    } else
      l++;
  }
  if (l != 0) // append final l if field is not finished by -1
    runLengths.append(l);

  postProcessField(runLengths, primType, &shape->coordIndex,        deprecated_osg::Geometry::BIND_PER_VERTEX);
  postProcessField(runLengths, primType, &shape->normalIndex,       normalBinding);
  postProcessField(runLengths, primType, &shape->materialIndex,     colorBinding);
  bool notUseTexCoords = shape->textureCoordIndex.getNum()==0 ||
                         (shape->textureCoordIndex.getNum()==1 && shape->textureCoordIndex[0] == -1);
  if (!notUseTexCoords)
    postProcessField(runLengths, primType, &shape->textureCoordIndex, deprecated_osg::Geometry::BIND_PER_VERTEX);

}


static SoMaterialBinding* createMaterialBinding(const deprecated_osg::Geometry *g, bool isMaterialIndexed)
{
  SoMaterialBinding *materialBinding = new SoMaterialBinding;
  switch (g->getColorBinding()) {
  case deprecated_osg::Geometry::BIND_OFF: // OFF means use material from state set (if any) for whole geometry
  case deprecated_osg::Geometry::BIND_OVERALL:
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET: materialBinding->value = SoMaterialBinding::OVERALL; break;
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE:     materialBinding->value = (isMaterialIndexed) ? SoMaterialBinding::PER_PART_INDEXED   : SoMaterialBinding::PER_PART; break;
  case deprecated_osg::Geometry::BIND_PER_VERTEX:        materialBinding->value = (isMaterialIndexed) ? SoMaterialBinding::PER_VERTEX_INDEXED : SoMaterialBinding::PER_VERTEX; break;
  default: assert(0);
  }
  return materialBinding;
}


static SoNormalBinding* createNormalBinding(const deprecated_osg::Geometry *g, bool areNormalsIndexed)
{
  // Convert normal binding
  SoNormalBinding *normalBinding = new SoNormalBinding;
  switch (g->getNormalBinding()) {
  case deprecated_osg::Geometry::BIND_OFF: // FIXME: what to do with BIND_OFF value?
  case deprecated_osg::Geometry::BIND_OVERALL:
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET: normalBinding->value = SoNormalBinding::OVERALL; break;
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE:     normalBinding->value = (areNormalsIndexed) ? SoNormalBinding::PER_PART_INDEXED   : SoNormalBinding::PER_PART; break;
  case deprecated_osg::Geometry::BIND_PER_VERTEX:        normalBinding->value = (areNormalsIndexed) ? SoNormalBinding::PER_VERTEX_INDEXED : SoNormalBinding::PER_VERTEX; break;
  default: assert(0);
  }
  return normalBinding;
}


static SoTextureCoordinateBinding* createTexCoordBinding(SbBool useIndexing)
{
  SoTextureCoordinateBinding *b = new SoTextureCoordinateBinding;
  b->value.setValue(useIndexing ? SoTextureCoordinateBinding::PER_VERTEX_INDEXED :
                    SoTextureCoordinateBinding::PER_VERTEX);
  return b;
}


static SoTexture2::Model convertTexEnvMode(osg::TexEnv::Mode osgMode, bool useIvExtensions)
{
  switch (osgMode) {
  case GL_MODULATE: return SoTexture2::MODULATE;
  case GL_REPLACE:  return (SoTexture2::Model)(useIvExtensions ? GL_REPLACE : GL_MODULATE);
  case GL_BLEND:    return SoTexture2::BLEND;
  case GL_DECAL:    return SoTexture2::DECAL;
  default: assert(0); return SoTexture2::MODULATE;
  }
}



static SoTexture2::Wrap convertTextureWrap(osg::Texture::WrapMode osgWrap)
{
  // notes on support of CLAMP_TO_BORDER, CLAMP_TO_EDGE, and MIRRORED_REPEAT:
  // original SGI Inventor: no
  // Coin: no (until current version Coin 2.5.0b3)
  // TGS Inventor: introduced in TGS Inventor 5.0 (available in SoTexture class)

  // note: Coin (since 2.0) uses CLAMP_TO_EDGE for rendering if SoTexture2::CLAMP is specified.

  switch (osgWrap) {
    case osg::Texture::CLAMP:
    case osg::Texture::CLAMP_TO_BORDER:
    case osg::Texture::CLAMP_TO_EDGE: return SoTexture2::CLAMP;
    case osg::Texture::REPEAT:
    case osg::Texture::MIRROR: return SoTexture2::REPEAT;
    default: assert(0); return SoTexture2::REPEAT;
  }
}


static void setSoTransform(SoTransform *tr, const osg::Vec3 &translation, const osg::Quat &rotation,
                           const osg::Vec3 &scale = osg::Vec3(1.,1.,1.))
{
  tr->translation.setValue(translation.ptr());
  tr->rotation.setValue(rotation.x(), rotation.y(), rotation.z(), rotation.w());
  tr->scaleFactor.setValue(scale.ptr());
  //tr->scaleCenter.setValue(osg::Vec3f(node.getPivotPoint())); <- testing required on this line
}


static bool updateMode(bool &flag, const osg::StateAttribute::GLModeValue value)
{
  if (value & osg::StateAttribute::INHERIT)  return flag;
  else  return (flag = (value & osg::StateAttribute::ON));
}


ConvertToInventor::InventorState* ConvertToInventor::createInventorState(const osg::StateSet *ss)
{
  // Push on stack
  const InventorState *ivPrevState = &ivStack.top();
  ivStack.push(*ivPrevState);
  InventorState *ivState = &ivStack.top();

  // Inventor graph
  ivState->ivHead = new SoSeparator;
  ivPrevState->ivHead->addChild(ivState->ivHead);

  if (ss) {

    //
    // Lighting
    //

    // enable/disable lighting
    updateMode(ivState->osgLighting, ss->getMode(GL_LIGHTING));
    if (ivState->osgLighting != ivPrevState->osgLighting) {
      SoLightModel *lm = new SoLightModel;
      lm->model = (ivState->osgLighting) ? SoLightModel::PHONG : SoLightModel::BASE_COLOR;
      ivState->ivHead->addChild(lm);
    }

    // two-sided lighting
    const osg::LightModel *osgLM = dynamic_cast<const osg::LightModel*>(ss->getAttribute(osg::StateAttribute::LIGHTMODEL));
    if (osgLM)
      ivState->osgTwoSided = osgLM->getTwoSided();

    // front face
    const osg::FrontFace *osgFF = dynamic_cast<const osg::FrontFace*>(ss->getAttribute(osg::StateAttribute::FRONTFACE));
    if (osgFF)
      ivState->osgFrontFace = osgFF->getMode();

    // face culling
    updateMode(ivState->osgCullFaceEnabled, ss->getMode(GL_CULL_FACE));
    const osg::CullFace *osgCF = dynamic_cast<const osg::CullFace*>(ss->getAttribute(osg::StateAttribute::CULLFACE));
    if (osgCF)
      ivState->osgCullFace = osgCF->getMode();

    // detect state change
    if (ivState->osgTwoSided != ivPrevState->osgTwoSided ||
        ivState->osgFrontFace != ivPrevState->osgFrontFace ||
        ivState->osgCullFaceEnabled != ivPrevState->osgCullFaceEnabled ||
        ivState->osgCullFace != ivPrevState->osgCullFace) {

      // new SoShapeHints
      SoShapeHints *sh = new SoShapeHints;
      if (ivState->osgTwoSided) {
        // warn if face culling is on
        if (ivState->osgCullFaceEnabled)
          OSG_WARN << "IvWriter: Using face culling and two-sided lighting together! "
                                    "Ignoring face culling." << std::endl;

        // set two-sided lighting and backface culling off
        sh->vertexOrdering = ivState->osgFrontFace==osg::FrontFace::COUNTER_CLOCKWISE ?
                             SoShapeHints::COUNTERCLOCKWISE : SoShapeHints::CLOCKWISE;
        sh->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
      }
      else {
        // set one-sided lighting and backface optionally
        if (ivState->osgCullFaceEnabled) {

          // determine vertex ordering
          bool ccw = ivState->osgFrontFace==osg::FrontFace::COUNTER_CLOCKWISE;
          if (ivState->osgCullFace != osg::CullFace::BACK)  ccw = !ccw;

          if (ccw)
            // Warn if culling the lit faces while rendering unlit faces.
            // Inventor does not support this setup and it lits the unculled faces only.
            OSG_WARN << "IvWriter: Culling was set in a way that one-sided lighting will lit the culled sides of faces. "
                                      "Using lighting on correct faces." << std::endl;

          // face culling on
          sh->vertexOrdering = ccw ? SoShapeHints::COUNTERCLOCKWISE : SoShapeHints::CLOCKWISE;
          sh->shapeType = SoShapeHints::SOLID;
        }
        else
          // no face culling
          sh->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
      }
      ivState->ivHead->addChild(sh);
    }

    //
    // Texturing
    //
    // FIXME: handle 1D and 3D textures

    // get OSG state
    ivState->osgTexture = dynamic_cast<const osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
    ivState->osgTexEnv = dynamic_cast<const osg::TexEnv*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXENV));
    updateMode(ivState->osgTexture2Enabled, ss->getTextureMode(0, GL_TEXTURE_2D));

    // detect state changes
    if (ivState->osgTexture2Enabled != ivPrevState->osgTexture2Enabled ||
        ivState->osgTexture != ivPrevState->osgTexture ||
        ivState->osgTexEnv != ivPrevState->osgTexEnv) {

      if (!ivState->osgTexture2Enabled ||
          ivState->osgTexture==NULL || ivState->osgTexture->getImage(0)==NULL)

        // empty texture disables texturing
        ivState->ivTexture = new SoTexture2;

      else {

        // reuse texture if possible
        ivState->ivTexture = ivTexturesMap[ivState->osgTexture][ivState->osgTexEnv];

        // if nothing for reusing, create new
        if (!ivState->ivTexture) {

          // create texture
          ivState->ivTexture = new SoTexture2;
          ivTexturesMap[ivState->osgTexture][ivState->osgTexEnv] = ivState->ivTexture;

          // texture file name
          const std::string &textureName = ivState->osgTexture->getImage(0)->getFileName();
          ivState->ivTexture->filename.setValue(textureName.c_str()); // FIXME: handle inlined texture data in the files

          // wrap
          ivState->ivTexture->wrapS.setValue(convertTextureWrap(
                                             ivState->osgTexture->getWrap(osg::Texture::WRAP_S)));
          ivState->ivTexture->wrapT.setValue(convertTextureWrap(
                                             ivState->osgTexture->getWrap(osg::Texture::WRAP_T)));

          // texture environment
          if (ivState->osgTexEnv) {
            ivState->ivTexture->model.setValue(convertTexEnvMode(
                                               ivState->osgTexEnv->getMode(), useIvExtensions));
            osg::Vec4 color = ivState->osgTexEnv->getColor();
            ivState->ivTexture->blendColor.setValue(color.r(), color.g(), color.b());
          }

          // notes on support of borderColor and borderWidth:
          // original SGI Inventor: no
          // Coin: no (until current version 2.5.0b3)
          // TGS Inventor: introduced in version 5.0 (as SoTexture::borderColor)

          // FIXME: implement support for texture filtering
        }
      }
    }

    // Texture coordinate generation
    updateMode(ivState->osgTexGenS, ss->getTextureMode(0, GL_TEXTURE_GEN_S));
    updateMode(ivState->osgTexGenT, ss->getTextureMode(0, GL_TEXTURE_GEN_T));
    ivState->osgTexGen = dynamic_cast<const osg::TexGen*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXGEN));

    // Material parameters
    const osg::Material *osgMaterial = dynamic_cast<const osg::Material*>(ss->getAttribute(osg::StateAttribute::MATERIAL));
    if (osgMaterial)
      ivState->osgMaterial = osgMaterial;

    if (ivState->osgMaterial != ivPrevState->osgMaterial) {

      ivState->ivMaterial = new SoMaterial;
      assert(ivState->osgMaterial);

      // Warn if using two side materials
      // FIXME: The geometry can be probably doubled, or some estimation can be made
      // whether only front or back faces are used.
      if (ivState->osgMaterial->getAmbientFrontAndBack() == false ||
          ivState->osgMaterial->getDiffuseFrontAndBack() == false ||
          ivState->osgMaterial->getSpecularFrontAndBack() == false ||
          ivState->osgMaterial->getEmissionFrontAndBack() == false ||
          ivState->osgMaterial->getShininessFrontAndBack() == false)
      {
        OSG_WARN << "IvWriter: Model contains different materials for front and "
                                  "back faces. This is not handled properly. Using front material only." << std::endl;
      }

      // Convert colors
      // OSG represents colors by: Vec3, Vec4,Vec4ub
      // Inventor by: uint32 (RGBA, by SoPackedColor), SbColor (Vec3f, by SoMaterial and SoBaseColor)
      // note: Inventor can use DIFFUSE component inside a shape only. Material is set for whole shape.
      // Although SoMaterial is used only, SoPackedColor may bring some possibilities on per-vertex
      // alpha and SoBaseColor may be useful on pre-lit scene.
      if (ivState->osgMaterial->getColorMode() != osg::Material::DIFFUSE &&
          ivState->osgMaterial->getColorMode() != osg::Material::OFF)
      {

        if (ivState->osgMaterial->getColorMode() == osg::Material::AMBIENT_AND_DIFFUSE)
        {
            OSG_WARN << "IvWriter: The model is using AMBIENT_AND_DIFFUSE material "
                                    "mode while Inventor supports DIFFUSE mode only. "
                                    "The model colors may not much exactly." << std::endl;
        }
        else
        {
            OSG_WARN << "IvWriter: The model is not using DIFFUSE material mode and "
                                    "Inventor supports DIFFUSE mode only. "
                                    "The model colors may not be correct." << std::endl;
        }
      }

      // Convert material components
      // FIXME: Transparency can be specified for each component in OSG
      // and just globally in Inventor.
      // Solutions? It can be averaged or just diffuse can be used.
      ((SoMaterial*)ivState->ivMaterial)->ambientColor.setValue(osgMaterial->getAmbient(
        osgMaterial->getAmbientFrontAndBack()   ? osg::Material::FRONT_AND_BACK : osg::Material::FRONT).ptr());
      ((SoMaterial*)ivState->ivMaterial)->diffuseColor.setValue(osgMaterial->getDiffuse(
        osgMaterial->getDiffuseFrontAndBack()   ? osg::Material::FRONT_AND_BACK : osg::Material::FRONT).ptr());
      ((SoMaterial*)ivState->ivMaterial)->specularColor.setValue(osgMaterial->getSpecular(
        osgMaterial->getSpecularFrontAndBack()  ? osg::Material::FRONT_AND_BACK : osg::Material::FRONT).ptr());
      ((SoMaterial*)ivState->ivMaterial)->emissiveColor.setValue(osgMaterial->getEmission(
        osgMaterial->getEmissionFrontAndBack()  ? osg::Material::FRONT_AND_BACK : osg::Material::FRONT).ptr());
      ((SoMaterial*)ivState->ivMaterial)->shininess.setValue(osgMaterial->getShininess(
        osgMaterial->getShininessFrontAndBack() ? osg::Material::FRONT_AND_BACK : osg::Material::FRONT));
      ((SoMaterial*)ivState->ivMaterial)->transparency.setValue(1.f - osgMaterial->getDiffuse(
        osgMaterial->getDiffuseFrontAndBack()   ? osg::Material::FRONT_AND_BACK : osg::Material::FRONT).a());
    }

    // note on "headlight":
    // OSG is using HEADLIGHT or SKY_LIGHT. In both cases it sets these defaults:
    // osg::LightModel::ambientIntensity(0.1, 0.1, 0.1, 1.0);
    // osg::Light::num(0)
    // osg::Light::ambient(0,0,0,1)
    // osg::Light::diffuse(0.8,0.8,0.8, 1)
    // osg::Light::specular(1,1,1,1)
    //
    // Inventor uses different settings:
    // SoEnvironment::ambientIntensity(0.2)
    // SoEnvironment::ambientColor(1,1,1)
    // SoDirectionalLight::intensity(1)
    // SoDirectionalLight::color(1,1,1)
    //
    // note on Inventor light handling:
    // ambient is set to 0,0,0,1
    // diffuse to color*intensity
    // specular to color*intensity


  #ifdef __COIN__
    //
    // Handle transparency
    //
    // OSG supports GL_BLEND and GL_ALPHA_TEST
    // Related classes: BlendFunc, BlendEquation, BlendColor, AlphaFunc
    //
    // Inventor is more difficult and not so robust. According to Inventor 2.1 standard,
    // just SoMaterial::transparency, SoTexture2 with alpha channel carry transparency information
    // that is controlled by SoGLRenderAction::transparency type that is set to SCREEN_DOOR by default
    // (dither pattern). So, if the user does not select better transparency type, there is no
    // possiblity to control transparency type from file format.
    //
    // However, SoTransparencyType node was introduced to overcome this historical limitation
    // because transparency was performance expensive long time ago.
    // Support by different Inventor branches:
    //
    // SGI Inventor: no
    // Coin: since 2.0
    // TGS Inventor: since 5.0
    //
    // Alpha test was introduced in TGS 4.0, but as SoGLRenderArea::setAlphaTest. So, no direct
    // control in the file format.
    //
    // Conclusion:
    // - Alpha test is not supported and all pixels will be drawn
    // - Blending - current strategy is following:
    //     ADD x BLEND - ADD is used if destination-blending-factor is GL_ONE
    //     DELAYED rendering is used if transparency bin is used by OSG
    //     SORTED_OBJECT is used if ...
    //

    updateMode(ivState->osgBlendEnabled, ss->getMode(GL_BLEND));
    ivState->osgBlendFunc = dynamic_cast<const osg::BlendFunc*>(ss->getAttribute(osg::StateAttribute::BLENDFUNC));

    if (useIvExtensions)
      if (ivState->osgBlendEnabled != ivPrevState->osgBlendEnabled ||
          ivState->osgBlendFunc != ivPrevState->osgBlendFunc ||
          (ivState->osgBlendFunc && ivPrevState->osgBlendFunc &&
          ivState->osgBlendFunc->getDestinationRGB() != ivPrevState->osgBlendFunc->getDestinationRGB())) {

        const SoTransparencyType::Type transparencyTable[8] = {
          SoTransparencyType::BLEND,         SoTransparencyType::ADD,
          SoTransparencyType::DELAYED_BLEND, SoTransparencyType::DELAYED_ADD,
          // FIXME: add sorted modes and test previous four
        };

        int index = 0;
        if (!ivState->osgBlendFunc)  index |= 0;
        else  index = (ivState->osgBlendFunc->getDestinationRGB() == osg::BlendFunc::ONE) ? 1 : 0;
        index |= (ss->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN) ? 2 : 0;

        SoTransparencyType *ivTransparencyType = new SoTransparencyType;
        ivTransparencyType->value = transparencyTable[index];
        ivState->ivHead->addChild(ivTransparencyType);
      }
  #endif

  }

  // ref Inventor nodes that are required when processing Drawables
  if (ivState->ivTexture)
    ivState->ivTexture->ref();
  if (ivState->ivMaterial)
    ivState->ivMaterial->ref();

  return ivState;
}


void ConvertToInventor::popInventorState()
{
  InventorState *ivState = &ivStack.top();

  // unref Inventor nodes
  if (ivState->ivTexture)
    ivState->ivTexture->unref();
  if (ivState->ivMaterial)
    ivState->ivMaterial->unref();

  ivStack.pop();
}


static bool processPrimitiveSet(const deprecated_osg::Geometry *g, const osg::PrimitiveSet *pset,
                                osg::UIntArray *drawElemIndices, bool needSeparateTriangles,
                                int elementsCount, int primSize, const int startIndex, int stopIndex,
                                int &normalIndex, int &colorIndex,
                                SoNode *ivCoords, SoNormal *ivNormals, SoNode *ivMaterial,
                                SoNode *ivTexCoords, SoNode *ivTexture, SoShape *shape,
                                SoSeparator *&indexedRoot, SoSeparator *&nonIndexedRoot)
{
  bool ok = true;
  const osg::DrawArrayLengths *drawArrayLengths =
    (elementsCount == -1) ? dynamic_cast<const osg::DrawArrayLengths*>(pset) : NULL;

  int drawArrayLengthsElems = 0;

  if (drawArrayLengths) {

    // compute elementsCount
    elementsCount = 0;
    drawArrayLengthsElems = 0;
    for (osg::DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
        primItr!=drawArrayLengths->end();
        ++primItr, drawArrayLengthsElems++)
      elementsCount += *primItr;

    // update stopIndex
    stopIndex = startIndex + elementsCount;
  }

  // NonIndexed data for nonIndexed shapes
  SoNode *nonIndexedCoords = NULL;
  SoNode *nonIndexedTexCoords = NULL;
  SoNormal *nonIndexedNormals = NULL;
  SoNode *nonIndexedMaterial = NULL;

  // Normal indexing
  int normalStart = (g->getNormalBinding() == deprecated_osg::Geometry::BIND_PER_VERTEX) ? startIndex : normalIndex;
  int numNormalsUsed = 0;
  switch (g->getNormalBinding()) {
  case deprecated_osg::Geometry::BIND_OFF: // FIXME: what is meaning of OFF value?
  case deprecated_osg::Geometry::BIND_OVERALL:           numNormalsUsed = 0; break;
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET: numNormalsUsed = 1; break;
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE:     numNormalsUsed = primSize!=0 ? (stopIndex-startIndex)/primSize :
                                                (drawArrayLengths ? drawArrayLengths->size() : 1); break;
  case deprecated_osg::Geometry::BIND_PER_VERTEX:        numNormalsUsed = stopIndex-startIndex; break;
  }
  normalIndex += numNormalsUsed;

  // Color indexing
  int colorStart = g->getColorBinding() == deprecated_osg::Geometry::BIND_PER_VERTEX ? startIndex : colorIndex;
  int numColorsUsed = 0;
  switch (g->getColorBinding()) {
  case deprecated_osg::Geometry::BIND_OFF:
  case deprecated_osg::Geometry::BIND_OVERALL:           numColorsUsed = 0; break;
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET: numColorsUsed = 1; break;
  case deprecated_osg::Geometry::BIND_PER_PRIMITIVE:     numColorsUsed = primSize!=0 ? (stopIndex-startIndex)/primSize :
                                                (drawArrayLengths ? drawArrayLengths->size() : 1); break;
  case deprecated_osg::Geometry::BIND_PER_VERTEX:        numColorsUsed = stopIndex-startIndex; break;
  }
  colorIndex += numColorsUsed;

  if (shape->isOfType(SoIndexedShape::getClassTypeId())) {

    // Convert to SoIndexedShape
    processIndices(drawElemIndices, ((SoIndexedShape*)shape)->coordIndex,
                   startIndex, stopIndex, primSize);

    if (ivNormals)
      processIndices(drawElemIndices, ((SoIndexedShape*)shape)->normalIndex,
                     normalStart, normalStart+(numNormalsUsed==0 ? 1 : numNormalsUsed),
                     g->getNormalBinding()==deprecated_osg::Geometry::BIND_PER_VERTEX ? primSize : 0);

    if (ivMaterial)
      processIndices(drawElemIndices, ((SoIndexedShape*)shape)->materialIndex,
                     colorStart, colorStart+(numColorsUsed==0 ? 1 : numColorsUsed),
                     g->getColorBinding()==deprecated_osg::Geometry::BIND_PER_VERTEX ? primSize : 0);

    if (ivTexCoords && !ivTexCoords->isOfType(SoTextureCoordinateFunction::getClassTypeId()))
      processIndices(drawElemIndices, ((SoIndexedShape*)shape)->textureCoordIndex,
                     startIndex, stopIndex, primSize);

    // Post-processing for DrawArrayLengths
    if (drawArrayLengths && primSize==0 && drawArrayLengths->size()>=2) {

      postProcessDrawArrayLengths(drawArrayLengths, &((SoIndexedShape*)shape)->coordIndex);

      if (ivNormals && g->getNormalBinding()==deprecated_osg::Geometry::BIND_PER_VERTEX)
        postProcessDrawArrayLengths(drawArrayLengths, &((SoIndexedShape*)shape)->normalIndex);

      if (ivMaterial && g->getColorBinding()==deprecated_osg::Geometry::BIND_PER_VERTEX)
        postProcessDrawArrayLengths(drawArrayLengths, &((SoIndexedShape*)shape)->materialIndex);

      if (ivTexCoords && !ivTexCoords->isOfType(SoTextureCoordinateFunction::getClassTypeId()))
        postProcessDrawArrayLengths(drawArrayLengths, &((SoIndexedShape*)shape)->textureCoordIndex);
    }

    if (needSeparateTriangles)
      postProcessTriangleSeparation((SoIndexedShape*)shape, (osg::PrimitiveSet::Mode)pset->getMode(),
                                    g->getNormalBinding(), g->getColorBinding());

  } else {

    // Convert to SoNonIndexedShape

    assert(shape->isOfType(SoNonIndexedShape::getClassTypeId()) && "Shape must be non-indexed.");

    int i,n = stopIndex-startIndex;

    // create alternate coordinates
    if (ivCoords->isOfType(SoCoordinate4::getClassTypeId()))
    {
      nonIndexedCoords = new SoCoordinate4;
      if (ok) {
        ((SoCoordinate4*)nonIndexedCoords)->point.setNum(n);
        ok = ivProcessArray<SbVec4f,SoMFVec4f>(drawElemIndices,
                                             &((SoCoordinate4*)nonIndexedCoords)->point,
                                             &((SoCoordinate4*)ivCoords)->point,
                                             startIndex, n);
      }
    } else {
      nonIndexedCoords = new SoCoordinate3;
      if (ok) {
        ((SoCoordinate3*)nonIndexedCoords)->point.setNum(n);
        ok = ivProcessArray<SbVec3f,SoMFVec3f>(drawElemIndices,
                                             &((SoCoordinate3*)nonIndexedCoords)->point,
                                             &((SoCoordinate3*)ivCoords)->point,
                                             startIndex, n);
      }
    }

    // create alternate texture coordinates
    if (ivTexCoords)
    {
      if (ivTexCoords->isOfType(SoTextureCoordinate2::getClassTypeId()))
      {
        nonIndexedTexCoords = new SoTextureCoordinate2;
        if (ok)
        {
          ((SoTextureCoordinate2*)nonIndexedTexCoords)->point.setNum(n);
          ok = ivProcessArray<SbVec2f,SoMFVec2f>(drawElemIndices,
                                               &((SoTextureCoordinate2*)nonIndexedTexCoords)->point,
                                               &((SoTextureCoordinate2*)ivTexCoords)->point,
                                               startIndex, n);
        }
      } else
#ifdef __COIN__
      if (ivTexCoords->isOfType(SoTextureCoordinate3::getClassTypeId()))
      {
        nonIndexedTexCoords = new SoTextureCoordinate3;
        if (ok)
        {
          ((SoTextureCoordinate3*)nonIndexedTexCoords)->point.setNum(n);
          ok = ivProcessArray<SbVec3f,SoMFVec3f>(drawElemIndices,
                                               &((SoTextureCoordinate3*)nonIndexedTexCoords)->point,
                                               &((SoTextureCoordinate3*)ivCoords)->point,
                                               startIndex, n);
        }
      }
      else
#endif  // __COIN__
        nonIndexedTexCoords = ivTexCoords;
    }

    // create alternate normals
    if (ivNormals) {
      nonIndexedNormals = new SoNormal;
      if (ok) {
        nonIndexedNormals->vector.setNum(numNormalsUsed==0 ? 1 : numNormalsUsed);
        ok = ivProcessArray<SbVec3f,SoMFVec3f>(g->getNormalBinding()==deprecated_osg::Geometry::BIND_PER_VERTEX ? drawElemIndices : NULL,
                                             &nonIndexedNormals->vector, &ivNormals->vector,
                                             normalStart, numNormalsUsed==0 ? 1 : numNormalsUsed);
      }
    }

    // create alternate material
    if (ivMaterial) {
      SoMFColor *dstColorField;
      if (ivMaterial->isOfType(SoMaterial::getClassTypeId())) {
        nonIndexedMaterial = new SoMaterial;
        dstColorField = &((SoMaterial*)nonIndexedMaterial)->diffuseColor;
      } else {
        nonIndexedMaterial = new SoBaseColor;
        dstColorField = &((SoBaseColor*)nonIndexedMaterial)->rgb;
      }
      if (ok) {
        // FIXME: diffuse only?
        SoMFColor *srcColorField = (ivMaterial->isOfType(SoMaterial::getClassTypeId())) ?
                                   &((SoMaterial*)ivMaterial)->diffuseColor :
                                   &((SoBaseColor*)ivMaterial)->rgb;
        dstColorField->setNum(numColorsUsed==0 ? 1 : numColorsUsed);
        ok = ivProcessArray<SbColor,SoMFColor>(g->getColorBinding()==deprecated_osg::Geometry::BIND_PER_VERTEX ? drawElemIndices : NULL,
                                            dstColorField, srcColorField,
                                            colorStart, numColorsUsed==0 ? 1 : numColorsUsed);
      }
    }

    if (shape->isOfType(SoPointSet::getClassTypeId()))
      ((SoPointSet*)shape)->numPoints.setValue(elementsCount); else

    if (shape->isOfType(SoLineSet::getClassTypeId())) {
      switch (pset->getMode()) {
      case GL_LINES:
          assert(elementsCount % 2 == 0 && "elementsCount is not multiple of 2.");
          n = elementsCount/2;
          ((SoLineSet*)shape)->numVertices.setNum(n);
          for (i=0; i<n; i++)
            ((SoLineSet*)shape)->numVertices.set1Value(i, 2);
          break;
      case GL_LINE_STRIP:
          if (drawArrayLengths) {
            ((SoLineSet*)shape)->numVertices.setNum(drawArrayLengthsElems);
            int i=0;
            for (osg::DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                primItr!=drawArrayLengths->end();
                ++primItr, i++)
              ((SoLineSet*)shape)->numVertices.set1Value(i, *primItr);
          } else {
            ((SoLineSet*)shape)->numVertices.setNum(1);
            ((SoLineSet*)shape)->numVertices.set1Value(0, elementsCount);
          }
          break;
      default:
          OSG_WARN << "IvWriter: NOT IMPLEMENTED" << std::endl;
          assert(0);
      }
    } else

    if (shape->isOfType(SoTriangleStripSet::getClassTypeId())) {
      switch (pset->getMode()) {
      case GL_TRIANGLES:
          n = elementsCount/3;
          assert(n*3 == elementsCount && "elementsCount is not multiple of 3.");
          ((SoTriangleStripSet*)shape)->numVertices.setNum(n);
          for (i=0; i<n; i++)
            ((SoTriangleStripSet*)shape)->numVertices.set1Value(i, 3);
          break;
      case GL_TRIANGLE_STRIP:
          if (drawArrayLengths) {
            ((SoTriangleStripSet*)shape)->numVertices.setNum(drawArrayLengthsElems);
            int i=0;
            for (osg::DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                primItr!=drawArrayLengths->end();
                ++primItr, i++)
              ((SoTriangleStripSet*)shape)->numVertices.set1Value(i, *primItr);
          } else {
            ((SoTriangleStripSet*)shape)->numVertices.setNum(1);
            ((SoTriangleStripSet*)shape)->numVertices.set1Value(0, elementsCount);
          }
          break;
      case GL_TRIANGLE_FAN:
          OSG_WARN << "IvWriter: GL_TRIANGLE_FAN NOT IMPLEMENTED" << std::endl;
          ((SoTriangleStripSet*)shape)->numVertices.setNum(1);
          ((SoTriangleStripSet*)shape)->numVertices.set1Value(0, elementsCount);
          break;
      case GL_QUAD_STRIP:
          assert(elementsCount % 2 == 0 && "elementsCount is not multiple of 2.");
          if (drawArrayLengths) {
            ((SoTriangleStripSet*)shape)->numVertices.setNum(drawArrayLengthsElems);
            int i=0;
            for (osg::DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                primItr!=drawArrayLengths->end();
                ++primItr, i++)
              ((SoTriangleStripSet*)shape)->numVertices.set1Value(i, *primItr);
          } else {
            ((SoTriangleStripSet*)shape)->numVertices.setNum(1);
            ((SoTriangleStripSet*)shape)->numVertices.set1Value(0, elementsCount);
          }
          break;
      default:
          OSG_WARN << "IvWriter: NOT IMPLEMENTED" << std::endl;
          assert(0);
      }
    } else

    if (shape->isOfType(SoFaceSet::getClassTypeId())) {
      switch (pset->getMode()) {
      case GL_QUADS:
          n = elementsCount/4;
          assert(n*4 == elementsCount && "elementsCount is not multiple of 4.");
          ((SoFaceSet*)shape)->numVertices.setNum(n);
          for (i=0; i<n; i++)
            ((SoFaceSet*)shape)->numVertices.set1Value(i, 4);
          break;
      case GL_POLYGON:
          if (drawArrayLengths) {
            ((SoFaceSet*)shape)->numVertices.setNum(drawArrayLengthsElems);
            int i=0;
            for (osg::DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                primItr!=drawArrayLengths->end();
                ++primItr, i++)
              ((SoFaceSet*)shape)->numVertices.set1Value(i, *primItr);
          } else {
            ((SoFaceSet*)shape)->numVertices.setNum(1);
            ((SoFaceSet*)shape)->numVertices.set1Value(0, elementsCount);
          }
          break;
      default:
          OSG_WARN << "IvWriter: NOT IMPLEMENTED" << std::endl;
          assert(0);
      }
    } else {
      OSG_WARN << "IvWriter: NOT IMPLEMENTED" << std::endl;
      assert(0 && "Unknown non-indexed shape type.");
    }
  }

  // Construct graph

  // Each osg::Drawable will have its own SoSeparator (render caching, culling, etc.)
  SoSeparator *sep = new SoSeparator;
  if (nonIndexedCoords) {
    assert(shape->isOfType(SoNonIndexedShape::getClassTypeId()) && "Not nonIndexed shape.");

    if (!ok) {

      // handle errors
      nonIndexedCoords->ref();
      nonIndexedCoords->unref();
      if (nonIndexedTexCoords) { nonIndexedTexCoords->ref(); nonIndexedTexCoords->unref(); }
      if (nonIndexedNormals) { nonIndexedNormals->ref(); nonIndexedNormals->unref(); }
      nonIndexedMaterial->ref();
      nonIndexedMaterial->unref();
      shape->ref();
      shape->unref();
      sep->ref();
      sep->unref();
    } else {

      // make scene graph
      if (nonIndexedRoot == NULL) {
        nonIndexedRoot = new SoSeparator;
        if (ivTexture)  nonIndexedRoot->addChild(ivTexture);
        if (ivTexture)  nonIndexedRoot->addChild(createTexCoordBinding(FALSE));
        nonIndexedRoot->addChild(createMaterialBinding(g, FALSE));
        if (ivNormals)  nonIndexedRoot->addChild(createNormalBinding(g, FALSE));
      }
      if (nonIndexedMaterial)  sep->addChild(nonIndexedMaterial);
      sep->addChild(nonIndexedCoords);
      if (nonIndexedNormals)  sep->addChild(nonIndexedNormals);
      if (nonIndexedTexCoords)  sep->addChild(nonIndexedTexCoords);
      sep->addChild(shape);
      nonIndexedRoot->addChild(sep);
    }
  } else {
    assert(shape->isOfType(SoIndexedShape::getClassTypeId()) && "Not indexed shape.");
    assert(nonIndexedCoords==NULL && nonIndexedNormals==NULL && nonIndexedMaterial==NULL);
    if (indexedRoot == NULL) {
      indexedRoot = new SoSeparator;
      if (ivTexture)  indexedRoot->addChild(ivTexture);
      if (ivTexture)  indexedRoot->addChild(createTexCoordBinding(TRUE));
      if (ivMaterial)  indexedRoot->addChild(ivMaterial);
      indexedRoot->addChild(createMaterialBinding(g, TRUE));
      indexedRoot->addChild(ivCoords);
      if (ivNormals)  indexedRoot->addChild(ivNormals);
      if (ivNormals)  indexedRoot->addChild(createNormalBinding(g, TRUE));
      if (ivTexCoords)  indexedRoot->addChild(ivTexCoords);
    }
    sep->addChild(shape);
    indexedRoot->addChild(sep);
  }

  return ok;
}


void ConvertToInventor::processGeometry(const deprecated_osg::Geometry *g, InventorState *ivState)
{
  int normalIndex = 0;
  int colorIndex = 0;

  // Inventor scene graph roots
  SoSeparator *indexedRoot = NULL;
  SoSeparator *nonIndexedRoot = NULL;

  // Active material
  SoMaterial *ivStateMaterial = ivState->ivMaterial;
  SoNode *ivMaterial = NULL;
  if (ivState->osgLighting || vrml1Conversion)
    // SoMaterial
    if (g->getColorArray())
      if (ivStateMaterial)  ivMaterial = ivStateMaterial->copy();
      else  ivMaterial = new SoMaterial; // FIXME: check default values of SoMaterial and OSG lighting
    else
      if (ivStateMaterial)  ivMaterial = ivStateMaterial;
      else  ivMaterial = NULL;
  else
    // SoBaseColor
    if (g->getColorArray())
      if (ivStateMaterial) {
        ivMaterial = new SoBaseColor;
        ((SoBaseColor*)ivMaterial)->rgb.setValue(ivStateMaterial->diffuseColor[0]); // copy first value
      } else
        ivMaterial = new SoBaseColor; // FIXME: check default values of SoBaseColor and OSG pre-lit scene
    else
      if (ivStateMaterial) {
        ivMaterial = new SoBaseColor;
        ((SoBaseColor*)ivMaterial)->rgb.setValue(ivStateMaterial->diffuseColor[0]); // copy first value
      } else
        ivMaterial = NULL;

  // Convert color array into the SoMaterial
  if (g->getColorArray()) {
    assert(ivMaterial);

    // Choose correct color field
    SoMFColor *colorField;
    if (ivMaterial->isOfType(SoMaterial::getClassTypeId())) {
      if (vrml1Conversion && ivState->osgLighting==false) {

        // special case of pre-lit VRML1 scene
        ((SoMaterial*)ivMaterial)->ambientColor.setValue(0.f,0.f,0.f);
        ((SoMaterial*)ivMaterial)->diffuseColor.setValue(0.f,0.f,0.f);
        ((SoMaterial*)ivMaterial)->specularColor.setValue(0.f,0.f,0.f);
        colorField = &((SoMaterial*)ivMaterial)->emissiveColor;
      } else
        // regular diffuse color
        colorField = &((SoMaterial*)ivMaterial)->diffuseColor;
    } else
      // Using of SoBaseColor
      colorField = &((SoBaseColor*)ivMaterial)->rgb;


    // Color array with material
    if (ivState->osgMaterial == NULL ||
        ivState->osgMaterial->getColorMode() == osg::Material::DIFFUSE ||
        ivState->osgMaterial->getColorMode() == osg::Material::AMBIENT_AND_DIFFUSE)
      osgArray2ivMField(g->getColorArray(), *colorField);
    else; // FIXME: implement some workaround for non-diffuse cases?
          // note: Warning was already shown in createInventorState().
          // note2: There is no effect to convert SoMaterial::[ambient|specular|emissive]color
          // here because Inventor does not set them per-vertex (performance reasons). See
          // Inventor documentation for more details.
  }


  // Convert coordinates
  // OSG represents coordinates by: Vec2, Vec3, Vec4
  // Inventor by: SbVec3f, SbVec4f
  SoNode *coords;
  if (g->getVertexArray()->getDataSize() == 4) {
    coords = new SoCoordinate4;
    osgArray2ivMField(g->getVertexArray(), ((SoCoordinate4*)coords)->point);
  } else {
    coords = new SoCoordinate3;
    osgArray2ivMField(g->getVertexArray(), ((SoCoordinate3*)coords)->point);
  }
  coords->ref();

  // Convert texture coordinates
  SoNode *texCoords = NULL;
  if (ivState->ivTexture) {
    if (ivState->osgTexGenS && ivState->osgTexGenT &&
        ivState->osgTexGen && ivState->osgTexGen->getMode()==osg::TexGen::SPHERE_MAP)
      texCoords = new SoTextureCoordinateEnvironment;
    else
    if (g->getTexCoordArray(0)) {
      if (g->getTexCoordArray(0)->getDataSize() <= 2) {
        texCoords = new SoTextureCoordinate2;
        osgArray2ivMField(g->getTexCoordArray(0), ((SoTextureCoordinate2*)texCoords)->point);
      }
#ifdef __COIN__
      else {
        texCoords = new SoTextureCoordinate3;
        osgArray2ivMField(g->getTexCoordArray(0), ((SoTextureCoordinate3*)texCoords)->point);
      }
#endif   // __COIN__
    }
    if (texCoords)
      texCoords->ref();
  }

  // Convert normals
  // OSG represents normals by: Vec3,Vec3s,Vec3b
  // and can handle: Vec4s,Vec4b by truncating them to three components
  // Inventor by: SbVec3f
  SoNormal *normals = NULL;
  if (g->getNormalArray()) {
    normals = new SoNormal;
    osgArray2ivMField(g->getNormalArray(), normals->vector);
    normals->ref();
  }

  // Convert osg::PrimitiveSets to Inventor's SoShapes
  int psetIndex,numPsets = g->getNumPrimitiveSets();
  for (psetIndex=0; psetIndex<numPsets; psetIndex++) {

    // Get PrimitiveSet
    const osg::PrimitiveSet *pset = g->getPrimitiveSet(psetIndex);
    osg::PrimitiveSet::Type type = pset->getType();
    GLenum mode = pset->getMode();

    // Create appropriate SoShape
    bool useIndices = vrml1Conversion;
    bool needSeparateTriangles = false;
    SoVertexShape *shape = NULL;
    switch (mode) {
      case GL_POINTS:         shape = new SoPointSet; break;
      case GL_LINES:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:      if (useIndices) shape = new SoIndexedLineSet;
                              else shape = new SoLineSet;
                              break;
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_QUAD_STRIP:     if (useIndices)
                                if (vrml1Conversion) {
                                  shape = new SoIndexedFaceSet;
                                  needSeparateTriangles = true;
                                } else
                                  shape = new SoIndexedTriangleStripSet;
                              else
                                shape = new SoTriangleStripSet;
                              break;
      case GL_TRIANGLE_FAN:   needSeparateTriangles = true;
                              shape = (vrml1Conversion) ? (SoVertexShape*)new SoIndexedFaceSet :
                                                          new SoIndexedTriangleStripSet;
                              break;
      case GL_QUADS:
      case GL_POLYGON:        if (useIndices) shape = new SoIndexedFaceSet;
                              else shape = new SoFaceSet;
                              break;
      default: assert(0);
    }

    // Size of single geometric primitive
    int primSize;
    switch (mode) {
    case GL_LINES:          primSize = 2; break;
    case GL_TRIANGLES:      primSize = 3; break;
    case GL_QUADS:          primSize = 4; break;
    default: primSize = 0;
    };


    bool ok = true;

    switch (type) {

      case osg::PrimitiveSet::DrawArraysPrimitiveType:
      {
        const osg::DrawArrays *drawArrays = dynamic_cast<const osg::DrawArrays*>(pset);

        int startIndex = drawArrays->getFirst();
        int stopIndex = startIndex + drawArrays->getCount();

        // FIXME: Am I using startIndex for all bundles that are PER_VERTEX?
        ok = processPrimitiveSet(g, pset, NULL, needSeparateTriangles,
                                  drawArrays->getCount(), primSize,
                                  startIndex, stopIndex, normalIndex, colorIndex,
                                  coords, normals, ivMaterial, texCoords,
                                  ivState->ivTexture, shape, indexedRoot, nonIndexedRoot);
        if (!ok)
        {
            OSG_WARN<<"Inventor plugin, ConvertToInventor processPrimitiveSet() failed."<<std::endl;
        }
        break;
      }

      case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
      {
        const osg::DrawArrayLengths *drawArrayLengths =
          dynamic_cast<const osg::DrawArrayLengths*>(pset);

        int startIndex = drawArrayLengths->getFirst();

        ok = processPrimitiveSet(g, pset, NULL, needSeparateTriangles,
                                  -1, primSize, startIndex, -1, normalIndex, colorIndex,
                                  coords, normals, ivMaterial, texCoords,
                                  ivState->ivTexture, shape, indexedRoot, nonIndexedRoot);
        if (!ok)
        {
            OSG_WARN<<"Inventor plugin, ConvertToInventor processPrimitiveSet() failed."<<std::endl;
        }

        break;
      }

      case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
      case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
      case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
      {
        osg::ref_ptr<osg::UIntArray> drawElemIndices = new osg::UIntArray;

        switch (type) {
        case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
          {
            const osg::DrawElementsUByte *drawElements =
              dynamic_cast<const osg::DrawElementsUByte*>(pset);
            for(osg::DrawElementsUByte::const_iterator primItr = drawElements->begin();
                primItr!=drawElements->end();
                ++primItr)
              drawElemIndices->push_back(*primItr);
            break;
          }
        case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
          {
            const osg::DrawElementsUShort *drawElements =
              dynamic_cast<const osg::DrawElementsUShort*>(pset);
            for(osg::DrawElementsUShort::const_iterator primItr = drawElements->begin();
                primItr!=drawElements->end();
                ++primItr)
              drawElemIndices->push_back(*primItr);
            break;
          }
        case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
          {
            const osg::DrawElementsUInt *drawElements =
              dynamic_cast<const osg::DrawElementsUInt*>(pset);
            for(osg::DrawElementsUInt::const_iterator primItr = drawElements->begin();
                primItr!=drawElements->end();
                ++primItr)
              drawElemIndices->push_back(*primItr);
            break;
          }
        default: assert(0);
        }

        ok = processPrimitiveSet(g, pset, drawElemIndices.get(), needSeparateTriangles,
                                  drawElemIndices->getNumElements(), primSize,
                                  0, drawElemIndices->getNumElements(), normalIndex, colorIndex,
                                  coords, normals, ivMaterial, texCoords,
                                  ivState->ivTexture, shape, indexedRoot, nonIndexedRoot);
        if (!ok)
        {
            OSG_WARN<<"Inventor plugin, ConvertToInventor processPrimitiveSet() failed."<<std::endl;
        }
        break;
      }

      default:
        OSG_WARN << "IvWriter: NOT IMPLEMENTED" << std::endl;
    }
  }

  if (indexedRoot)  ivState->ivHead->addChild(indexedRoot);
  if (nonIndexedRoot)  ivState->ivHead->addChild(nonIndexedRoot);

  coords->unref();
  if (texCoords)  texCoords->unref();
  if (normals)  normals->unref();
}


void ConvertToInventor::processShapeDrawable(const osg::ShapeDrawable *d, InventorState *ivState)
{
  // visitor for converting ShapeDrawables
  class MyShapeVisitor : public osg::ConstShapeVisitor {
  public:
    void processNode(SoNode *ivNode, const osg::Vec3& center, osg::Quat rotation,
                      SoGroup *root) {
      // convert rotation
      rotation = osg::Quat(-M_PI_2, osg::Vec3(0.,1.,0.)) * osg::Quat(M_PI_2, osg::Vec3(1.,0.,0.)) * rotation;

      if (center.length2()==0. && rotation.zeroRotation() && ivState->ivTexture==NULL)

        // optimized handling of single node
        root->addChild(ivNode);

      else {
        SoSeparator *root2 = new SoSeparator;

        // handle transformation
        if (center.length2()!=0. || !rotation.zeroRotation()) {
          SoTransform *ivTransform = new SoTransform;
          setSoTransform(ivTransform, center, rotation);
          root2->addChild(ivTransform);
        }

        // handle texture
        if (ivState->ivTexture)
          root2->addChild(ivState->ivTexture);

        // build graph
        root2->addChild(ivNode);
        root->addChild(root2);
      }
    }

    virtual void apply(const osg::Sphere &s) {
      SoSphere *ivSphere = new SoSphere;
      ivSphere->radius.setValue(s.getRadius());
      processNode(ivSphere, s.getCenter(), osg::Quat(0., osg::Vec3(1.,0.,0.)), ivState->ivHead);
    }
    virtual void apply(const osg::Box &b) {
      SoCube *ivCube = new SoCube;
      ivCube->width  = 2 * b.getHalfLengths().y();
      ivCube->height = 2 * b.getHalfLengths().z();
      ivCube->depth  = 2 * b.getHalfLengths().x();
      processNode(ivCube, b.getCenter(), b.getRotation(), ivState->ivHead);
    }
    virtual void apply(const osg::Cone &c) {
      SoCone *ivCone = new SoCone;
      ivCone->bottomRadius = c.getRadius();
      ivCone->height = c.getHeight();
      osg::Vec3 newCenter(c.getCenter());
      newCenter.ptr()[2] -= c.getBaseOffset();
      processNode(ivCone, newCenter, c.getRotation(), ivState->ivHead);
    }
    virtual void apply(const osg::Cylinder &c) {
      SoCylinder *ivCylinder = new SoCylinder;
      ivCylinder->radius = c.getRadius();
      ivCylinder->height = c.getHeight();
      processNode(ivCylinder, c.getCenter(), c.getRotation(), ivState->ivHead);
    }

    void warnNonSupported() {
      OSG_WARN << "IvWriter: Not supported ShapeDrawable found. Skipping it." << std::endl;
    }
    virtual void apply(const osg::Capsule&)        { warnNonSupported(); }
    virtual void apply(const osg::InfinitePlane&)  { warnNonSupported(); }
    virtual void apply(const osg::TriangleMesh&)   { warnNonSupported(); }
    virtual void apply(const osg::ConvexHull&)     { warnNonSupported(); }
    virtual void apply(const osg::HeightField&)    { warnNonSupported(); }
    virtual void apply(const osg::CompositeShape&) { warnNonSupported(); }

    InventorState *ivState;
    MyShapeVisitor(InventorState *ivState) { this->ivState = ivState; }
  } shapeVisitor(ivState);

  // convert ShapeDrawable
  const osg::Shape *shape = d->getShape();
  if (shape)
    shape->accept(shapeVisitor);
}


void ConvertToInventor::processDrawable(osg::Drawable *d)
{
  deprecated_osg::Geometry *g = dynamic_cast<deprecated_osg::Geometry*>(d); // FIXME: other drawables have to be handled also
  osg::ShapeDrawable *sd = 0;

  // Create SoSeparator and convert StateSet for Drawable
  InventorState *ivDrawableState = createInventorState(d->getStateSet());

  if (g != NULL)
    processGeometry(g, ivDrawableState);
  else

  if ((sd = dynamic_cast<osg::ShapeDrawable*>(d)) != NULL) {
    processShapeDrawable(sd, ivDrawableState);
  }
  else
    OSG_WARN << "IvWriter: Unsupported drawable found: \"" << d->className() <<
                              "\". Skipping it." << std::endl;

  // Restore state
  popInventorState();
}


void ConvertToInventor::apply(osg::Geode &node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: Geode traversed" << std::endl;
#endif

  // Create SoSeparator and convert StateSet for Geode
  /*InventorState *ivGeodeState = */createInventorState(node.getStateSet());

  // Convert drawables
  const int numDrawables = node.getNumDrawables();
  for (int i=0; i<numDrawables; i++)
    processDrawable(node.getDrawable(i));

  traverse(node);

  // Restore state
  popInventorState();
}


void ConvertToInventor::apply(osg::Group &node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: Group traversed" << std::endl;
#endif

  // Create SoSeparator and convert StateSet
  /*InventorState *ivState = */createInventorState(node.getStateSet());

  traverse(node);

  popInventorState();
}


void ConvertToInventor::apply(osg::Billboard& node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: Billboard traversed" << std::endl;
#endif

#ifdef __COIN__

  if (useIvExtensions) {

    // Create SoSeparator and convert StateSet
    InventorState *ivState = createInventorState(node.getStateSet());
    SoGroup *root = ivState->ivHead;

    // Process drawables
    const int numDrawables = node.getNumDrawables();
    for (int i=0; i<numDrawables; i++) {

      SoVRMLBillboard *billboard = new SoVRMLBillboard;

      // SoVRMLBillboard is VRML 2.0 node supported by Coin (?since 2.0?)
      // However, I am seeing bug in my Coin 2.4.5 so that if
      // SoVRMLBillboard::axisOfRotation is not 0,0,0, the billboard behaviour is strange.
      // As long as it is set to 0,0,0, POINT_ROT_EYE-style billboard works perfectly.
      // AXIAL_ROT seems not possible with the bug. And POINT_ROT_WORLD was not
      // investigated by me until now.
      // There is also billboard culling bug in Coin, so the billboards may not be
      // rendered properly from time to time. PCJohn-2007-09-08
    #if 0
      SbVec3f axis;
      switch (node.getMode()) {
        case osg::Billboard::POINT_ROT_EYE:   axis = SbVec3f(0.f,0.f,0.f); break;
        case osg::Billboard::POINT_ROT_WORLD: axis = SbVec3f(0.f,0.f,0.f); break;
        case osg::Billboard::AXIAL_ROT:       axis = node.getAxis().ptr(); break;
        default:
          axis = SbVec3f(0.f,0.f,0.f);
      };
      billboard->axisOfRotation.setValue(axis);
    #else

      billboard->axisOfRotation.setValue(SbVec3f(0.f,0.f,0.f));

    #endif

      SoTranslation *translation = new SoTranslation;
      translation->translation.setValue(node.getPosition(i).ptr());

      // Rotate billboard correctly (OSG->IV conversion)
      // Note: use SoTransform instead of SoRotation because SoRotation is not supported by VRML1.
      SoTransform *transform = new SoTransform;
      transform->rotation = SbRotation(SbVec3f(1.f,0.f,0.f), float(-M_PI_2));

      SoSeparator *separator = new SoSeparator;
      separator->addChild(translation);
      separator->addChild(billboard);
      billboard->addChild(transform);

      root->addChild(separator);
      ivState->ivHead = billboard;

      processDrawable(node.getDrawable(i));

      traverse((osg::Node&)node);
    }

    popInventorState();

  } else
    apply((osg::Geode&)node);

#else

  apply((osg::Geode&)node);

#endif
}


void ConvertToInventor::apply(osg::MatrixTransform& node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: MatrixTransform traversed" << std::endl;
#endif

  // Convert matrix
  SoMatrixTransform *ivTransform = new SoMatrixTransform;
  SbMatrix ivMatrix;
  const osg::Matrix::value_type *src = node.getMatrix().ptr();
  float *dest = ivMatrix[0];
  for (int i=0; i<16; i++,dest++,src++)
    *dest = *src;
  ivTransform->matrix.setValue(ivMatrix);

  // Create SoSeparator and convert StateSet
  InventorState *ivState = createInventorState(node.getStateSet());
  ivState->ivHead->addChild(ivTransform);

  traverse((osg::Node&)node);

  popInventorState();
}


void ConvertToInventor::apply(osg::PositionAttitudeTransform& node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: PositionAttitudeTransform traversed" << std::endl;
#endif

  // Convert matrix
  SoTransform *ivTransform = new SoTransform;
  setSoTransform(ivTransform, node.getPosition(), node.getAttitude(), node.getScale());

  // Create SoSeparator and convert StateSet
  InventorState *ivState = createInventorState(node.getStateSet());
  ivState->ivHead->addChild(ivTransform);

  traverse((osg::Node&)node);

  popInventorState();
}


void ConvertToInventor::apply(osg::LOD& node)
{
#ifdef DEBUG_IV_WRITER
  OSG_INFO << "IvWriter: LOD traversed" << std::endl;
#endif

  // Convert LOD
  SoGroup *ivLOD = NULL;
  osg::LOD::RangeMode rangeMode = node.getRangeMode();
  if (rangeMode == osg::LOD::DISTANCE_FROM_EYE_POINT) {

    // use SoLOD for DISTANCE_FROM_EYE_POINT
    SoLOD *lod = new SoLOD;

    // copy ranges
    int i,c=node.getNumRanges();
    for (i=0; i<c; i++)
       lod->range.set1Value(i, node.getMaxRange(i));

    // set center
    osg::Vec3f center(node.getCenter());
    lod->center.setValue(center.ptr());

    ivLOD = lod;

  } else
  if (rangeMode == osg::LOD::PIXEL_SIZE_ON_SCREEN) {

    // use SoLevelOfDetail for PIXEL_SIZE_ON_SCREEN
    SoLevelOfDetail *lod = new SoLevelOfDetail;

    // copy ranges
    int i,c=node.getNumRanges();
    for (i=0; i<c; i++)
       lod->screenArea.set1Value(i, node.getMaxRange(i));

    ivLOD = lod;

  } else {

    // undefined mode -> put warning
    OSG_WARN << "IvWriter: Undefined LOD::RangeMode value." << std::endl;
    ivLOD = new SoGroup;
  }

  // Create SoSeparator and convert StateSet
  InventorState *ivState = createInventorState(node.getStateSet());
  ivState->ivHead->addChild(ivLOD);
  ivState->ivHead = ivLOD;

  traverse((osg::Node&)node);

  popInventorState();
}

