#include "osg/GL"
#include "osg/ExtensionSupported"

#include <stdlib.h>
#include <string.h>

// copied form glut_ext.c
// to be rewritten... Robert Osfield, November 2000.
bool osg::ExtensionSupported(const char *extension)
{
  static const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;

  if (!extensions) {
    extensions = glGetString(GL_EXTENSIONS);
  }
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string.  Don't be fooled by sub-strings,
     etc. */
  start = extensions;
  for (;;) {
    /* If your application crashes in the strstr routine below,
       you are probably calling glutExtensionSupported without
       having a current window.  Calling glGetString without
       a current OpenGL context has unpredictable results.
       Please fix your program. */
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ') {
      if (*terminator == ' ' || *terminator == '\0') {
        return true;
      }
    }
    start = terminator;
  }
  return false;
}
