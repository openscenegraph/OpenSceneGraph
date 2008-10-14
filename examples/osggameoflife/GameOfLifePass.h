/* OpenSceneGraph example, osggameoflife.
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

#ifndef GAMEOFLIFEPASS_H
#define GAMEOFLIFEPASS_H 1

#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

class ProcessPass {
  public:
    ProcessPass(osg::TextureRectangle *in_tex,
                osg::TextureRectangle *out_tex,
                int width, int height);
    ~ProcessPass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture() { return _OutTexture; }
    void setShader(std::string filename);
    
  private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void setupCamera();

    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InTexture;
    osg::ref_ptr<osg::TextureRectangle> _OutTexture;
    int _TextureWidth;
    int _TextureHeight;
    osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;
};

class GameOfLifePass {
  public:
    GameOfLifePass(osg::Image *in_image);
    ~GameOfLifePass();
    osg::ref_ptr<osg::Group> getRoot() { return _RootGroup; }
    osg::ref_ptr<osg::TextureRectangle> getOutputTexture();
    void setShader(std::string filename);
    // Switch branches so we flip textures
    void flip();

  private:
    osg::ref_ptr<osg::Group> createTexturedQuad();
    void setupCamera();
    void createOutputTextures();
    void activateBranch();
    
    osg::ref_ptr<osg::Group> _RootGroup;
    osg::ref_ptr<osg::Camera> _Camera;
    osg::ref_ptr<osg::TextureRectangle> _InOutTextureLife[2];
    int _TextureWidth;
    int _TextureHeight;
    int _ActiveBranch;
    osg::ref_ptr<osg::Program> _FragmentProgram;
    osg::ref_ptr<osg::StateSet> _StateSet;
    osg::ref_ptr<osg::Switch> _BranchSwith[2];
    ProcessPass *_ProcessPass[2];
};

#endif //GAMEOFLIFEPASS_H
