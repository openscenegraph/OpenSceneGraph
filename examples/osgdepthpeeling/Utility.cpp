/*
  Steffen Frey
  Fachpraktikum Graphik-Programmierung 2007
  Institut fuer Visualisierung und Interaktive Systeme
  Universitaet Stuttgart
 */

#include "Utility.h"

#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <osg/Geometry>
#include <osg/Geode>
#include <osgDB/FileUtils>
#include <osgDB/fstream>

bool Utility::readFile(const char* fName, std::string& s)
{
  std::string foundFile = osgDB::findDataFile(fName);
  if (foundFile.empty()) return false;
  
  osgDB::ifstream is;//(fName);
  is.open(foundFile.c_str());
  if (is.fail())
    {
      std::cerr << "Could not open " << fName << " for reading.\n";
      return false;
    }
  char ch = is.get();
  while (!is.eof())
    {
      s += ch;
      ch = is.get();
    }
  is.close();
  return true;
}

std::string Utility::toString(double d)
{
  std::stringstream ostr;
  ostr << d;
  return ostr.str();
}

osg::Program* Utility::createProgram(std::string vs, std::string fs)
{
  //setup shader
  std::string vertSource;
  if(!readFile((char*)vs.c_str(), vertSource))
    {
      printf("shader source not found\n");
      return false;
    }
  
  std::string fragSource;
  if(!readFile((char*)fs.c_str(), fragSource))
    {
      printf("shader source not found\n");
      return false;
    }

  
  osg::Program* program = new osg::Program;
  program->addShader( new osg::Shader( osg::Shader::VERTEX, vertSource.c_str() ) );
  program->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource.c_str() ) );
  return program;
}

double Utility::getNoise(unsigned x, unsigned y, unsigned random)
{
  int n = x + y * 57 + random * 131;
  n = (n<<13) ^ n;
  double noise = (1.0f - ( (n * (n * n * 15731 + 789221) +
                1376312589)&0x7fffffff)* 0.000000000931322574615478515625f);
  return noise;
}

double Utility::smoothNoise(unsigned width, unsigned height, unsigned x, unsigned y, unsigned char* noise)
{
  assert(noise);
  
  if(x==0 || x > width -2 
     || y==0 || y > height -2)
    return noise[x + y*width];
  
  double corners = (noise[x-1 + (y-1) *width]
            +noise[x+1 + (y-1)*width]
            +noise[x-1 + (y+1) * width]
            +noise[x+1 + (y+1) * width]) / 16.0;
  double sides   = (noise[x-1 + y*width]  
            +noise[x+1 + y*width]  
            +noise[x + (y-1)*width]  
            +noise[x + (y+1)*width]) / 8.0;
  double center  =  noise[x + y*width] / 4.0;
  
  return corners + sides + center;
}

osg::Texture2D* Utility::newColorTexture2D(unsigned width, unsigned height, unsigned accuracy)
{
  osg::Texture2D* texture2D = new osg::Texture2D;
  
  texture2D->setTextureSize(width, height);
  if(accuracy == 32)
    {
      texture2D->setInternalFormat(GL_RGBA32F_ARB);
      texture2D->setSourceFormat(GL_RGBA);
    }
  else if(accuracy == 8)
    {
      texture2D->setInternalFormat(GL_RGBA);
    }
  texture2D->setSourceType(GL_FLOAT);
  texture2D->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
  texture2D->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
  return texture2D;
}

osg::Geode* Utility::getCanvasQuad(unsigned width, unsigned height, double depth)
{
  osg::Vec3Array* vertices = new osg::Vec3Array;
  osg::Vec2Array* texCoords = new osg::Vec2Array;
  vertices->push_back(osg::Vec3(0,0,depth));
  texCoords->push_back(osg::Vec2(0,0));
  
  vertices->push_back(osg::Vec3(width,0,depth));
  texCoords->push_back(osg::Vec2(1,0));
      
  vertices->push_back(osg::Vec3(0,height,depth));
  texCoords->push_back(osg::Vec2(0,1));
  
  vertices->push_back(osg::Vec3(width,height,depth));
  texCoords->push_back(osg::Vec2(1,1));
      
  osg::Geometry* quad = new osg::Geometry;
  quad->setVertexArray(vertices);
  quad->setTexCoordArray(1,texCoords);
  
  quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP,0,vertices->size()));
  
  osg::Vec4Array* colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
  quad->setColorArray(colors);
  quad->setColorBinding(osg::Geometry::BIND_OVERALL);
  
  osg::Geode* geode = new osg::Geode();
  geode->addDrawable(quad);
  
  return geode;
  
}
