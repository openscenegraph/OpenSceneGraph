#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <iostream>
#include <stdio.h>


using namespace osg;

class ReaderWriter3DC : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "3DC point cloud reader"; }
        
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"3dc") ||
                   osgDB::equalCaseInsensitive(extension,"asc");
        }

        virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
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
                    std::cout <<"Comment: "<<line<<std::endl;
                }
                else
                {
                    ++num;
                }
                //std::cout << "["<<std::endl;
                //std::cout <<line<<std::endl;
                //std::cout <<"]"<<std::endl;
            }
            
            
            std::cout << "num="<<num<<std::endl;
    
            osg::Geometry* geometry = new osg::Geometry;
            
            osg::Vec3Array* vertices = new osg::Vec3Array;
            osg::Vec3Array* normals = new osg::Vec3Array;
            osg::UByte4Array* colours = new osg::UByte4Array;
            //osg::Vec4Array* colours = new osg::Vec4Array;
            
            vertices->reserve(num);
            normals->reserve(num);
            colours->reserve(num);
            
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
                        vertices->push_back(pos);
                        normals->push_back(normal);
                        colours->push_back(osg::UByte4(r,g,b,255));
                        //colours->push_back(osg::Vec4((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,1.0f));
                        
                    }
                }

                
                //std::cout << "["<<std::endl;
                //std::cout <<line<<std::endl;
                //std::cout <<"]"<<std::endl;
            }

            geometry->setUseDisplayList(false);    
            geometry->setVertexArray(vertices);
            geometry->setNormalArray(normals);
            geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            geometry->setColorArray(colours);
            geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geometry);
    
            return geode;
    
        }
    
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriter3DC> g_readerWriter_3DX_Proxy;
