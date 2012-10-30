/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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


/* README:
 * 
 * This code is loosely based on the QTKit implementation of Eric Wing, I removed
 * some parts and added other parts. 
 * 
 * What's new:
 * - it can handle URLs currently http and rtsp
 * - it supports OS X's CoreVideo-technology, this will render the movie-frames
 *   into a bunch of textures. If you load your movie via readImageFile you'll
 *   get the standard behaviour, an ImageStream, where the data gets updated on 
 *   every new video-frame. This may be slow.
 *   To get CoreVideo, you'll need to use readObjectFile and cast the result (if any)
 *   to an osg::Texture and use that as your video-texture. If you need access to the
 *   imagestream, just cast getImage to an image-stream. Please note, the data-
 *   property of the image-stream does NOT store the current frame, instead it's empty.
 *
 */


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include "RestHttpDevice.hpp"





class ReaderWriterRestHttp : public osgDB::ReaderWriter
{
    public:

        ReaderWriterRestHttp()
        {
            supportsExtension("resthttp", "Virtual Device Integration via a HTTP-Server and a REST-interface");
            
            supportsOption("documentRoot", "document root of asset files to server via the http-server");
            supportsOption("serverAddress", "server address to listen for incoming requests");
            supportsOption("serverPort", "server port to listen for incoming requests");
            supportsOption("documentRegisteredHandlers", "dump a documentation of all registered REST-handler to the console");
            
        }

        virtual const char* className() const { return "Rest/HTTP Virtual Device Integration plugin"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            if (osgDB::getFileExtension(file) == "resthttp")
            {
                std::string document_root = options ? options->getPluginStringData("documentRoot") : "htdocs/";
                std::string server_address = options ? options->getPluginStringData("serverAddress") : "localhost";
                std::string server_port    = options ? options->getPluginStringData("serverPort") : "8080";
                
                // supported file-name scheme to get address, port and document-root: <server-address>:<server-port>/<document-root>
                // example: '192.168.1.1:88888/var/www/htdocs'
                
                std::string file_wo_ext = osgDB::getNameLessAllExtensions(file);
                if ((file_wo_ext.find('/') != std::string::npos) && (file_wo_ext.find(':') != std::string::npos)) {
                    std::string server_part = file_wo_ext.substr(0, file_wo_ext.find('/'));
                    document_root = file_wo_ext.substr(file_wo_ext.find('/'));
                    server_address = server_part.substr(0,server_part.find(':'));
                    server_port = server_part.substr(server_part.find(':') + 1);
                }
                
                try
                {
                    osg::ref_ptr<RestHttpDevice> device = new RestHttpDevice(server_address, server_port, document_root);
                    
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/slide/first", osgGA::GUIEventAdapter::KEY_Home));
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/slide/last", osgGA::GUIEventAdapter::KEY_End));
                    
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/slide/next", osgGA::GUIEventAdapter::KEY_Right));
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/slide/previous", osgGA::GUIEventAdapter::KEY_Left));

                    device->addRequestHandler(new SendKeystrokeRequestHandler("/layer/next", osgGA::GUIEventAdapter::KEY_Down));
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/layer/previous", osgGA::GUIEventAdapter::KEY_Up));
                    
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/slideorlayer/next", osgGA::GUIEventAdapter::KEY_Page_Down));
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/slideorlayer/previous", osgGA::GUIEventAdapter::KEY_Page_Up));
                    
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/unpause", 'o'));
                    device->addRequestHandler(new SendKeystrokeRequestHandler("/pause", 'p'));
                    
    
                
                    if (options && (options->getPluginStringData("documentRegisteredHandlers") == "true"))
                    {
                        std::cout << *device << std::endl;
                    }
                    return device.release();
                }
                catch(std::exception& e)
                {
                    OSG_WARN << "ReaderWriterRestHttpDevice : could not create http-server! Reason: " << e.what() << std::endl;
                    return ReadResult::ERROR_IN_READING_FILE;
                }
                catch(...)
                {
                    OSG_WARN << "ReaderWriterRestHttpDevice : could not create http-server, unknown excpetion thrown " << std::endl;
                    return ReadResult::ERROR_IN_READING_FILE;
                }
                
            }
            
            return ReadResult::FILE_NOT_HANDLED;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(resthttp, ReaderWriterRestHttp)
