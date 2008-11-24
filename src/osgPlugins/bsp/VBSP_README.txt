
Source Engine BSP map reader for OSG

by Jason Daly

Overview
--------
This plugin allows .bsp files from games that make use of Valve's Source
engine (Half Life 2, etc) to be loaded by OSG.

I've tested this plugin on several HL2 deathmatch maps, as well as some 3rd
party maps.


Using the Plugin
----------------
Unless you've got a 3rd party map that doesn't make use of any data from the
original Source engine games, you'll need to extract the relevant maps and
materials from the .gcf files that come with the game.  A good tool for this
is GCFScape, available at http://www.nemesis.thewavelength.net/index.php?p=26

The plugin expects the maps and materials to be arranged as they are in the
.gcf file (maps in maps/  materials and textures in materials/).  Only the
maps/ and materials/ directories are used by this plugin (models aren't loaded
yet, see below).  

Make sure to preserve the file and directory structure as it is in the .gcf
files.  The standard osgDB search routines don't work, because the .bsp
file and .vmt files often mix case when referring to materials, textures, etc.
The regular osgDB search routines can't do a fully case-insensitive search
for a file, so an explicit, special-purpose search is performed by the plugin
instead.


What Works
----------
All brush geometry and faces.

Displacement surfaces.

Textures (including blended textures on displacement surfaces).  This makes
use of the VTF Reader plugin which was submitted at the same time as this
plugin.


What Doesn't Work (yet)
-----------------------
Light maps.  This requires reading the light map information lumps from the
.bsp file, and creating a shader to render them on the various surfaces.
Shouldn't be too hard.  Currently, the plugin just treats lightmapped surfaces
the same as vertex lit surfaces.

Environment maps and shiny textures.  Both of these require reading cube maps.
Cube maps and environment mapping materials are typically stored inside the
.bsp file as an embedded .pak file (essentially a .zip file with no
compression.  Reading them would require parsing the .pak file and reading the
embedded .vmt to find the pointers to the base texture and cube map.  Then the
cube map must be read from the embedded .pak file as well.  Finally, a shader
would be created to render them.  Currently, the plugin cheats by guessing
the base texture from the embedded material name, and loading it.  This
seems to work well enough for now.

Water.  Water shaders in the Source engine are a lot more complicated than
the generic lightmapped and vertex lit shaders.  Currently, water surfaces just
show up solid white.

Props (both static and physics).  This is a big one, as props often contribute
to the overall flow and feeling of a map.  Unfortunately, this requires yet
another plugin to load the .mdl/.vvd/.vtx files that store Source models.
These formats aren't nearly as well documented as the .bsp format, which is the
main reason they're not done yet.

World lights (light sources).  Should be simple to read, but I don't know
how many you'll find on a given map.  If there are a lot of them, you'll
have to be creative on how they're rendered.

HDR materials, bump maps, detail props, and other eye-candy.  Implement it if
you want!  :-)


Acknowledgements
----------------
This plugin was written for some real-world work I'm doing at the University
of Central Florida Institute for Simulation and Training.  I want to thank
our sponsors for funding our work and allowing me to contribute this code to
the OSG community.

This plugin wouldn't have been possible without Rof's "The Source Engine BSP
Format" at http://www.geocities.com/cofrdrbob/bspformat.html  which itself
is based on Max McGuire's "Quake 2 BSP File Format" at
http://www.flipcode.com/archives/Quake_2_BSP_File_Format.shtml

Portions of the code borrow heavily from the Source SDK (especially the
code for the displacement surfaces), so thanks to Valve for making much of
that code public.

Of course, this code would be superfluous without the Open Scene Graph and
all of its contributors.

