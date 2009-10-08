/* OpenSceneGraph example, osgvertexattributes.
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

#include <osgUtil/ShaderGen>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>

class ConvertToVertexAttibArrays : public osg::NodeVisitor
{
    public:

        typedef std::pair<unsigned int, std::string> AttributeAlias;

        ConvertToVertexAttibArrays():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
            // mappings taken from http://www.opengl.org/registry/specs/NV/vertex_program.txt
            _vertexAlias = AttributeAlias(0, "osg_Vertex");
            _normalAlias = AttributeAlias(2, "osg_Normal");
            _colorAlias = AttributeAlias(3, "osg_Color");
            _secondaryColorAlias = AttributeAlias(4, "osg_SecondaryColor");
            _fogCoordAlias = AttributeAlias(5, "osg_FogCoord");
            _texCoordAlias[0] = AttributeAlias(8, "osg_MultiTexCoord0");
            _texCoordAlias[1] = AttributeAlias(9, "osg_MultiTexCoord1");
            _texCoordAlias[2] = AttributeAlias(10, "osg_MultiTexCoord2");
            _texCoordAlias[3] = AttributeAlias(11, "osg_MultiTexCoord3");
            _texCoordAlias[4] = AttributeAlias(12, "osg_MultiTexCoord4");
            _texCoordAlias[5] = AttributeAlias(13, "osg_MultiTexCoord5");
            _texCoordAlias[6] = AttributeAlias(14, "osg_MultiTexCoord6");
            _texCoordAlias[7] = AttributeAlias(15, "osg_MultiTexCoord7");
        }


        void apply(osg::Geode& geode)
        {
            for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
            {
                osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
                if (geom) apply(*geom);
            }
        }

        void apply(osg::Geometry& geom)
        {
            osg::notify(osg::NOTICE)<<"Found geometry "<<&geom<<std::endl;
            if (geom.getVertexArray())
            {
                setVertexAttrib(geom, _vertexAlias, geom.getVertexArray(), false, osg::Geometry::BIND_PER_VERTEX);
                geom.setVertexArray(0);
            }

            if (geom.getNormalArray())
            {
                setVertexAttrib(geom, _normalAlias, geom.getNormalArray(), true, geom.getNormalBinding());
                geom.setNormalArray(0);
            }

            if (geom.getColorArray())
            {
                setVertexAttrib(geom, _colorAlias, geom.getColorArray(), true, geom.getColorBinding());
                geom.setColorArray(0);
            }

            if (geom.getSecondaryColorArray())
            {
                setVertexAttrib(geom, _secondaryColorAlias, geom.getSecondaryColorArray(), true, geom.getSecondaryColorBinding());
                geom.setSecondaryColorArray(0);
            }

            if (geom.getFogCoordArray())
            {
                // should we normalize the FogCoord array? Don't think so...
                setVertexAttrib(geom, _fogCoordAlias, geom.getFogCoordArray(), false, geom.getSecondaryColorBinding());
                geom.setFogCoordArray(0);
            }

            unsigned int maxNumTexCoords = geom.getNumTexCoordArrays();
            if (maxNumTexCoords>8)
            {
                osg::notify(osg::NOTICE)<<"Warning: Ignoring "<<maxNumTexCoords-8<<" texture coordinate arrays, only 8 are currently supported in vertex attribute conversion code."<<std::endl;
                maxNumTexCoords = 8;
            }
            for(unsigned int i=0; i<maxNumTexCoords; ++i)
            {
                if (geom.getTexCoordArray(i))
                {
                    setVertexAttrib(geom, _texCoordAlias[i], geom.getTexCoordArray(i), true, geom.getSecondaryColorBinding());
                    geom.setTexCoordArray(i,0);
                }
                else
                {
                    osg::notify(osg::NOTICE)<<"Found empty TexCoordArray("<<i<<")"<<std::endl;
                }
            }
        }

        void setVertexAttrib(osg::Geometry& geom, const AttributeAlias& alias, osg::Array* array, bool normalize, osg::Geometry::AttributeBinding binding)
        {
            unsigned int index = alias.first;
            const std::string& name = alias.second;
            array->setName(name);
            geom.setVertexAttribArray(index, array);
            geom.setVertexAttribNormalize(index, normalize);
            geom.setVertexAttribBinding(index, binding);

            osg::notify(osg::NOTICE)<<"   vertex attrib("<<name<<", index="<<index<<", normalize="<<normalize<<" binding="<<binding<<")"<<std::endl;
        }

        AttributeAlias _vertexAlias;
        AttributeAlias _normalAlias;
        AttributeAlias _colorAlias;
        AttributeAlias _secondaryColorAlias;
        AttributeAlias _fogCoordAlias;
        AttributeAlias _texCoordAlias[8];
};



int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    std::string outputFileName;
    while (arguments.read("-o",outputFileName)) {}

    bool runShaderGen = true;
    while (arguments.read("--shader-gen")) { runShaderGen = true; }
    while (arguments.read("--no-shader-gen")) { runShaderGen = false; }

    bool runConvertToVertexAttributes = true;
    while (arguments.read("--vertex-attrib")) { runConvertToVertexAttributes = true; }
    while (arguments.read("--no-vertex-attrib")) { runConvertToVertexAttributes = false; }

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel.get())
    {
        osg::notify(osg::NOTICE)<<"No model loaded, please specify a model filename."<<std::endl;
        return 1;
    }

    if (runShaderGen)
    {
        // convert fixed function pipeline to shaders
        osgUtil::ShaderGenVisitor sgv;
        loadedModel->accept(sgv);
    }

    if (runConvertToVertexAttributes)
    {
        // find any conventional vertex, colour, normal and tex coords arrays and convert to vertex attributes
        ConvertToVertexAttibArrays ctvaa;
        loadedModel->accept(ctvaa);
    }

    if (!outputFileName.empty())
    {
        osgDB::writeNodeFile(*loadedModel, outputFileName);
        return 0;
    }

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(loadedModel.get());

    return viewer.run();
}
