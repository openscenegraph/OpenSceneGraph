
Source Engine MDL reader for OSG

by Jason Daly

Overview
--------
This plugin allows .mdl files from games that make use of Valve's Source
engine (Half Life 2, etc) to be loaded by OSG.

I've tested this plugin with dozens of HL2 models, as well as some 3rd party
models.


Using the Plugin
----------------
If you want to load models from the original Source engine games, you'll need
to extract the relevant models and materials from the .gcf files that come
with the game.  A good tool for this is GCFScape, available at
http://www.nemesis.thewavelength.net/index.php?p=26

The plugin expects the models and materials to be arranged as they are in the
.gcf files (models in models/  materials and textures in materials/).  Only the
models/ and materials/ directories are used by this plugin.  Note that the
.mdl plugin typically will need to read additional data from companion .vvd
and .vtx files, as well as textures from other files in the materials/
directory.

It is important to preserve the file and directory structure as it is in the
.gcf files, although you can merge the data from several files (different
games) together.  For example, you can extract the models/ directory from
source models.gcf, the materials/ directory from source materials.gcf, and the
maps/, models/, and materials/ directories from half-life 2 deathmatch.gcf and
combine them together into a single parent directory (called "hl2data/", for
example).  This arrangement will let you load any of the HL2 deathmatch maps,
with all of the props and materials that might be needed.

If you're confused, here's a lame ASCII art drawing to confuse you even more:

hl2data
 |
 +----maps
 |
 +----materials
 |
 +----models


If you want to use the OSGFILEPATH environment variable to let OSG search for
maps or models, point it to the parent directory ("hl2data" in the example
above), then load your model like this:

   osgviewer models/alyx.mdl


What Works
----------
All geometry and textures.  

Some models have multiple sub-models in them (for example, a door model might
have multiple variations of handles).  These are supported with a switch,
but you'll have to find it and manipulate it yourself (I can't figure out
how the Source engine selects which sub-model to show).


What Doesn't Work (yet)
-----------------------
Skeletons and bone animations.  My guess is that this would be possible using
osgAnimation, but I didn't have time to do it for the first cut.

Facial animations.  Would require support for vertex morphing (I don't know
if osgAnimation provides this yet or not).

Physics.  There is no OSG infrastructure yet, and I can't find much information
on the .phy file format that models use for physics calculations.  Rag-doll
physics obviously doesn't work either.

Eyes.  This is one of the more ugly artifacts you'll see if you load a
character model.  Only the whites of the eyes are drawn (no iris or pupil).
The eyes (as well as the teeth) use special shaders that I didn't have time to
figure out.  This is ugly enough that I might come back to it soon.


Acknowledgements
----------------
This plugin was written for some real-world work I'm doing at the University
of Central Florida Institute for Simulation and Training.  I want to thank
our sponsors for funding our work and allowing me to contribute this code to
the OSG community.

Portions of the code borrow heavily from the Source SDK.  Most of the
file format reading came from the header files for studio models, so thanks to
Valve for making much of that code public.

Of course, this code would be pointless without the Open Scene Graph and
all of its contributors.

