/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include "LuaScriptEngine.h"

class ReaderWriterLua : public osgDB::ReaderWriter
{
    public:

        ReaderWriterLua()
        {
            supportsExtension("lua","lua script");
        }

        virtual const char* className() const { return "Lua ScriptEngine plugin"; }

        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            osg::ref_ptr<osg::Script> script = new osg::Script;
            script->setLanguage("lua");

            std::string str;
            while(fin)
            {
                int c = fin.get();
                if (c>=0 && c<=255)
                {
                    str.push_back(c);
                }
            }
            script->setScript(str);

            return script.release();
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            if (file=="ScriptEngine.lua") return new lua::LuaScriptEngine();

            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;

            return readObject(istream, options);
        }


        virtual ReadResult readObjectFromScript(std::istream& fin, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            ReadResult result = readObject(fin, options);

            if (!result.validObject()) return result;
            osg::ref_ptr<osg::Script> script = dynamic_cast<osg::Script*>(result.getObject());
            if (!script) return ReadResult::ERROR_IN_READING_FILE;

            std::string entryPoint = "";
            osg::ScriptEngine::Parameters inputParameters;
            osg::ScriptEngine::Parameters outputParameters;

            osg::ref_ptr<lua::LuaScriptEngine> se = new lua::LuaScriptEngine();
            if (!se->run(script.get(), entryPoint, inputParameters, outputParameters)) return 0;

            if (outputParameters.empty()) return 0;

            typedef std::vector< osg::ref_ptr<osg::Object> > Objects;
            Objects objects;

            for(osg::ScriptEngine::Parameters::iterator itr = outputParameters.begin();
                itr != outputParameters.end();
                ++itr)
            {
                osg::Object* object = dynamic_cast<osg::Object*>(itr->get());
                if (object) objects.push_back(object);
            }

            if (objects.empty()) return 0;

            if (objects.size()==1)
            {
                osg::Group* group = dynamic_cast<osg::Group*>(objects[0].get());
                OSG_NOTICE<<"readObject form script, have one object "<<objects[0]->className()<<" "<<objects[0].get()<<std::endl;
                if (group) OSG_NOTICE<<"  group numChildren()="<<group->getNumChildren()<<std::endl;
                return objects[0].get();
            }

            osg::ref_ptr<osg::Group> group = new osg::Group;
            for(Objects::iterator itr = objects.begin();
                itr != objects.end();
                ++itr)
            {
                osg::Node* node = dynamic_cast<osg::Node*>(itr->get());
                if (node) group->addChild(node);
            }

            if (group->getNumChildren()>0) return group.get();
            else return 0;
        }

        virtual ReadResult readImage(std::istream& fin, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readObjectFromScript(fin, options);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;

            return readImage(istream, options);
        }

        virtual ReadResult readNode(std::istream& fin, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readObjectFromScript(fin, options);
        }

       virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
       {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;

            return readNode(istream, options);
       }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(lua, ReaderWriterLua)
