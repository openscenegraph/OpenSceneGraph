
Summary: Open Scene Graph
Name: osg
Version: 0.8.44
Release: 1
Copyright: GLPL
Group: Graphics
Source: osg-0.8.44.tar.gz
URL: http://www.openscenegraph.org 
Packager: 

%description

Open Scene Graph is an open-source scene graph API.

%prep
%setup
%build
%install

cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/CVS /usr/bin/CVS
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgbillboard /usr/bin/osgbillboard
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgcluster /usr/bin/osgcluster
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgconv /usr/bin/osgconv
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgcopy /usr/bin/osgcopy
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgcube /usr/bin/osgcube
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osghangglide /usr/bin/osghangglide
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgimpostor /usr/bin/osgimpostor
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgreflect /usr/bin/osgreflect
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgscribe /usr/bin/osgscribe
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgstereoimage /usr/bin/osgstereoimage
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgtexture /usr/bin/osgtexture
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/osgviews /usr/bin/osgviews
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/bin/sgv /usr/bin/sgv
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/CVS /usr/lib/CVS
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/libosgDB.so /usr/lib/libosgDB.so
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/libosgGLUT.so /usr/lib/libosgGLUT.so
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/libosg.so /usr/lib/libosg.so
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/libosgText.so /usr/lib/libosgText.so
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/libosgUtil.so /usr/lib/libosgUtil.so
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/lib/osgPlugins /usr/lib/osgPlugins
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/include/CVS /usr/include/CVS
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/include/osg /usr/include/osg
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/include/osgDB /usr/include/osgDB
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/include/osgGLUT /usr/include/osgGLUT
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/include/osgText /usr/include/osgText
cp -rf /usr/src/redhat/BUILD/osg-0.8.44/include/osgUtil /usr/include/osgUtil



# ---------------------
# FILES Sections
%files

%attr(755, root, root) /usr/bin/CVS
%attr(755, root, root) /usr/bin/osgbillboard
%attr(755, root, root) /usr/bin/osgcluster
%attr(755, root, root) /usr/bin/osgconv
%attr(755, root, root) /usr/bin/osgcopy
%attr(755, root, root) /usr/bin/osgcube
%attr(755, root, root) /usr/bin/osghangglide
%attr(755, root, root) /usr/bin/osgimpostor
%attr(755, root, root) /usr/bin/osgreflect
%attr(755, root, root) /usr/bin/osgscribe
%attr(755, root, root) /usr/bin/osgstereoimage
%attr(755, root, root) /usr/bin/osgtexture
%attr(755, root, root) /usr/bin/osgviews
%attr(755, root, root) /usr/bin/sgv
%attr(755, root, root) /usr/lib/CVS
%attr(755, root, root) /usr/lib/libosgDB.so
%attr(755, root, root) /usr/lib/libosgGLUT.so
%attr(755, root, root) /usr/lib/libosg.so
%attr(755, root, root) /usr/lib/libosgText.so
%attr(755, root, root) /usr/lib/libosgUtil.so
%attr(755, root, root) /usr/lib/osgPlugins
%attr(444, root, root) /usr/include/osg/AlphaFunc
%attr(444, root, root) /usr/include/osg/AnimationPath
%attr(444, root, root) /usr/include/osg/Billboard
%attr(444, root, root) /usr/include/osg/BoundingBox
%attr(444, root, root) /usr/include/osg/BoundingSphere
%attr(444, root, root) /usr/include/osg/BoundsChecking
%attr(444, root, root) /usr/include/osg/Camera
%attr(444, root, root) /usr/include/osg/ClippingVolume
%attr(444, root, root) /usr/include/osg/ClipPlane
%attr(444, root, root) /usr/include/osg/ColorMask
%attr(444, root, root) /usr/include/osg/ColorMatrix
%attr(444, root, root) /usr/include/osg/CopyOp
%attr(444, root, root) /usr/include/osg/CullFace
%attr(444, root, root) /usr/include/osg/CVS
%attr(444, root, root) /usr/include/osg/Depth
%attr(444, root, root) /usr/include/osg/DisplaySettings
%attr(444, root, root) /usr/include/osg/Drawable
%attr(444, root, root) /usr/include/osg/EarthSky
%attr(444, root, root) /usr/include/osg/Export
%attr(444, root, root) /usr/include/osg/Fog
%attr(444, root, root) /usr/include/osg/FrameStamp
%attr(444, root, root) /usr/include/osg/FrontFace
%attr(444, root, root) /usr/include/osg/Geode
%attr(444, root, root) /usr/include/osg/GeoSet
%attr(444, root, root) /usr/include/osg/GL
%attr(444, root, root) /usr/include/osg/GLExtensions
%attr(444, root, root) /usr/include/osg/GLU
%attr(444, root, root) /usr/include/osg/Group
%attr(444, root, root) /usr/include/osg/Image
%attr(444, root, root) /usr/include/osg/Impostor
%attr(444, root, root) /usr/include/osg/ImpostorSprite
%attr(444, root, root) /usr/include/osg/Light
%attr(444, root, root) /usr/include/osg/LightModel
%attr(444, root, root) /usr/include/osg/LightSource
%attr(444, root, root) /usr/include/osg/LineSegment
%attr(444, root, root) /usr/include/osg/LineStipple
%attr(444, root, root) /usr/include/osg/LineWidth
%attr(444, root, root) /usr/include/osg/LOD
%attr(444, root, root) /usr/include/osg/Material
%attr(444, root, root) /usr/include/osg/Math
%attr(444, root, root) /usr/include/osg/Matrix
%attr(444, root, root) /usr/include/osg/MemoryManager
%attr(444, root, root) /usr/include/osg/Node
%attr(444, root, root) /usr/include/osg/NodeCallback
%attr(444, root, root) /usr/include/osg/NodeVisitor
%attr(444, root, root) /usr/include/osg/Notify
%attr(444, root, root) /usr/include/osg/Object
%attr(444, root, root) /usr/include/osg/Plane
%attr(444, root, root) /usr/include/osg/Point
%attr(444, root, root) /usr/include/osg/PolygonMode
%attr(444, root, root) /usr/include/osg/PolygonOffset
%attr(444, root, root) /usr/include/osg/PositionAttitudeTransform
%attr(444, root, root) /usr/include/osg/Projection
%attr(444, root, root) /usr/include/osg/Quat
%attr(444, root, root) /usr/include/osg/Referenced
%attr(444, root, root) /usr/include/osg/ref_ptr
%attr(444, root, root) /usr/include/osg/ShadeModel
%attr(444, root, root) /usr/include/osg/State
%attr(444, root, root) /usr/include/osg/StateAttribute
%attr(444, root, root) /usr/include/osg/StateSet
%attr(444, root, root) /usr/include/osg/Statistics
%attr(444, root, root) /usr/include/osg/Stencil
%attr(444, root, root) /usr/include/osg/Switch
%attr(444, root, root) /usr/include/osg/TexEnv
%attr(444, root, root) /usr/include/osg/TexGen
%attr(444, root, root) /usr/include/osg/TexMat
%attr(444, root, root) /usr/include/osg/Texture
%attr(444, root, root) /usr/include/osg/TextureCubeMap
%attr(444, root, root) /usr/include/osg/Timer
%attr(444, root, root) /usr/include/osg/Transform
%attr(444, root, root) /usr/include/osg/Transparency
%attr(444, root, root) /usr/include/osg/Types
%attr(444, root, root) /usr/include/osg/Vec2
%attr(444, root, root) /usr/include/osg/Vec3
%attr(444, root, root) /usr/include/osg/Vec4
%attr(444, root, root) /usr/include/osg/Version
%attr(444, root, root) /usr/include/osg/Viewport

