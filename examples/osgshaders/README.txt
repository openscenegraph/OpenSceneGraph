osgshaders/README.txt - mike.weiblen@3dlabs.com - 2003-09-30
Copyright 2003 3Dlabs Inc.

osgshaders is a simple demo of the OpenGL Shading Language, as supported in the
Open Scene Graph using the "osgGL2" library.

Demonstrated are:
- loading, compiling and linking OpenGL Shading Language (OGLSL) shaders.
- multicontext support (use different Producer configurations to create
  multiple windows)
- "uniform" variable support: enables an application to set parameters for
  shaders at runtime for animation, etc.
- multi-texture support: using textures both as applied imagery and as lookup
  tables for controlling shader behavior.

The two screen grabs show the demo at the extremes of a periodic animation
parameter.


WHAT's ON THE SCREEN?

1st row: the Marble shader, consisting of white marble with green veins.
The veins are animated to slosh back and forth, and look somewhat like
bands of stormclouds on Jupiter.

2nd row: the Eroded shader, which demonstrates the OGLSL "discard" command.
The animation erodes the surface of the objects based on a noise texture,
for an effect similar to rusting.

3rd row: the Blocky shader, which demonstrates animated scaling in the vertex
shader, as well as color animation using uniform variables set by the app.

4th row: the Microshader, an extremely simple shader which colors fragments
based on their position.  It's small codesize makes it convenient to hardcode
directly into the application source.


RUNTIME KEY COMMANDS

These key commands are included in the onscreen help (as displayed by pressing
the "h" key):

x - Reload and recompile shader source code from their files.
    You may edit the shader source code file, and recompile the shaders
    while the application runs.

y - toggle shader enable.

#EOF

