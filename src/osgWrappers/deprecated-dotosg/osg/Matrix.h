#ifndef DOTOSG_MATRIX
#define DOTOSG_MATRIX

#include <osg/Matrix>

#include <osgDB/Input>
#include <osgDB/Output>

extern bool readMatrix(osg::Matrix& matrix, osgDB::Input& fr, const char* keyword="Matrix");

extern bool writeMatrix(const osg::Matrix& matrix, osgDB::Output& fw, const char* keyword="Matrix");

#endif
