#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>

#include <iostream>
#include <stdio.h>
#include <string.h>

class Writer3DCNodeVisitor: public osg::NodeVisitor {

    public:
        Writer3DCNodeVisitor(std::ostream& fout) :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _fout(fout)
        {
           // _fout << "# file written by OpenSceneGraph" << std::endl << std::endl;
        }

        virtual void apply(osg::Geode &node);

    protected:

        Writer3DCNodeVisitor& operator = (const Writer3DCNodeVisitor&) { return *this; }
        std::ostream& _fout;

};

void Writer3DCNodeVisitor::apply( osg::Geode &node )
{
    osg::Matrix matrix = osg::computeLocalToWorld(getNodePath());

    unsigned int count = node.getNumDrawables();
    for ( unsigned int i = 0; i < count; i++ )
    {
        osg::Geometry *geometry = node.getDrawable( i )->asGeometry();
        if ( geometry )
        {
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
            osg::Vec3Array* colours = dynamic_cast<osg::Vec3Array*>(geometry->getColorArray());

            if ( vertices ) {
                for (unsigned int ii=0;ii<vertices->size();ii++) {

                    // update nodes with world coords
                    osg::Vec3d v = vertices->at(ii) * matrix;
                    _fout << v[0] << ' ' << v[1] << ' ' << v[2];

                    if ( colours )
                    {
                        v=colours->at(ii);
                        _fout << ' ' << (int)v[0]*255.0 << ' ' << (int)v[1]*255.0 << ' ' << (int)v[2]*255.0;
                    }
                    else
                    {
                        _fout << " 255 255 255";
                    }

                    if ( normals )
                    {
                        v = normals->at(ii);
                        _fout << ' ' << v[0] << ' ' << v[1] << ' ' << v[2];
                    }
                    else
                    {
                        _fout << " 0.0 0.0 1.0";
                    }


                    _fout << std::endl;
                }
            }

        }
    }
}

class ReaderWriter3DC : public osgDB::ReaderWriter
{
    public:

        ReaderWriter3DC()
        {
            supportsExtension("3dc","3DC point cloud format");
            supportsExtension("asc","3DC point cloud format");
        }

        virtual const char* className() const { return "3DC point cloud reader"; }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            OSG_INFO << "Reading file "<<fileName<<std::endl;

            const int LINE_SIZE = 1024;
            char line[LINE_SIZE];

            unsigned int targetNumVertices = 10000;

            osg::Geode* geode = new osg::Geode;

            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            osg::Vec3Array* normals = new osg::Vec3Array;
            osg::Vec4ubArray* colours = new osg::Vec4ubArray;

            osg::Vec3 pos;
            osg::Vec3 normal(0.0,0.0,1.0);
            int r=255,g=255,b=255,a=255;
            char sep;

            osgDB::ifstream fin(fileName.c_str());
            while (fin)
            {
                fin.getline(line,LINE_SIZE);
                if (line[0]=='#')
                {
                    // comment line
                    OSG_INFO <<"Comment: "<<line<<std::endl;
                }
                else if (strlen(line)>0)
                {
                    int matched = sscanf(line,"%f%c%f%c%f%c%d%c%d%c%d%c%f%c%f%c%f",
                                   &pos.x(),&sep,&pos.y(),&sep,&pos.z(),&sep,
                                   &r,&sep,&g,&sep,&b,&sep,
                                   &normal.x(),&sep,&normal.y(),&sep,&normal.z());

                    if (matched)
                    {

                        if (vertices->size()>=targetNumVertices)
                        {
                            // finishing setting up the current geometry and add it to the geode.
                            geometry->setUseDisplayList(true);
                            geometry->setUseVertexBufferObjects(true);
                            geometry->setVertexArray(vertices);
                            geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
                            geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);
                            geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

                            geode->addDrawable(geometry);

                            // allocate a new geometry
                            geometry = new osg::Geometry;

                            vertices = new osg::Vec3Array;
                            normals = new osg::Vec3Array;
                            colours = new osg::Vec4ubArray;

                            vertices->reserve(targetNumVertices);
                            normals->reserve(targetNumVertices);
                            colours->reserve(targetNumVertices);

                        }

                        vertices->push_back(pos);
                        normals->push_back(normal);
                        colours->push_back(osg::Vec4ub(r,g,b,a));
                    }
                }
            }


            geometry->setUseDisplayList(true);
            geometry->setUseVertexBufferObjects(true);
            geometry->setVertexArray(vertices);
            geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
            geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);
            geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

            geode->addDrawable(geometry);

            return geode;

        }

        virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName,const Options* options =NULL) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if( !acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream f(fileName.c_str());

            Writer3DCNodeVisitor nv(f);

            // we must cast away constness
            (const_cast<osg::Node*>(&node))->accept(nv);

            return WriteResult::FILE_SAVED;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(3dc, ReaderWriter3DC)
