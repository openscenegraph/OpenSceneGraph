
Summary: A C++ scene graph API on OpenGL for real time graphics
Name: OpenSceneGraph_dev
Version: 0.9.2
Release: 1
Copyright: LGPL
Group: Graphics
Source: OpenSceneGraph_dev-0.9.2.tar.gz
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

cd /usr/src/redhat/BUILD/OpenSceneGraph_dev-0.9.2 
tar cvf - . | tar xvfC - /

# ---------------------
# FILES Sections
%files

%attr(755, root, root) /usr/local/include/osg/AlphaFunc
%attr(755, root, root) /usr/local/include/osg/AnimationPath
%attr(755, root, root) /usr/local/include/osg/Array
%attr(755, root, root) /usr/local/include/osg/Billboard
%attr(755, root, root) /usr/local/include/osg/BlendFunc
%attr(755, root, root) /usr/local/include/osg/BoundingBox
%attr(755, root, root) /usr/local/include/osg/BoundingSphere
%attr(755, root, root) /usr/local/include/osg/BoundsChecking
%attr(755, root, root) /usr/local/include/osg/buffered_value
%attr(755, root, root) /usr/local/include/osg/Camera
%attr(755, root, root) /usr/local/include/osg/ClearNode
%attr(755, root, root) /usr/local/include/osg/ClipNode
%attr(755, root, root) /usr/local/include/osg/ClipPlane
%attr(755, root, root) /usr/local/include/osg/CollectOccludersVisitor
%attr(755, root, root) /usr/local/include/osg/ColorMask
%attr(755, root, root) /usr/local/include/osg/ColorMatrix
%attr(755, root, root) /usr/local/include/osg/ConvexPlanarOccluder
%attr(755, root, root) /usr/local/include/osg/ConvexPlanarPolygon
%attr(755, root, root) /usr/local/include/osg/CopyOp
%attr(755, root, root) /usr/local/include/osg/CullFace
%attr(755, root, root) /usr/local/include/osg/CullingSet
%attr(755, root, root) /usr/local/include/osg/CullStack
%attr(755, root, root) /usr/local/include/osg/Depth
%attr(755, root, root) /usr/local/include/osg/DisplaySettings
%attr(755, root, root) /usr/local/include/osg/DOFTransform
%attr(755, root, root) /usr/local/include/osg/Drawable
%attr(755, root, root) /usr/local/include/osg/DrawPixels
%attr(755, root, root) /usr/local/include/osg/Export
%attr(755, root, root) /usr/local/include/osg/fast_back_stack
%attr(755, root, root) /usr/local/include/osg/Fog
%attr(755, root, root) /usr/local/include/osg/FrameStamp
%attr(755, root, root) /usr/local/include/osg/FrontFace
%attr(755, root, root) /usr/local/include/osg/Geode
%attr(755, root, root) /usr/local/include/osg/Geometry
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
%attr(755, root, root) /usr/local/include/osg/MatrixTransform
%attr(755, root, root) /usr/local/include/osg/MemoryManager
%attr(755, root, root) /usr/local/include/osg/Node
%attr(755, root, root) /usr/local/include/osg/NodeCallback
%attr(755, root, root) /usr/local/include/osg/NodeVisitor
%attr(755, root, root) /usr/local/include/osg/Notify
%attr(755, root, root) /usr/local/include/osg/Object
%attr(755, root, root) /usr/local/include/osg/OccluderNode
%attr(755, root, root) /usr/local/include/osg/Plane
%attr(755, root, root) /usr/local/include/osg/Point
%attr(755, root, root) /usr/local/include/osg/PolygonMode
%attr(755, root, root) /usr/local/include/osg/PolygonOffset
%attr(755, root, root) /usr/local/include/osg/PolygonStipple
%attr(755, root, root) /usr/local/include/osg/Polytope
%attr(755, root, root) /usr/local/include/osg/PositionAttitudeTransform
%attr(755, root, root) /usr/local/include/osg/PrimitiveSet
%attr(755, root, root) /usr/local/include/osg/Projection
%attr(755, root, root) /usr/local/include/osg/Quat
%attr(755, root, root) /usr/local/include/osg/Referenced
%attr(755, root, root) /usr/local/include/osg/ref_ptr
%attr(755, root, root) /usr/local/include/osg/Sequence
%attr(755, root, root) /usr/local/include/osg/ShadeModel
%attr(755, root, root) /usr/local/include/osg/ShadowVolumeOccluder
%attr(755, root, root) /usr/local/include/osg/Shape
%attr(755, root, root) /usr/local/include/osg/ShapeDrawable
%attr(755, root, root) /usr/local/include/osg/State
%attr(755, root, root) /usr/local/include/osg/StateAttribute
%attr(755, root, root) /usr/local/include/osg/StateSet
%attr(755, root, root) /usr/local/include/osg/Statistics
%attr(755, root, root) /usr/local/include/osg/Stencil
%attr(755, root, root) /usr/local/include/osg/Switch
%attr(755, root, root) /usr/local/include/osg/TexEnv
%attr(755, root, root) /usr/local/include/osg/TexEnvCombine
%attr(755, root, root) /usr/local/include/osg/TexGen
%attr(755, root, root) /usr/local/include/osg/TexMat
%attr(755, root, root) /usr/local/include/osg/Texture
%attr(755, root, root) /usr/local/include/osg/Texture1D
%attr(755, root, root) /usr/local/include/osg/Texture2D
%attr(755, root, root) /usr/local/include/osg/Texture3D
%attr(755, root, root) /usr/local/include/osg/TextureCubeMap
%attr(755, root, root) /usr/local/include/osg/Timer
%attr(755, root, root) /usr/local/include/osg/Transform
%attr(755, root, root) /usr/local/include/osg/Types
%attr(755, root, root) /usr/local/include/osg/UByte4
%attr(755, root, root) /usr/local/include/osg/UnitTestFramework
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
%attr(755, root, root) /usr/local/include/osgGA/AnimationPathManipulator
%attr(755, root, root) /usr/local/include/osgGA/CameraManipulator
%attr(755, root, root) /usr/local/include/osgGA/DriveManipulator
%attr(755, root, root) /usr/local/include/osgGA/Export
%attr(755, root, root) /usr/local/include/osgGA/FlightManipulator
%attr(755, root, root) /usr/local/include/osgGA/GUIActionAdapter
%attr(755, root, root) /usr/local/include/osgGA/GUIEventAdapter
%attr(755, root, root) /usr/local/include/osgGA/GUIEventHandler
%attr(755, root, root) /usr/local/include/osgGA/GUIEventHandlerVisitor
%attr(755, root, root) /usr/local/include/osgGA/KeySwitchCameraManipulator
%attr(755, root, root) /usr/local/include/osgGA/SetSceneViewVisitor
%attr(755, root, root) /usr/local/include/osgGA/StateSetManipulator
%attr(755, root, root) /usr/local/include/osgGA/TrackballManipulator
%attr(755, root, root) /usr/local/include/osgGA/Version
%attr(755, root, root) /usr/local/include/osgGLUT/Export
%attr(755, root, root) /usr/local/include/osgGLUT/glut
%attr(755, root, root) /usr/local/include/osgGLUT/GLUTEventAdapter
%attr(755, root, root) /usr/local/include/osgGLUT/Version
%attr(755, root, root) /usr/local/include/osgGLUT/Viewer
%attr(755, root, root) /usr/local/include/osgGLUT/Window
%attr(755, root, root) /usr/local/include/osgParticle/AccelOperator
%attr(755, root, root) /usr/local/include/osgParticle/CenteredPlacer
%attr(755, root, root) /usr/local/include/osgParticle/Counter
%attr(755, root, root) /usr/local/include/osgParticle/Emitter
%attr(755, root, root) /usr/local/include/osgParticle/Export
%attr(755, root, root) /usr/local/include/osgParticle/FluidFrictionOperator
%attr(755, root, root) /usr/local/include/osgParticle/ForceOperator
%attr(755, root, root) /usr/local/include/osgParticle/Interpolator
%attr(755, root, root) /usr/local/include/osgParticle/LinearInterpolator
%attr(755, root, root) /usr/local/include/osgParticle/ModularEmitter
%attr(755, root, root) /usr/local/include/osgParticle/ModularProgram
%attr(755, root, root) /usr/local/include/osgParticle/MultiSegmentPlacer
%attr(755, root, root) /usr/local/include/osgParticle/Operator
%attr(755, root, root) /usr/local/include/osgParticle/Particle
%attr(755, root, root) /usr/local/include/osgParticle/ParticleProcessor
%attr(755, root, root) /usr/local/include/osgParticle/ParticleSystem
%attr(755, root, root) /usr/local/include/osgParticle/ParticleSystemUpdater
%attr(755, root, root) /usr/local/include/osgParticle/Placer
%attr(755, root, root) /usr/local/include/osgParticle/PointPlacer
%attr(755, root, root) /usr/local/include/osgParticle/Program
%attr(755, root, root) /usr/local/include/osgParticle/RadialShooter
%attr(755, root, root) /usr/local/include/osgParticle/RandomRateCounter
%attr(755, root, root) /usr/local/include/osgParticle/range
%attr(755, root, root) /usr/local/include/osgParticle/SectorPlacer
%attr(755, root, root) /usr/local/include/osgParticle/SegmentPlacer
%attr(755, root, root) /usr/local/include/osgParticle/Shooter
%attr(755, root, root) /usr/local/include/osgParticle/VariableRateCounter
%attr(755, root, root) /usr/local/include/osgParticle/Version
%attr(755, root, root) /usr/local/include/osgSim/BlinkSequence
%attr(755, root, root) /usr/local/include/osgSim/Export
%attr(755, root, root) /usr/local/include/osgSim/LightPoint
%attr(755, root, root) /usr/local/include/osgSim/LightPointDrawable
%attr(755, root, root) /usr/local/include/osgSim/LightPointNode
%attr(755, root, root) /usr/local/include/osgSim/Sector
%attr(755, root, root) /usr/local/include/osgSim/Version
%attr(755, root, root) /usr/local/include/osgText/Export
%attr(755, root, root) /usr/local/include/osgText/Font
%attr(755, root, root) /usr/local/include/osgText/Paragraph
%attr(755, root, root) /usr/local/include/osgText/Text
%attr(755, root, root) /usr/local/include/osgText/Version
%attr(755, root, root) /usr/local/include/osgUtil/AppVisitor
%attr(755, root, root) /usr/local/include/osgUtil/CubeMapGenerator
%attr(755, root, root) /usr/local/include/osgUtil/CullVisitor
%attr(755, root, root) /usr/local/include/osgUtil/DisplayListVisitor
%attr(755, root, root) /usr/local/include/osgUtil/DisplayRequirementsVisitor
%attr(755, root, root) /usr/local/include/osgUtil/Export
%attr(755, root, root) /usr/local/include/osgUtil/HalfWayMapGenerator
%attr(755, root, root) /usr/local/include/osgUtil/HighlightMapGenerator
%attr(755, root, root) /usr/local/include/osgUtil/InsertImpostorsVisitor
%attr(755, root, root) /usr/local/include/osgUtil/IntersectVisitor
%attr(755, root, root) /usr/local/include/osgUtil/Optimizer
%attr(755, root, root) /usr/local/include/osgUtil/ReflectionMapGenerator
%attr(755, root, root) /usr/local/include/osgUtil/RenderBin
%attr(755, root, root) /usr/local/include/osgUtil/RenderGraph
%attr(755, root, root) /usr/local/include/osgUtil/RenderLeaf
%attr(755, root, root) /usr/local/include/osgUtil/RenderStage
%attr(755, root, root) /usr/local/include/osgUtil/RenderStageLighting
%attr(755, root, root) /usr/local/include/osgUtil/RenderToTextureStage
%attr(755, root, root) /usr/local/include/osgUtil/SceneView
%attr(755, root, root) /usr/local/include/osgUtil/SmoothingVisitor
%attr(755, root, root) /usr/local/include/osgUtil/Tesselator
%attr(755, root, root) /usr/local/include/osgUtil/TransformCallback
%attr(755, root, root) /usr/local/include/osgUtil/TriStripVisitor
%attr(755, root, root) /usr/local/include/osgUtil/Version
%attr(755, root, root) /usr/share/OpenSceneGraph/src/Make/makedefs
%attr(755, root, root) /usr/share/OpenSceneGraph/src/Make/makerules
