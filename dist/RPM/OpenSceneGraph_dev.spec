
Summary: A C++ scene graph API on OpenGL for real time graphics
Name: OpenSceneGraph_dev
Version: 0.8.44
Release: 1
Copyright: LGPL
Group: Graphics
Source: OpenSceneGraph_dev-0.8.44.tar.gz
URL: http://www.openscenegraph.org
Packager: Don Burns

%description

The Open Scene Graph is a cross-platform C++/OpenGL library for the real-time 
visualization. Uses range from visual simulation, scientific modeling, virtual 
reality through to games.  Open Scene Graph employs good practices in software
engineering through the use of standard C++, STL and generic programming, and
design patterns.  Open Scene Graph strives for high performance and quality in
graphics rendering, protability, and extensibility

%prep

%setup

%build

%install

cd /usr/src/redhat/BUILD/OpenSceneGraph_dev-0.8.44 
tar cvf - . | tar xvfC - /

# ---------------------
# FILES Sections
%files

%attr(755, root, root) /usr/local/include/osg/AlphaFunc
%attr(755, root, root) /usr/local/include/osg/AnimationPath
%attr(755, root, root) /usr/local/include/osg/Billboard
%attr(755, root, root) /usr/local/include/osg/BoundingBox
%attr(755, root, root) /usr/local/include/osg/BoundingSphere
%attr(755, root, root) /usr/local/include/osg/BoundsChecking
%attr(755, root, root) /usr/local/include/osg/Camera
%attr(755, root, root) /usr/local/include/osg/ClippingVolume
%attr(755, root, root) /usr/local/include/osg/ClipPlane
%attr(755, root, root) /usr/local/include/osg/ColorMask
%attr(755, root, root) /usr/local/include/osg/ColorMatrix
%attr(755, root, root) /usr/local/include/osg/CopyOp
%attr(755, root, root) /usr/local/include/osg/CullFace
%attr(755, root, root) /usr/local/include/osg/Depth
%attr(755, root, root) /usr/local/include/osg/DisplaySettings
%attr(755, root, root) /usr/local/include/osg/Drawable
%attr(755, root, root) /usr/local/include/osg/DrawPixels
%attr(755, root, root) /usr/local/include/osg/EarthSky
%attr(755, root, root) /usr/local/include/osg/Export
%attr(755, root, root) /usr/local/include/osg/Fog
%attr(755, root, root) /usr/local/include/osg/FrameStamp
%attr(755, root, root) /usr/local/include/osg/FrontFace
%attr(755, root, root) /usr/local/include/osg/Geode
%attr(755, root, root) /usr/local/include/osg/GeoSet
%attr(755, root, root) /usr/local/include/osg/GL
%attr(755, root, root) /usr/local/include/osg/GLExtensions
%attr(755, root, root) /usr/local/include/osg/GLU
%attr(755, root, root) /usr/local/include/osg/Group
%attr(755, root, root) /usr/local/include/osg/Image
%attr(755, root, root) /usr/local/include/osg/Impostor
%attr(755, root, root) /usr/local/include/osg/ImpostorSprite
%attr(755, root, root) /usr/local/include/osg/Light
%attr(755, root, root) /usr/local/include/osg/LightModel
%attr(755, root, root) /usr/local/include/osg/LightSource
%attr(755, root, root) /usr/local/include/osg/LineSegment
%attr(755, root, root) /usr/local/include/osg/LineStipple
%attr(755, root, root) /usr/local/include/osg/LineWidth
%attr(755, root, root) /usr/local/include/osg/LOD
%attr(755, root, root) /usr/local/include/osg/Material
%attr(755, root, root) /usr/local/include/osg/Math
%attr(755, root, root) /usr/local/include/osg/Matrix
%attr(755, root, root) /usr/local/include/osg/MemoryManager
%attr(755, root, root) /usr/local/include/osg/Node
%attr(755, root, root) /usr/local/include/osg/NodeCallback
%attr(755, root, root) /usr/local/include/osg/NodeVisitor
%attr(755, root, root) /usr/local/include/osg/Notify
%attr(755, root, root) /usr/local/include/osg/Object
%attr(755, root, root) /usr/local/include/osg/Plane
%attr(755, root, root) /usr/local/include/osg/Point
%attr(755, root, root) /usr/local/include/osg/PolygonMode
%attr(755, root, root) /usr/local/include/osg/PolygonOffset
%attr(755, root, root) /usr/local/include/osg/PositionAttitudeTransform
%attr(755, root, root) /usr/local/include/osg/Projection
%attr(755, root, root) /usr/local/include/osg/Quat
%attr(755, root, root) /usr/local/include/osg/Referenced
%attr(755, root, root) /usr/local/include/osg/ref_ptr
%attr(755, root, root) /usr/local/include/osg/ShadeModel
%attr(755, root, root) /usr/local/include/osg/State
%attr(755, root, root) /usr/local/include/osg/StateAttribute
%attr(755, root, root) /usr/local/include/osg/StateSet
%attr(755, root, root) /usr/local/include/osg/Statistics
%attr(755, root, root) /usr/local/include/osg/Stencil
%attr(755, root, root) /usr/local/include/osg/Switch
%attr(755, root, root) /usr/local/include/osg/TexEnv
%attr(755, root, root) /usr/local/include/osg/TexGen
%attr(755, root, root) /usr/local/include/osg/TexMat
%attr(755, root, root) /usr/local/include/osg/Texture
%attr(755, root, root) /usr/local/include/osg/TextureCubeMap
%attr(755, root, root) /usr/local/include/osg/Timer
%attr(755, root, root) /usr/local/include/osg/Transform
%attr(755, root, root) /usr/local/include/osg/Transparency
%attr(755, root, root) /usr/local/include/osg/Types
%attr(755, root, root) /usr/local/include/osg/Vec2
%attr(755, root, root) /usr/local/include/osg/Vec3
%attr(755, root, root) /usr/local/include/osg/Vec4
%attr(755, root, root) /usr/local/include/osg/Version
%attr(755, root, root) /usr/local/include/osg/Viewport
%attr(755, root, root) /usr/local/include/osgDB/DotOsgWrapper
%attr(755, root, root) /usr/local/include/osgDB/DynamicLibrary
%attr(755, root, root) /usr/local/include/osgDB/Export
%attr(755, root, root) /usr/local/include/osgDB/Field
%attr(755, root, root) /usr/local/include/osgDB/FieldReader
%attr(755, root, root) /usr/local/include/osgDB/FieldReaderIterator
%attr(755, root, root) /usr/local/include/osgDB/FileNameUtils
%attr(755, root, root) /usr/local/include/osgDB/FileUtils
%attr(755, root, root) /usr/local/include/osgDB/Input
%attr(755, root, root) /usr/local/include/osgDB/Output
%attr(755, root, root) /usr/local/include/osgDB/ReaderWriter
%attr(755, root, root) /usr/local/include/osgDB/ReadFile
%attr(755, root, root) /usr/local/include/osgDB/Registry
%attr(755, root, root) /usr/local/include/osgDB/Version
%attr(755, root, root) /usr/local/include/osgDB/WriteFile
%attr(755, root, root) /usr/local/include/osgGLUT/Export
%attr(755, root, root) /usr/local/include/osgGLUT/glut
%attr(755, root, root) /usr/local/include/osgGLUT/GLUTEventAdapter
%attr(755, root, root) /usr/local/include/osgGLUT/Version
%attr(755, root, root) /usr/local/include/osgGLUT/Viewer
%attr(755, root, root) /usr/local/include/osgGLUT/Window
%attr(755, root, root) /usr/local/include/osgText/Export
%attr(755, root, root) /usr/local/include/osgText/Font
%attr(755, root, root) /usr/local/include/osgText/Paragraph
%attr(755, root, root) /usr/local/include/osgText/Text
%attr(755, root, root) /usr/local/include/osgText/Version
%attr(755, root, root) /usr/local/include/osgUtil/AppVisitor
%attr(755, root, root) /usr/local/include/osgUtil/CameraManipulator
%attr(755, root, root) /usr/local/include/osgUtil/CullViewState
%attr(755, root, root) /usr/local/include/osgUtil/CullVisitor
%attr(755, root, root) /usr/local/include/osgUtil/DepthSortedBin
%attr(755, root, root) /usr/local/include/osgUtil/DisplayListVisitor
%attr(755, root, root) /usr/local/include/osgUtil/DisplayRequirementsVisitor
%attr(755, root, root) /usr/local/include/osgUtil/DriveManipulator
%attr(755, root, root) /usr/local/include/osgUtil/Export
%attr(755, root, root) /usr/local/include/osgUtil/FlightManipulator
%attr(755, root, root) /usr/local/include/osgUtil/GUIActionAdapter
%attr(755, root, root) /usr/local/include/osgUtil/GUIEventAdapter
%attr(755, root, root) /usr/local/include/osgUtil/GUIEventHandler
%attr(755, root, root) /usr/local/include/osgUtil/InsertImpostorsVisitor
%attr(755, root, root) /usr/local/include/osgUtil/IntersectVisitor
%attr(755, root, root) /usr/local/include/osgUtil/Optimizer
%attr(755, root, root) /usr/local/include/osgUtil/RenderBin
%attr(755, root, root) /usr/local/include/osgUtil/RenderGraph
%attr(755, root, root) /usr/local/include/osgUtil/RenderLeaf
%attr(755, root, root) /usr/local/include/osgUtil/RenderStage
%attr(755, root, root) /usr/local/include/osgUtil/RenderStageLighting
%attr(755, root, root) /usr/local/include/osgUtil/RenderToTextureStage
%attr(755, root, root) /usr/local/include/osgUtil/SceneView
%attr(755, root, root) /usr/local/include/osgUtil/SceneViewManipulator
%attr(755, root, root) /usr/local/include/osgUtil/SmoothingVisitor
%attr(755, root, root) /usr/local/include/osgUtil/StateSetManipulator
%attr(755, root, root) /usr/local/include/osgUtil/Tesselator
%attr(755, root, root) /usr/local/include/osgUtil/TrackballManipulator
%attr(755, root, root) /usr/local/include/osgUtil/TransformCallback
%attr(755, root, root) /usr/local/include/osgUtil/TriStripVisitor
%attr(755, root, root) /usr/local/include/osgUtil/Version
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/base.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/GliderManipulator.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/GliderManipulator.h
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/hangglide.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/hat.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/hat.h
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/ReaderWriterFLY.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/sky.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/tank.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/terrain.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/terrain_data.h
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/hangglide/trees.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgbillboard/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgbillboard/osgbillboard.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcluster/broadcaster.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcluster/broadcaster.h
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcluster/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcluster/osgcluster.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcluster/receiver.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcluster/receiver.h
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgconv/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgconv/OrientationConverter.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgconv/OrientationConverter.h
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgconv/osgconv.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcopy/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcopy/osgcopy.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcube/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgcube/osgcube.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osghud/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osghud/osghud.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgimpostor/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgimpostor/osgimpostor.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgreflect/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgreflect/osgreflect.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgscribe/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgscribe/osgscribe.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgstereoimage/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgstereoimage/osgstereoimage.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgtext/main.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgtext/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgtexture/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgtexture/osgtexture.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgversion/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgversion/osgversion.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgviews/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/osgviews/osgviews.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/sgv/Makefile
%attr(755, root, root) /usr/share/OpenSceneGraph/src/demos/sgv/sgv.cpp
%attr(755, root, root) /usr/share/OpenSceneGraph/src/Make/makedefs
%attr(755, root, root) /usr/share/OpenSceneGraph/src/Make/makerules
