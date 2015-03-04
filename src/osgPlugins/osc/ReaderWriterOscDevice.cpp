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


/*
 *  ReadMe
 *
 *  the osc-plugin can return an osgGA::Device which handles various osc-messages
 *  and puts them into the event-queue of the app
 *  you can set arbitrary values via /osg/set_user_value, these values
 *  are set on the attached UserDataConntainer (see below)
 *
 *  To open the osc-device for receiving do something like this:
 *
 *  std::string filename = "<your-port-number-to-listen-on>.receiver.osc";
 *  osgGA::Device* device = dynamic_cast<osgGA::Device*>(osgDB::readObjectFile(filename));
 *
 *  and add that device to your viewer
 *  The plugin supports the following option: documentRegisteredHandlers, which will
 *  dump all registered handlers to the console. The device registers some convenient
 *  handlers to remote control a p3d-presentation.
 *
 *
 *  The plugin supports forwarding most of the events per osc to another host.
 *  It uses a special event-handler, which forwards the events. To get this
 *  event-handler, do something like this:
 *
 *  std::string filename = "<target-address>:<target-port>.sender.osc";
 *  osgGA::GUIEventHandler* event_handler = dynamic_cast<osgGA::GUIEventHandler*>(osgDB::readObjectFile(filename));
 *
 *  and add that event handler as the first event handler to your viewer/app
 *
 *
 *  TODO:
 *  - be more tolerant with given filenames
 */



#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include "OscSendingDevice.hpp"
#include "OscReceivingDevice.hpp"
#include <osgPresentation/PropertyManager>




class ReaderWriterOsc : public osgDB::ReaderWriter
{
    public:

        ReaderWriterOsc()
        {
            supportsExtension("osc", "Virtual Device Integration via a OSC_receiver");
            supportsOption("documentRegisteredHandlers", "dump a documentation of all registered REST-handler to the console");
            supportsOption("numMessagesPerEvent", "set the number of osc-messages to send for one event (sender-only)");
            supportsOption("delayBetweenSendsInMillisecs", "when sending multiple msgs per event you can specify an optional delay between the sends (sender-only)");


        }

        virtual const char* className() const { return "OSC Virtual Device Integration plugin"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            if (osgDB::getFileExtension(file) == "osc")
            {
                std::string file_name = osgDB::getNameLessExtension(file);

                if (osgDB::getFileExtension(file_name) == "sender")
                {
                    file_name = osgDB::getNameLessExtension(file_name);

                    std::string server_address = file_name.substr(0,file_name.find(':'));
                    std::string server_port = file_name.substr(file_name.find(':') + 1);

                    unsigned int num_messages_per_event = 1;
                    if (options && !options->getPluginStringData("numMessagesPerEvent").empty()) {
                        std::string num_messages_per_event_str = options->getPluginStringData("numMessagesPerEvent");
                        num_messages_per_event = osg::maximum(1, atoi(num_messages_per_event_str.c_str()));
                    }

                    unsigned int delay_between_sends_in_millisecs = 0;
                    if (options && !options->getPluginStringData("delayBetweenSendsInMillisecs").empty()) {
                        std::string delay_between_sends_in_millisecs_str = options->getPluginStringData("delayBetweenSendsInMillisecs");
                        delay_between_sends_in_millisecs = atoi(delay_between_sends_in_millisecs_str.c_str());
                    }

                    return new OscSendingDevice(server_address, atoi(server_port.c_str()), num_messages_per_event, delay_between_sends_in_millisecs);
                }
                else
                {
                    // defaults to receiver
                    file_name = osgDB::getNameLessExtension(file_name);
                    if (file_name.find(':') == std::string::npos) {
                        file_name = "0.0.0.0:" + file_name;
                    }
                    std::string server_address = file_name.substr(0,file_name.find(':'));
                    std::string server_port = file_name.substr(file_name.find(':') + 1);
                    int port = atoi(server_port.c_str());
                    if (port <= 0)
                    {
                        OSG_WARN << "ReaderWriterOsc :: can't get valid port from " << osgDB::getNameLessAllExtensions(file) << std::endl;
                        port = 8000;
                    }
                    try {

                        osg::ref_ptr<OscReceivingDevice> device = new OscReceivingDevice(server_address, port);


                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/slide/first", osgGA::GUIEventAdapter::KEY_Home));
                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/slide/last", osgGA::GUIEventAdapter::KEY_End));

                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/slide/next", osgGA::GUIEventAdapter::KEY_Right));
                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/slide/previous", osgGA::GUIEventAdapter::KEY_Left));

                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/layer/next", osgGA::GUIEventAdapter::KEY_Down));
                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/layer/previous", osgGA::GUIEventAdapter::KEY_Up));

                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/slideorlayer/next", osgGA::GUIEventAdapter::KEY_Page_Down));
                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/slideorlayer/previous", osgGA::GUIEventAdapter::KEY_Page_Up));

                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/unpause", 'o'));
                        device->addRequestHandler(new SendKeystrokeRequestHandler("/p3d/pause", 'p'));

                        device->addRequestHandler(new SendKeystrokeRequestHandler("/osgviewer/home", ' '));
                        device->addRequestHandler(new SendKeystrokeRequestHandler("/osgviewer/stats", 's'));




                        if ((options && (options->getPluginStringData("documentRegisteredHandlers") == "true")))
                        {
                            std::cout << *device << std::endl;
                        }


                        return device.release();
                    }
                    catch(const osc::Exception& e)
                    {
                        OSG_WARN << "OscDevice :: could not register UDP listener : " << e.what() << std::endl;
                        return ReadResult::ERROR_IN_READING_FILE;
                    }
                    catch(const std::exception& e)
                    {
                        OSG_WARN << "OscDevice :: could not register UDP listener : " << e.what() << std::endl;
                        return ReadResult::ERROR_IN_READING_FILE;
                    }
                    catch(...)
                    {
                        OSG_WARN << "OscDevice :: could not register UDP listener" << std::endl;
                        return ReadResult::ERROR_IN_READING_FILE;
                    }

                }

            }

            return ReadResult::FILE_NOT_HANDLED;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(osc, ReaderWriterOsc)
