Features and issues with Performer plugin
=========================================

Issues
------
After compiling the Peformer plugin you can run sgv and it will
pick up the library osgPlugins/osgdb_pfb correctly, so you'll now be
able to type something like :

  cd /usr/share/Performer/data
  sgv iris.pfb 

Unfortunately this then results in the following error message :

  DynamicLibrary::failed loading /home/robert/OpenSceneGraph-0.8/lib/osgPlugins/osgdb_pfb.so
  DynamicLibrary::error /usr/lib/libpr.so: undefined symbol: __ucmpdi2

An attempt to fixing the undefined symbol by including -lgcc did not fix the
problem, have a look at src/osgPlugins/pfb/Makefile, for the link lines
tested. Suggestions welcome.

This error disappears if you link sgv with Performer, you can do this by
simply swapping the #comment around in src/Viewier/Makefile so that :

  LIBS =  -losgUtil -losg  -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi
  #LIBS = ${PFLIBS} -losgUtil -losg  -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi

is edited to become:

  #LIBS =  -losgUtil -losg  -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi
  LIBS = ${PFLIBS} -losgUtil -losg  -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi

Not an ideal solution but it does work.  Now try :

  cd /usr/share/Performer/data
  sgv town_ogl_pfi.pfb


Features
--------
You can also use osgdb_pfb.so as a Performer plugin, by linking/copying the
osgPlugins/osgdb_pfb.so to libpfosg.so.   Try something like :

  cd OpenSceneGraph-0.8/lib
  ln -s osgPlugins/osgdb_pfb.so libpfosg.so

then 

  perfly turtle.osg

or

  pfconv /usr/share/Performer/data/iris.pfb iris.osg
  sgv iris.osg

