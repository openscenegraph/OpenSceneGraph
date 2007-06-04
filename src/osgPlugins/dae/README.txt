osgPlugins/dae/README.txt -  Mike Weiblen http://mew.cx/

OSG reader/writer plugin for the COLLADA digital asset exchange (DAE) schema.
See http://collada.org/ and http://khronos.org/collada/ for further info.


RUNTIME USAGE EXAMPLES

    osgviewer myFile.dae
    osgconv myFile.osg myFile.dae


RUNTIME PLUGIN OPTIONS

Import Options
--------------
none

Export Options
--------------
polygon : export polygons as COLLADA polygons instead of polylists.
          This option can be used for if polylists are not supported.
          ex : osgconv -O polygon myFile.osg myFile.dae


BUILD DEPENDENCIES

- COLLADA DOM (document object model) v1.4.1
  http://collada.org/mediawiki/index.php/DOM_guide:_Setting_up

- libxml2

- iconv

A standard "ccmake ." while in the root of the OSG source will expose
two CMake variables: COLLADA_INCLUDE_DIR and COLLADA_LIBRARY. Using
these two variables correctly can be a bit tricky (especially as
the "right" and "wrong" ways to use Collada are hard to define), but
I will briefly explain them below.

  COLLADA_INCLUDE_DIR: This variable wants the path where it can
  find the COLLADA headers. On most systems this will be something
  like /usr/include or /usr/include/collada.

  COLLADA_LIBRARY: This variable is asking for the FULL PATH to the
  file libcollada_dom.a. As long as it can find this file, the
  OSG Collada ReaderWriter should build just fine.
