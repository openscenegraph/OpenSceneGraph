#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <iostream>
#include <stdio.h>


using namespace osg;

class ReaderWriter3DC : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "3DC point cloud reader"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"3dc") ||
                   osgDB::equalCaseInsensitive(extension,"asc");
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            
            std::cout << "try to read file "<<fileName<<std::endl;
    
            const int LINE_SIZE = 1024;
            char line[LINE_SIZE];
            
            std::ifstream fin(fileName.c_str());
            
            unsigned int num = 0;
            while (fin)
            {
                fin.getline(line,LINE_SIZE);
                if (line[0]=='#')
                {
                    // comment line
                    osg::notify(osg::INFO) <<"Comment: "<<line<<std::endl;
                }
                else
                {
                    ++num;
                }
            }
            
            
            std::cout << "num="<<num<<std::endl;
            
            unsigned int targetNumVertices = 10000;
           
    
            osg::Geode* geode = new osg::Geode;

            osg::Geometry* geometry = new osg::Geometry;
            
            osg::Vec3Array* vertices = new osg::Vec3Array;
            osg::Vec3Array* normals = new osg::Vec3Array;
            osg::Vec4ubArray* colours = new osg::Vec4ubArray;
            
            vertices->reserve(targetNumVertices);
            normals->reserve(targetNumVertices);
            colours->reserve(targetNumVertices);
            
            fin.close();
            
            std::ifstream fin2(fileName.c_str());
            while (fin2)
            {
                fin2.getline(line,LINE_SIZE);
                if (line[0]=='#')
                {
                    // comment line
                    //std::cout <<"Comment: "<<line<<std::endl;
                }
                else if (strlen(line)>0)
                {
                    ++num;
                    
                    osg::Vec3 pos,normal;
                    int r,g,b;
                    
                    int a = sscanf(line,"%f %f %f %d %d %d %f %f %f",
                                   &pos.x(),&pos.y(),&pos.z(),
                                   &r,&g,&b,
                                   &normal.x(),&normal.y(),&normal.z());
                                
                                
                    if (a)
                    {
                    
                        if (vertices->size()>=targetNumVertices)
                        {
                            // finishing setting up the current geometry and add it to the geode.
                            geometry->setUseDisplayList(true);    
                            geometry->setUseVertexBufferObjects(true);    
                            geometry->setVertexArray(vertices);
                            geometry->setNormalArray(normals);
                            geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                            geometry->setColorArray(colours);
                            geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
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
                        colours->push_back(osg::Vec4ub(r,g,b,255));
                        
                    }
                }
                
            }


            geometry->setUseDisplayList(true);
            geometry->setUseVertexBufferObjects(true);    
            geometry->setVertexArray(vertices);
            geometry->setNormalArray(normals);
            geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            geometry->setColorArray(colours);
            geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

            geode->addDrawable(geometry);
    
            return geode;
    
        }
    
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriter3DC> g_readerWriter_3DX_Proxy;
