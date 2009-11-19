#include "osg/Image"
#include "osg/Notify"

#include <osg/Geode>

#include <osg/observer_ptr>

#include "osg/GL"

#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include "osgDB/FileUtils"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>


#ifndef __APPLE__
#include "Components.h"
#include "QuickTimeComponents.h"
#else
#include <QuickTime/QuickTime.h>
#endif

#ifndef SEEK_SET
#  define SEEK_SET 0
#endif
#include "QTUtils.h"
#include "QTLiveUtils.h"
#include "QTImportExport.h"
#include "QuicktimeImageStream.h"
#include "QuicktimeLiveImageStream.h"


using namespace osg;


class ReaderWriterQT : public osgDB::ReaderWriter
{
public:


    // This class is used as a helper to de-initialize
    // properly quicktime, when the last media loaded 
    // with the quicktime plugin is released. 
    // All loaded media must be added to the observer 
    // (see ReaderWriterQT::readImage() function) 
    class QuicktimeInitializer : public osg::Observer
    {
    public:

       QuicktimeInitializer (): 
          _instanceCount(0),
             _setup(false)
          {}

          virtual ~QuicktimeInitializer()
          {
             // When we get here, the exit() function 
             // should have been called, when last media was released. 
             // In case no media has been added after initialization, 
             // let's perform an extra check
             if (_setup && _instanceCount == 0)
             {
                exit();
             }
          };

          void addMedia(Image* ptr)
          {
             ptr->addObserver(this);
             ++ _instanceCount;
          }

          virtual void objectDeleted(void*) 
          {
             -- _instanceCount;
             if(_instanceCount== 0)
                exit();
          }

          void init()
          {
             if (!_setup)
             {
                #ifndef __APPLE__
                InitializeQTML(0);
                #endif

                OSErr err = EnterMovies();
                if (err!=0)
                   osg::notify(osg::FATAL) << "Error while initializing quicktime: " << err << std::endl; 
                else
                   osg::notify(osg::DEBUG_INFO) << "Quicktime initialized successfully"  << std::endl;            

                _setup = true;            
             }
          }

          void exit()
          {
             #ifndef __APPLE__
             ExitMovies();
             #endif

             _setup = false;
          }

    private:
       unsigned int _instanceCount;
       bool _setup;

    };





    ReaderWriterQT()
    {

       registerQtReader();


        supportsExtension("mov","Movie format");
        supportsExtension("mpg","Movie format");
        supportsExtension("mpv","Movie format");
        supportsExtension("mp4","Movie format");
        supportsExtension("m4v","Movie format");
        supportsExtension("dv","Movie format");
        supportsExtension("avi","Movie format");
        supportsExtension("sdp","RTSP Movie format");
        supportsExtension("flv","Movie format");
        supportsExtension("swf","Movie format");
        supportsExtension("3gp","Mobile movie format");

        supportsExtension("live","Live video streaming");
        
        supportsProtocol("http", "streaming media per http");
        supportsProtocol("rtsp", "streaming media per rtsp");

        #ifdef QT_HANDLE_IMAGES_ALSO
        supportsExtension("jpg","jpg image format");
        supportsExtension("jpeg","jpeg image format");
        supportsExtension("tif","tif image format");
        supportsExtension("tiff","tiff image format");
        supportsExtension("gif","gif image format");
        supportsExtension("png","png image format");
        supportsExtension("pict","pict image format");
        supportsExtension("pct","pct image format");
        supportsExtension("tga","tga image format");
        supportsExtension("psd","psd image format");
        #endif
    }

    ~ReaderWriterQT()
    {
    }


   virtual const char* className() const { return "Default Quicktime Image Reader/Writer"; }

   virtual bool acceptsMovieExtension(const std::string& extension) const
   {
      return osgDB::equalCaseInsensitive(extension,"mov") ||
         osgDB::equalCaseInsensitive(extension,"mpg") ||
         osgDB::equalCaseInsensitive(extension,"mpv") ||
         osgDB::equalCaseInsensitive(extension,"mp4") ||
         osgDB::equalCaseInsensitive(extension,"m4v") ||
         osgDB::equalCaseInsensitive(extension,"dv")  ||
         osgDB::equalCaseInsensitive(extension,"avi") ||
         osgDB::equalCaseInsensitive(extension,"sdp") ||
         osgDB::equalCaseInsensitive(extension,"flv") ||
         osgDB::equalCaseInsensitive(extension,"swf") ||
         osgDB::equalCaseInsensitive(extension,"3gp");
   }

   virtual bool acceptsLiveExtension(const std::string& extension) const
   {
       return osgDB::equalCaseInsensitive(extension,"live");     
   }

   virtual bool acceptsExtension(const std::string& extension) const
   {
      // this should be the only image importer required on the Mac
      // dont know what else it supports, but these will do
      return

         #ifdef QT_HANDLE_IMAGES_ALSO
         osgDB::equalCaseInsensitive(extension,"jpg") || 
         osgDB::equalCaseInsensitive(extension,"jpeg") ||
         osgDB::equalCaseInsensitive(extension,"tif") ||               
         osgDB::equalCaseInsensitive(extension,"tiff") || 
         osgDB::equalCaseInsensitive(extension,"gif") ||
         osgDB::equalCaseInsensitive(extension,"png") ||
         osgDB::equalCaseInsensitive(extension,"pict") ||
         osgDB::equalCaseInsensitive(extension,"pct") ||
         osgDB::equalCaseInsensitive(extension,"tga") ||
         osgDB::equalCaseInsensitive(extension,"psd") ||
         #endif 

         acceptsMovieExtension(extension) ||
         acceptsLiveExtension(extension);
   }

   virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
   {
      std::string ext = osgDB::getLowerCaseFileExtension(file);
      if (osgDB::equalCaseInsensitive(ext,"qt"))
      {
         return readImage(osgDB::getNameLessExtension(file),options);
      }
      
      if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

      // if the file is a ".live" video encoded string then load as an ImageStream
      if (acceptsLiveExtension(ext))
      {
          long num_video_components;
          {
              // Begin QuickTime
              QTScopedQTMLInitialiser  qt_init;
              QTScopedMovieInitialiser qt_movie_init;
              //
              ComponentDescription video_component_description;
              video_component_description.componentType         = 'vdig';  /* A unique 4-byte code indentifying the command set */
              video_component_description.componentSubType      = 0L;      /* Particular flavor of this instance */
              video_component_description.componentManufacturer = 0L;      /* Vendor indentification */
              video_component_description.componentFlags        = 0L;      /* 8 each for Component,Type,SubType,Manuf/revision */
              video_component_description.componentFlagsMask    = 0L;      /* Mask for specifying which flags to consider in search, zero during registration */
              num_video_components = CountComponents (&video_component_description);
          }
          if (osgDB::getNameLessExtension(file) == "devices")
          {
              osg::notify(osg::ALWAYS) << " available Video DigitizerComponents : " << num_video_components << std::endl;
              if (num_video_components)
              {
                  // Probe Video Dig
                  probe_video_digitizer_components();
                  // Probe SG
                  std::vector<OSG_SGDeviceList> devices_list = probe_sequence_grabber_components();
                  if (devices_list.size())
                  {
                      // Video
                      OSG_SGDeviceList& video_device_list = devices_list[0];
                      // Print
                      osg::notify(osg::ALWAYS) << std::endl;
                      osg::notify(osg::ALWAYS) << "Video Component/Input IDs follow: " << std::endl;
                      osg::notify(osg::ALWAYS) << std::endl;
                      for (unsigned int device_input = 0; device_input < video_device_list.size(); ++device_input)
                      {
                          OSG_SGDevicePair device_pair = video_device_list[device_input];
                          osg::notify(osg::ALWAYS) << device_pair.first.c_str() << "    " << device_pair.second.c_str() << std::endl;
                      }
                  }
                  if (devices_list.size() > 1)
                  {
                      // Audio
                      OSG_SGDeviceList& audio_device_list = devices_list[1];
                      // Print
                      osg::notify(osg::ALWAYS) << std::endl;
                      osg::notify(osg::ALWAYS) << "Audio Component/Input IDs follow: " << std::endl;
                      osg::notify(osg::ALWAYS) << std::endl;
                      for (unsigned int device_input = 0; device_input < audio_device_list.size(); ++device_input)
                      {
                          OSG_SGDevicePair device_pair = audio_device_list[device_input];
                          osg::notify(osg::ALWAYS) << device_pair.first.c_str() << "    " << device_pair.second.c_str() << std::endl;
                      }
                  }
              }
              return ReadResult::FILE_NOT_HANDLED;
          }
          else
          {
              osg::notify(osg::DEBUG_INFO) << " available Video DigitizerComponents : " << num_video_components << std::endl;
              if (num_video_components)
              {
                  // Note from Riccardo Corsi 
                  // Quicktime initialization is done here, when a media is found
                  // and before any image or movie is loaded. 
                  // After the first call the function does nothing. 
                  // The cleaning up is left to the QuicktimeInitializer (see below)
                  _qtExitObserver.init();

                  //
                  QuicktimeLiveImageStream* p_qt_image_stream = new QuicktimeLiveImageStream(osgDB::getNameLessExtension(file));
                  // add the media to the observer for proper clean up on exit
                  _qtExitObserver.addMedia(p_qt_image_stream);
                  return p_qt_image_stream;
              }
              else
              {
                  osg::notify(osg::DEBUG_INFO) << "No available Video DigitizerComponents : " <<  std::endl;
                  return ReadResult::FILE_NOT_HANDLED;
              }
          }
      }

      // Not an encoded "live" psuedo file - so check a real file exists
      // only find the file if it isn't a URL
      std::string fileName = file;
      
      
      // Note from Riccardo Corsi 
      // Quicktime initialization is done here, when a media is found
      // and before any image or movie is loaded. 
      // After the first call the function does nothing. 
      // The cleaning up is left to the QuicktimeInitializer (see below)
      _qtExitObserver.init();


      // if the file is a movie file then load as an ImageStream.
      if (acceptsMovieExtension(ext))
      {
         // quicktime supports playing movies from URLs
         if (!osgDB::containsServerAddress(fileName)) {
             fileName = osgDB::findDataFile( file,  options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
         }
         
         // note from Robert Osfield when integrating, we should probably have so
         // error handling mechanism here.  Possibly move the load from
         // the constructor to a seperate load method, and have a valid
         // state on the ImageStream... will integrated as is right now
         // to get things off the ground.
         QuicktimeImageStream* moov = new QuicktimeImageStream(fileName);
         // moov->play();

         // add the media to the observer for proper clean up on exit
         _qtExitObserver.addMedia(moov);

         return moov;
      }
        
        
        // no live-video, no movie-file, so try to load as an image
        
        fileName = osgDB::findDataFile( file,  options);
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            
        QuicktimeImportExport importer;

        std::ifstream is;
        is.open (fileName.c_str(), std::ios::binary | std::ios::in );
        is.seekg (0, std::ios::end);
        long length = is.tellg();
        is.seekg (0, std::ios::beg);

        osg::ref_ptr<osg::Image> image = importer.readFromStream(is, fileName, length);
        is.close();
        if (!importer.success() || (image == NULL)) {
            osg::notify(osg::WARN) << "Error reading file " << file << " : " << importer.getLastErrorString() << std::endl;
            return ReadResult::ERROR_IN_READING_FILE;
        }

      _qtExitObserver.addMedia(image.get());

      return image.release();
   }
   
    virtual ReadResult readImage (std::istream& is, const osgDB::ReaderWriter::Options* options=NULL) const 
    {
        std::string filename = "";
        long sizeHint(0);
        // check options for a file-type-hint 
        if (options) {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt) 
            {
                int index = opt.find( "=" );
                if( opt.substr( 0, index ) == "filename" ||
                    opt.substr( 0, index ) == "FILENAME" )
                {
                    filename = opt.substr( index+1 );
                } else if( opt.substr( 0, index ) == "size" ||
                    opt.substr( 0, index ) == "SIZE" )
                {
                    std::string sizestr = opt.substr( index+1 );
                    sizeHint = atol(sizestr.c_str());
                }
            }
        }

        _qtExitObserver.init();
        
        QuicktimeImportExport importer;
        osg::ref_ptr<osg::Image> image = importer.readFromStream(is, filename, sizeHint);
        
        if (!importer.success() || (image == NULL)) {
            osg::notify(osg::WARN) << "Error reading from stream "  << importer.getLastErrorString() << std::endl;
            return ReadResult::ERROR_IN_READING_FILE;
        }
        _qtExitObserver.addMedia(image.get());
        return image.release();
        
    }

    virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options*) const
    {
        std::string ext = osgDB::getFileExtension(fileName);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        _qtExitObserver.init();

        //Buidl map  of extension <-> osFileTypes
        std::map<std::string, OSType> extmap;

        extmap.insert(std::pair<std::string, OSType>("jpg",  kQTFileTypeJPEG));
        extmap.insert(std::pair<std::string, OSType>("jpeg", kQTFileTypeJPEG));
        extmap.insert(std::pair<std::string, OSType>("bmp",  kQTFileTypeBMP));
        extmap.insert(std::pair<std::string, OSType>("tif",  kQTFileTypeTIFF));
        extmap.insert(std::pair<std::string, OSType>("tiff", kQTFileTypeTIFF));
        extmap.insert(std::pair<std::string, OSType>("png",  kQTFileTypePNG));
        extmap.insert(std::pair<std::string, OSType>("gif",  kQTFileTypeGIF));
        extmap.insert(std::pair<std::string, OSType>("psd",  kQTFileTypePhotoShop));
        extmap.insert(std::pair<std::string, OSType>("sgi",  kQTFileTypeSGIImage));
        
        std::map<std::string, OSType>::iterator cur = extmap.find(ext);

        // can not handle this type of file, perhaps a movie?
        if (cur == extmap.end())
         return WriteResult::FILE_NOT_HANDLED;

        std::ofstream os(fileName.c_str(), std::ios::binary | std::ios::trunc | std::ios::out);
        if(os.good()) 
        {
            QuicktimeImportExport exporter;
            exporter.writeToStream(os, const_cast<osg::Image*>(&img), fileName);
            
            if (exporter.success()) 
                return WriteResult::FILE_SAVED;
        } 

        return WriteResult::ERROR_IN_WRITING_FILE; 
    }
   
    virtual WriteResult writeImage (const osg::Image& img, std::ostream& os, const Options* options=NULL) const
    {
        std::string filename = "file.jpg"; // use jpeg if not otherwise specified
        
        if (options) {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt) 
            {
                int index = opt.find( "=" );
                if( opt.substr( 0, index ) == "filename" ||
                    opt.substr( 0, index ) == "FILENAME" )
                {
                    filename = opt.substr( index+1 );
                }
            }
        }
        
        _qtExitObserver.init();

        QuicktimeImportExport exporter;
        exporter.writeToStream(os, const_cast<osg::Image*>(&img), filename);
            
        if (exporter.success()) 
            return WriteResult::FILE_SAVED;
        
        return WriteResult::ERROR_IN_WRITING_FILE;         
    }

protected:

   //internal utils
   void registerQtReader() const
   {
      osgDB::Registry* r = osgDB::Registry::instance();
      r->addFileExtensionAlias("mov",  "qt");

      #ifdef QT_HANDLE_IMAGES_ALSO
      r->addFileExtensionAlias("jpg",  "qt");
      r->addFileExtensionAlias("jpe",  "qt");
      r->addFileExtensionAlias("jpeg", "qt");
      r->addFileExtensionAlias("tif",  "qt");
      r->addFileExtensionAlias("tiff", "qt");
      r->addFileExtensionAlias("gif",  "qt");
      r->addFileExtensionAlias("png",  "qt");
      r->addFileExtensionAlias("psd",  "qt");
      r->addFileExtensionAlias("tga",  "qt");
      r->addFileExtensionAlias("mov",  "qt");
      r->addFileExtensionAlias("avi",  "qt");
      r->addFileExtensionAlias("mpg",  "qt");
      r->addFileExtensionAlias("mpv",  "qt");
      r->addFileExtensionAlias("dv",   "qt");
      r->addFileExtensionAlias("mp4",  "qt");
      r->addFileExtensionAlias("m4v",  "qt");         
      #endif
   }

    mutable QuicktimeInitializer _qtExitObserver;
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(quicktime, ReaderWriterQT)
