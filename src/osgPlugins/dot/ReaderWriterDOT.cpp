/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osgDB/ReaderWriter>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include "SimpleDotVisitor.h"

class ReaderWriterDOT : public osgDB::ReaderWriter {
  public:
    virtual const char* className() const { return "DOT Writer"; }
    virtual bool acceptsExtension(const std::string& extension) const { return osgDB::equalCaseInsensitive(extension,"dot"); }

    virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName,const Options* options = NULL) const { 
      std::ofstream o( fileName.c_str(), std::ios_base::out );
      if ( o ) {
        return writeNode( node, o, options );
      }

      return WriteResult(WriteResult::ERROR_IN_WRITING_FILE);
    }
    
    virtual WriteResult writeNode(const osg::Node& node,std::ostream& fout,const Options* options = NULL) const {
      osgDot::SimpleDotVisitor sdv;
      sdv.run( *const_cast<osg::Node*>( &node ), &fout );
      return WriteResult(WriteResult::FILE_SAVED);
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dot, ReaderWriterDOT)
