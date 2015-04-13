/* OpenSceneGraph example, osgtexture3D.
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

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture2DArray>
#include <osg/TexGen>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>

#include <iostream>

//
// A simple demo demonstrating different texturing modes,
// including using of texture extensions.
//


typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

class SubloadCallback : public osg::Texture2DArray::SubloadCallback
{
  public:
    virtual void load(const osg::Texture2DArray& texture, osg::State& state) const   { }
    virtual void subload(const osg::Texture2DArray& texture, osg::State& state) const   { }
};


osg::StateSet* createState(osg::ArgumentParser& arguments)
{
    // read 4 2d images
    osg::ref_ptr<osg::Image> image_0 = osgDB::readImageFile("Images/lz.rgb");
    osg::ref_ptr<osg::Image> image_1 = osgDB::readImageFile("Images/reflect.rgb");
    osg::ref_ptr<osg::Image> image_2 = osgDB::readImageFile("Images/tank.rgb");
    osg::ref_ptr<osg::Image> image_3 = osgDB::readImageFile("Images/skymap.jpg");

    if (!image_0 || !image_1 || !image_2 || !image_3)
    {
        std::cout << "Warning: could not open files."<<std::endl;
        return new osg::StateSet;
    }

    if (image_0->getPixelFormat()!=image_1->getPixelFormat() || image_0->getPixelFormat()!=image_2->getPixelFormat() || image_0->getPixelFormat()!=image_3->getPixelFormat())
    {
        std::cout << "Warning: image pixel formats not compatible."<<std::endl;
        return new osg::StateSet;
    }

    GLint textureSize = 1024;

    // scale them all to the same size.
    image_0->scaleImage(textureSize,textureSize,1);
    image_1->scaleImage(textureSize,textureSize,1);
    image_2->scaleImage(textureSize,textureSize,1);
    image_3->scaleImage(textureSize,textureSize,1);


    osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
    texture->setFilter(osg::Texture2DArray::MIN_FILTER, osg::Texture2DArray::LINEAR);
    texture->setFilter(osg::Texture2DArray::MAG_FILTER, osg::Texture2DArray::LINEAR);
    texture->setWrap(osg::Texture2DArray::WRAP_R, osg::Texture2DArray::REPEAT);

    if (arguments.read("--mipmap"))
    {
        OSG_NOTICE<<"Enabling Mipmaping"<<std::endl;
        texture->setUseHardwareMipMapGeneration(true);
        texture->setFilter(osg::Texture2DArray::MIN_FILTER, osg::Texture2DArray::LINEAR_MIPMAP_LINEAR);
    }

    if (arguments.read("--subload"))
    {
        texture->setTextureSize(textureSize, textureSize, 1);
        texture->setSubloadCallback(new SubloadCallback());
    }
    else if (arguments.read("--packed"))
    {
        OSG_NOTICE<<"Packing all images into a single osg::Image to pass to Texture2DArray."<<std::endl;

        osg::ref_ptr<osg::Image> image_3d = new osg::Image;
        image_3d->allocateImage(textureSize, textureSize, 4, image_0->getPixelFormat(), image_0->getDataType());

        // copy the 2d images into the 3d image.
        image_3d->copySubImage(0,0,0,image_0.get());
        image_3d->copySubImage(0,0,1,image_1.get());
        image_3d->copySubImage(0,0,2,image_2.get());
        image_3d->copySubImage(0,0,3,image_3.get());

        image_3d->setInternalTextureFormat(image_0->getInternalTextureFormat());

        texture->setImage(0, image_3d.get());
    }
    else
    {
        OSG_NOTICE<<"Assigned all images to Texture2DArray separately."<<std::endl;

        texture->setImage(0, image_0.get());
        texture->setImage(1, image_1.get());
        texture->setImage(2, image_2.get());
        texture->setImage(3, image_3.get());
    }

    std::string vsFileName("shaders/osgtexture2DArray.vert");
    std::string fsFileName("shaders/osgtexture2DArray.frag");

    if (arguments.read("--vs", vsFileName)) {}
    if (arguments.read("--fs", vsFileName)) {}

    osg::ref_ptr<osg::Program> program = new osg::Program;

    osg::ref_ptr<osg::Shader> vertexShader = osgDB::readShaderFile( osg::Shader::VERTEX, vsFileName) ;
    if (vertexShader.get()) program->addShader( vertexShader.get() );

    osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readShaderFile( osg::Shader::FRAGMENT, fsFileName) ;
    if (fragmentShader.get()) program->addShader( fragmentShader.get() );

    // create the StateSet to store the texture data
    osg::StateSet* stateset = new osg::StateSet;
    stateset->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
    stateset->addUniform( new osg::Uniform( "texture", 0 ) );
    stateset->setAttribute( program.get() );
    return stateset;
}

/** create 2,2 square with center at 0,0,0 and aligned along the XZ plan */
osg::Drawable* createSquare()
{
    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0].set(-1.0f,0.0f,1.0f);
    (*coords)[1].set(-1.0f,0.0f,-1.0f);
    (*coords)[2].set(1.0f,0.0f,-1.0f);
    (*coords)[3].set(1.0f,0.0f,1.0f);
    geom->setVertexArray(coords);

    osg::Vec3Array* norms = new osg::Vec3Array(1);
    (*norms)[0].set(0.0f,-1.0f,0.0f);
    geom->setNormalArray(norms, osg::Array::BIND_OVERALL);

    osg::Vec3Array* tcoords = new osg::Vec3Array(4);
    (*tcoords)[0].set(0.0f, 1.0f, 0.0f);
    (*tcoords)[1].set(0.0f, 0.0f, 0.0f);
    (*tcoords)[2].set(1.0f, 0.0f, 0.0f);
    (*tcoords)[3].set(1.0f, 1.0f, 0.0f);
    geom->setTexCoordArray(0,tcoords);

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

    return geom;
}

osg::Node* createModel(osg::ArgumentParser& arguments)
{
    // create the geometry of the model, just a simple 2d quad right now.
    osg::Geode* geode = new osg::Geode;

    geode->addDrawable(createSquare());

    geode->setStateSet(createState(arguments));

    return geode;

}


int main(int argc, char **argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // create a model from the images and pass it to the viewer.
    osg::ref_ptr<osg::Node> model = createModel(arguments);
    if (!model) return 0;

    std::string filename;
    if (arguments.read("-o",filename))
    {
        osg::ref_ptr<osgDB::Options> options = new osgDB::Options;
        options->setOptionString("WriteImageHint=IncludeData");
        osgDB::writeNodeFile(*model, filename, options.get());
        return 0;
    }


    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // assign scene graph to viewer
    viewer.setSceneData(model.get());

    // run viewer
    return viewer.run();
}
