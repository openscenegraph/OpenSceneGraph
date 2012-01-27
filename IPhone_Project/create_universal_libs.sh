#!/bin/sh

SYMROOT="build"

DEBUG_DEVICE_DIR=${SYMROOT}/Debug-iphoneos
DEBUG_SIMULATOR_DIR=${SYMROOT}/Debug-iphonesimulator
DEBUG_UNIVERSAL_DIR=${SYMROOT}/Debug-universal

RELEASE_DEVICE_DIR=${SYMROOT}/Release-iphoneos
RELEASE_SIMULATOR_DIR=${SYMROOT}/Release-iphonesimulator
RELEASE_UNIVERSAL_DIR=${SYMROOT}/Release-universal

echo "deleting old universal dirs"
rm -rf "${DEBUG_UNIVERSAL_DIR}"
rm -rf "${RELEASE_UNIVERSAL_DIR}"
mkdir "${DEBUG_UNIVERSAL_DIR}"
mkdir "${RELEASE_UNIVERSAL_DIR}"

create_universal_lib () {
   FILE_NAME=lib${1}.a
   echo "creating universal lib '${FILE_NAME}'"

   lipo -create -output "${DEBUG_UNIVERSAL_DIR}/${FILE_NAME}" "${DEBUG_DEVICE_DIR}/${FILE_NAME}" "${DEBUG_SIMULATOR_DIR}/${FILE_NAME}"
   lipo -create -output "${RELEASE_UNIVERSAL_DIR}/${FILE_NAME}" "${RELEASE_DEVICE_DIR}/${FILE_NAME}" "${RELEASE_SIMULATOR_DIR}/${FILE_NAME}"
}

for i in \
  OpenThreads \
  osg \
  osgDB \
  osgGA \
  osgUtil \
  osgText \
  osgViewer \
  osgVolume \
  osgAnimation \
  osgTerrain \
  osgShadow \
  osgWidget \
  osgSim \
  osgParticle \
  osgPresentation \
  osgdb_imageio \
  osgdb_obj \
  osgdb_ive \
  osgdb_osg \
; do
   create_universal_lib $i
done

