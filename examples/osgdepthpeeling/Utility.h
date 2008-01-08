/*
  Steffen Frey
  Fachpraktikum Graphik-Programmierung 2007
  Institut fuer Visualisierung und Interaktive Systeme
  Universitaet Stuttgart
 */


#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <string>
#include <sstream>
#include <osg/Program>
#include <osg/Texture2D>

namespace Utility
{
/*!
  Reads a file and returns a string
 */
bool readFile(const char* fName, std::string& s);

/*!
  Converts a number to a string
 */
std::string toString(double d);

/*!
  Create a osg shader program consisting of a vertex shader and a 
  fragment shader
 */
osg::Program* createProgram(std::string vs, std::string fs);

/*!
This is a random generator to generate noise patterns.
The returned values range from -1 to 1
*/
double getNoise(unsigned x, unsigned y, unsigned random);

/*!
  Returns a smoothed noise version of the value that is read from the noise
  texture
 */
double smoothNoise(unsigned width, unsigned height, unsigned x, unsigned y, unsigned char* noise);

/*!
  Creates a two dimensional color texture and apply some standard settings
 */
 osg::Texture2D* newColorTexture2D(unsigned width, unsigned height, unsigned accuracy);

/*!
  Get a quad with screen size in order to show a texture full screen
 */
osg::Geode* getCanvasQuad(unsigned width, unsigned height, double depth=-1);
}
#endif
