#ifndef __PFB_H
#define __PFB_H

#include <stdio.h>

// Open Scene Graph includes.
#include <osg/Registry>

// Performer includes.
#include <Performer/pf/pfNode.h>

class PerformerReaderWriter : public osg::ReaderWriter
{
    public:

        PerformerReaderWriter();
        ~PerformerReaderWriter();

        virtual const char* className() { return "Performer Reader/Writer"; }
        virtual bool acceptsExtension(const std::string& extension) { return extension=="pfb"; }

        virtual osg::Object* readObject(const std::string& fileName);
        virtual osg::Node* readNode(const std::string& fileName);
        virtual bool writeObject(osg::Object& obj,const std::string& fileName);
        virtual bool writeNode(osg::Node& node,const std::string& fileName);


    protected:

        void initialisePerformer(const std::string& fileName);

    private:

        bool _perfomerInitialised;

};

#endif
