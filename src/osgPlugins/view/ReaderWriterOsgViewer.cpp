/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2007 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#include <sstream>

#include <osgViewer/View>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

class ReaderWriterOsgViewer : public osgDB::ReaderWriter
{
public:
    ReaderWriterOsgViewer()
    {
        supportsExtension("osgviewer","OpenSceneGraph viewer configuration format");
        supportsExtension("view","OpenSceneGraph viewer configuration format");
        supportsOption("precision","Set the floating point precision of output");
        supportsOption("OutputTextureFiles","Output texture image to file");
    }

    virtual const char* className() const { return "osgViewer configuration loader"; }

    void setPrecision(osgDB::Output& fout, const osgDB::ReaderWriter::Options* options) const
    {
        if (options)
        {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                if(opt=="PRECISION" || opt=="precision")
                {
                    int prec;
                    iss >> prec;
                    fout.precision(prec);
                }
                if (opt=="OutputTextureFiles")
                {
                    fout.setOutputTextureFiles(true);
                }
            }
        }
    }


    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if( !acceptsExtension(ext) )
            return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        // code for setting up the database path so that internally referenced file are searched for on relative paths.
        osgDB::ifstream fin(fileName.c_str());
        if (fin)
        {
            return readObject(fin, options);
        }
        return 0L;

    }

    virtual ReadResult readObject(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
    {
        osgDB::Input fr;
        fr.attach(&fin);
        fr.setOptions(options);

        typedef std::vector< osg::ref_ptr<osgViewer::View> > ViewList;
        ViewList viewList;

        // load all nodes in file, placing them in a group.
        while(!fr.eof())
        {
            osg::ref_ptr<osg::Object> object = fr.readObject();
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(object.get());
            if (view)
            {
                viewList.push_back(view);
            }
            else fr.advanceOverCurrentFieldOrBlock();
        }

        if  (viewList.empty())
        {
            return ReadResult("No data loaded");
        }
        else if (viewList.size()==1)
        {
            return viewList.front().get();
        }
        else
        {
            OSG_NOTICE<<"Found multiple view's, just taking first"<<std::endl;
            return viewList.front().get();
        }

    }

    virtual WriteResult writeObject(const osg::Object& obj,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osgDB::Output fout(fileName.c_str());
        fout.setOptions(options);
        if (fout)
        {
            setPrecision(fout,options);

            fout.writeObject(obj);
            fout.close();
            return WriteResult::FILE_SAVED;
        }
        return WriteResult("Unable to open file for output");
    }


    virtual WriteResult writeObject(const osg::Object& obj,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
    {
        osgDB::Output foutput;
        foutput.setOptions(options);

        std::ios &fios = foutput;
        fios.rdbuf(fout.rdbuf());

        if (fout)
        {
            setPrecision(foutput,options);

            foutput.writeObject(obj);
            return WriteResult::FILE_SAVED;
        }
        return WriteResult("Unable to write to output stream");
    }

};


// Add ourself to the Registry to instantiate the reader/writer.
REGISTER_OSGPLUGIN(osgViewer, ReaderWriterOsgViewer)

