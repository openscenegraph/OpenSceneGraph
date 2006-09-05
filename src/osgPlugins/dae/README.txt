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
  see http://sourceforge.net/projects/collada-dom
  svn export -r 34 https://svn.sourceforge.net/svnroot/collada-dom/trunk

- libxml2

- iconv

UNIX BUILD

set the env vars:

  COLLADA_INSTALLED = yes
  COLLADA_DAE_HOME = root directory of COLLADA DOM
   
  And if you've compiled the debug version of the COLLADA DOM then define:
  
  COLLADA_DEBUG_LIBS = yes
  
  Note, Collada svn trunk currently defaults to debug build.

The above env vars can also be setup in your own custom Make/depdendencies file (copy this and point
to the locally modified copy using the env var OSG_DEPENDCIES so the the OSG's build system can find
it.

//EOF
