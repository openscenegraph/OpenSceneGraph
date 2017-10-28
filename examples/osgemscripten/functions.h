/* OpenSceneGraph example, osgemscripten.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#ifndef OSGEMSCRIPTEN_FUNCTIONS_H
#define OSGEMSCRIPTEN_FUNCTIONS_H

#include <osg/Notify>

// Convert NotifySeverity enum value to string.
std::string logLevelToString(osg::NotifySeverity severity)
{
    switch (severity)
    {
        case osg::DEBUG_FP:
            // Verbose.
            return "V";
        case osg::DEBUG_INFO:
            // Debug.
            return "D";
        case osg::NOTICE:
        case osg::INFO:
            // Info.
            return "I";
        case osg::WARN:
            // Warning.
            return "W";
        case osg::FATAL:
        case osg::ALWAYS:
            // Error.
            return "E";
    }
}

#include <osg/Program>

// Fragment shader to display everything in red colour.
static const char shaderFragment[] =
    "void main() {                             \n"
    "  gl_FragColor = vec4(0.5, 0.3, 0.3, 1.0);\n"
    "}                                         \n";
// Geometry shader to pass geometry vertices to fragment shader.
static const char shaderVertex[] =
    "void main() {                                              \n"
    "  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;  \n"
    "}                                                          \n";

osg::Program *createShaderProgram(
    const std::string &vertexShader,
    const std::string &fragmentShader)
{
    // Load shaders.
    osg::Shader *vs = new osg::Shader(osg::Shader::VERTEX, vertexShader);
    osg::Shader *fs = new osg::Shader(osg::Shader::FRAGMENT, fragmentShader);
    // Compile shaders and compose shader program.
    osg::Program *prog = new osg::Program;
    prog->addShader(vs);
    prog->addShader(fs);
    return prog;
}

#endif // OSGEMSCRIPTEN_FUNCTIONS_H
