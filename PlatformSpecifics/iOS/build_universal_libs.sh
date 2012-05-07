ROOT=${PWD}
SOURCE_DIR=${PWD}/../../
INSTALL_DIR=${ROOT}/products
DEVICE=iphoneos
SIMULATOR=iphonesimulator
COMPILER=com.apple.compilers.llvmgcc42

UNIVERSAL_DIR=${INSTALL_DIR}/universal
DEVICE_DIR=${ROOT}/build/device
SIMULATOR_DIR=${ROOT}/build/simulator

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

CMAKE_DEVICE_OPTIONS="-DCMAKE_OSX_ARCHITECTURES:STRING=armv7"
CMAKE_SIMULATOR_OPTIONS=""

XCODEBUILD=/Developer/usr/bin/xcodebuild
if [ ! -f ${XCODEBUILD} ]; then
  XCODEBUILD=/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild
fi

#create build dirs

mkdir -p ${SIMULATOR_DIR}
mkdir -p ${DEVICE_DIR}

mkdir -p ${UNIVERSAL_DIR}/lib

mkdir -p ${INSTALL_DIR}/device
mkdir -p ${INSTALL_DIR}/simulator


create_project () {
	cd ${ROOT}/build/${1}
	
	/usr/bin/cmake -G Xcode \
		${CMAKE_OPTIONS} \
		${3} \
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

create_universal_lib () {
   FILE_NAME=${1}
   echo "creating universal lib '${FILE_NAME}'"

   lipo -create -output "${UNIVERSAL_DIR}/lib/${FILE_NAME}" "${DEVICE_DIR}/lib/${FILE_NAME}" "${SIMULATOR_DIR}/lib/${FILE_NAME}"
}



# create xcode-projects
create_project device OSG_BUILD_PLATFORM_IPHONE ${CMAKE_DEVICE_OPTIONS}
create_project simulator OSG_BUILD_PLATFORM_IPHONE_SIMULATOR ${CMAKE_SIMULATOR_OPTIONS}

# build device
cd ${ROOT}/build/device
build_project install ${DEVICE} build 

# build simulator
cd ${ROOT}/build/simulator
build_project install ${SIMULATOR} build


# create univeral libs
for i in $( ls "${DEVICE_DIR}/lib" ); 
do
   if [ -d $i ]
      then
            echo "ignoring directory ${i}"
      else
      		if [ ${i: -2} == ".a" ]
      		then
		      	create_universal_lib $i
			fi
    fi
done

# copy header files
cp -r ${INSTALL_DIR}/device/include ${UNIVERSAL_DIR}
rm -rf ${INSTALL_DIR}/device
rm -rf ${INSTALL_DIR}/simulator
