#ifndef READER_WRITER_POV_H
#define READER_WRITER_POV_H

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>


class ReaderWriterPOV : public osgDB::ReaderWriter
{
   public:
      ReaderWriterPOV();

      virtual const char* className() const
      {
         return "POV reader/writer";
      }

      bool isInventorExtension( const std::string& extension ) const
      {
         return osgDB::equalCaseInsensitive( extension, "pov" ) ? true : false;
      }

      virtual WriteResult writeNode( const osg::Node& node,
                                     const std::string& filename,
                                     const osgDB::ReaderWriter::Options* options = NULL) const;

      virtual WriteResult writeNode( const osg::Node& node, std::ostream& fout,
                                     const Options* = NULL) const;

};


#endif /* READER_WRITER_POV_H */
