#!smake
include ../../Make/makedefs

C++FILES = \
	Registry.cpp\
	OSG.cpp\
	AlphaFunc.cpp\
	Group.cpp\
	Field.cpp\
	FieldReader.cpp\
	FieldReaderIterator.cpp\
	GeoSet.cpp\
	GeoSet_ogl.cpp\
	GeoState.cpp\
	Geode.cpp\
	Input.cpp\
        FileNameUtils.cpp\
	Light.cpp\
	LightSource.cpp\
	Lighting.cpp\
	Material.cpp\
	Matrix.cpp\
        Quat.cpp\
        Seg.cpp\
	Node.cpp\
	NodeVisitor.cpp\
	Object.cpp\
	Output.cpp\
        DynamicLibrary.cpp\
	TexEnv.cpp\
	TexGen.cpp\
	Image.cpp\
	Texture.cpp\
	DCS.cpp\
	Scene.cpp\
	Switch.cpp\
	Sequence.cpp\
	LOD.cpp\
	Billboard.cpp\
	BoundingSphere.cpp\
	BoundingBox.cpp\
	Transparency.cpp\
	CullFace.cpp\
	TexMat.cpp\
	Timer.cpp\
	Fog.cpp\
	ReaderWriterOSG.cpp\
	ReaderWriterRGB.cpp\
	Camera.cpp\
        Notify.cpp\
        ExtensionSupported.cpp\
        Point.cpp\
        PolygonOffset.cpp\
        Version.cpp\
        

TARGET_BASENAME = osg

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so

TARGET_INCLUDE_FILES = \
	osg/AlphaFunc\
	osg/Billboard\
	osg/BoundingBox\
	osg/BoundingSphere\
	osg/Camera\
	osg/CullFace\
	osg/DCS\
	osg/DynamicLibrary\
	osg/Export\
	osg/Field\
	osg/FieldReader\
	osg/FieldReaderIterator\
	osg/FileNameUtils\
	osg/Fog\
	osg/GeoSet\
	osg/GeoState\
	osg/Geode\
	osg/Group\
	osg/GL\
	osg/Image\
	osg/Input\
	osg/LOD\
	osg/Light\
	osg/LightSource\
	osg/Lighting\
	osg/Material\
	osg/Matrix\
	osg/Node\
	osg/Notify\
	osg/NodeVisitor\
	osg/OSG\
	osg/Object\
	osg/Output\
	osg/Point\
	osg/PolygonOffset\
	osg/Quat\
	osg/Referenced\
	osg/Registry\
	osg/Scene\
	osg/Seg\
	osg/Sequence\
	osg/State\
	osg/Switch\
	osg/TexEnv\
	osg/TexGen\
	osg/TexMat\
	osg/Texture\
	osg/Transparency\
	osg/Timer\
	osg/Types\
	osg/Vec2\
	osg/Vec3\
	osg/Vec4\
	osg/Version\


TARGET_DATA_FILES = \
	cessna.osg\
	cow.osg\
	dumptruck.osg\
	e-s-bike.osg\
	flight_park.fly\
	glider.osg\
	ncc1701d.osg\
	paraglider.osg\
	torus.osg\
	turtle.osg\
	Test/CullFace.osg\
	Test/Point.osg\
	Test/PolygonOffset.osg\
	Images/lz.rgb\
	Images/reflect.rgb\
	Images/tank.rgb\
	Images/tree0.rgba\
	Images/water.rgb\
	Images/white.rgb\


LIBS    = -ldl

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

C++FLAGS += -I ../../include

include ../../Make/makerules

