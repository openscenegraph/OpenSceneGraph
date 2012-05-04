ROOT=${PWD}
SOURCE_DIR=${PWD}/../../
INSTALL_DIR=${ROOT}/products
DEVICE=iphoneos
SIMULATOR=iphonesimulator
COMPILER=com.apple.compilers.llvmgcc42
ACTION="build"
CMAKE_OPTIONS="-D BUILD_OSG_APPLICATIONS:BOOL=OFF \
	-D OSG_WINDOWING_SYSTEM:STRING=IOS \
	-D OSG_GL1_AVAILABLE:BOOL=OFF \
	-D OSG_GL2_AVAILABLE:BOOL=OFF \
	-D OSG_GLES1_AVAILABLE:BOOL=ON \
	-D OSG_GL_DISPLAYLISTS_AVAILABLE:BOOL=OFF \
	-D OSG_GL_FIXED_FUNCTION_AVAILABLE:BOOL=ON \
	-D OSG_GL_LIBRARY_STATIC:BOOL=OFF \
	-D OSG_GL_MATRICES_AVAILABLE:BOOL=ON \
	-D OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE:BOOL=ON \
	-D OSG_GL_VERTEX_FUNCS_AVAILABLE:BOOL=OFF \
	-D CURL_INCLUDE_DIR:PATH="" \
	-D DYNAMIC_OPENSCENEGRAPH:BOOL=OFF \
	-D DYNAMIC_OPENTHREADS:BOOL=OFF "

XCODEBUILD=/Developer/usr/bin/xcodebuild
if [ ! -f ${XCODEBUILD} ]; then
  XCODEBUILD=/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild
fi

#create build dirs

mkdir -p ${ROOT}/build/device
mkdir -p ${ROOT}/build/simulator

# create install-dirs

mkdir -p ${INSTALL_DIR}/device
mkdir -p ${INSTALL_DIR}/simulator


create_project () {
	cd ${ROOT}/build/${1}
	
	/usr/bin/cmake -G Xcode \
		${CMAKE_OPTIONS} \
		-D ${2}:BOOL=ON \
		-D CMAKE_INSTALL_PREFIX:PATH="${INSTALL_DIR}/${1}" \
		${SOURCE_DIR}

}


build_project () {
  TARGET=${1}
  DEVICE=${2}
  ACTION=${3}

  ${XCODEBUILD} -configuration Debug -target "${TARGET}" -sdk ${DEVICE} ${ACTION} -parallelizeTargets RUN_CLANG_STATIC_ANALYZER=NO GCC_VERSION=${COMPILER}
  if [ $? -eq 1 ] ; then
    echo "compile went wrong"
    exit 1
  fi
  
  ${XCODEBUILD} -configuration Release -target "${TARGET}" -sdk ${DEVICE} ${ACTION} -parallelizeTargets RUN_CLANG_STATIC_ANALYZER=NO GCC_VERSION=${COMPILER}
  if [ $? -eq 1 ] ; then
    echo "compile went wrong"
    exit 1
  fi
  
}


#create xcode-projects
create_project device OSG_BUILD_PLATFORM_IPHONE
create_project simulator OSG_BUILD_PLATFORM_IPHONE_SIMULATOR

#build device
cd ${ROOT}/build/device
build_project install iphoneos build

