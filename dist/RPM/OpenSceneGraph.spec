
Summary: A C++ scene graph API on OpenGL for real time graphics
Name: OpenSceneGraph
Version: 0.8.44
Release: 1
Copyright: LGPL
Group: Graphics
Source: OpenSceneGraph-0.8.44.tar.gz
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

cd /usr/src/redhat/BUILD/OpenSceneGraph-0.8.44 
tar cvf - . | tar xvfC - /

# ---------------------
# FILES Sections
%files

%attr(755, root, root) /usr/local/lib/libosgDB.so
%attr(755, root, root) /usr/local/lib/libosgGLUT.so
%attr(755, root, root) /usr/local/lib/libosg.so
%attr(755, root, root) /usr/local/lib/libosgText.so
%attr(755, root, root) /usr/local/lib/libosgUtil.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_3ds.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_bmp.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_dw.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_dx.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_flt.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_gif.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_jpeg.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_lwo.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_obj.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_osg.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_osgtgz.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_pfb.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_pic.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_png.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_rgb.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_tga.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_tgz.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_tiff.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_txp.so
%attr(755, root, root) /usr/local/lib/osgPlugins/osgdb_zip.so
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgbillboard
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgcluster
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgconv
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgcopy
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgcube
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osghangglide
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osghud
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgimpostor
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgreflect
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgscribe
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgstereoimage
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgtext
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgtexture
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgversion
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/osgviews
%attr(755, root, root) /usr/share/OpenSceneGraph/bin/sgv
