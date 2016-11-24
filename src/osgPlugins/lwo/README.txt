LIGHTWAVE (LWO2) PLUGIN INTRODUCTION
------------------------------------------------------------------------------

  This is the plugin version of my LWO2->OSG converter. It has all the
features (and drawbacks) of the original converter but it doesn't replace
it completely.
  I'm planning to further enhance the stand-alone converter by adding osgNV 
compatibility and I can't do this on the plugin version because I'm not going
to introduce a dependency to osgNV into OSG.
  There is also a LWS plugin which reads Lightwave scene files; make sure you
only read scene files that point to LWO2 (not LWO1) objects, otherwise you may
experience problems (the LWO1 plugin doesn't convert the coordinate system
correctly).


PLUGIN OPTIONS
------------------------------------------------------------------------------

   USE_OLD_READER           pass control to the old LWO/LWO2 reader plugin
                            (all other options ignored)

   FORCE_ARB_COMPRESSION    create compressed textures
   
   USE_OSGFX                use osgFX effects to improve visual appearance
   
   NO_LIGHTMODEL_ATTRIBUTE  prevent the plugin from creating a LightModel
                            state attribute; using this option may result in
                            visual artifacts

   MAX_TEXTURE_UNITS <n>    set the maximum number of texture units to be
                            used when creating multi-textured surfaces


NOTES
------------------------------------------------------------------------------

  NOTE_1: this plugin works fine in reading LWO2 files but it's not well 
optimized,so you can expect slowness and large memory usage.

  NOTE_2: the LWS (scene) support is a quick-and-dirty work, it's there 
only because I needed it. Do not pretend too much from it, at least until
I improve it somehow.

  NOTE_3: the osgFX support is still limited, only osgFX::SpecularHighlights
is used to improve the specularity effects on materials that have a specular
component. Future enhancements will allow osgFX to be used more widely to 
give a better visual matching between the original LWO2 model and OSG.


  Marco Jez <marco.jez@poste.it>
  January 2004
