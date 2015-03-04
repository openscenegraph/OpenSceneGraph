/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osgWidget/PdfReader>

#include <osgPresentation/SlideShowConstructor>
#include <osgPresentation/AnimationMaterial>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <osgDB/XmlParser>

#include <sstream>
#include <iostream>


/**
 * OpenSceneGraph plugin wrapper/converter.
 */
class ReaderWriterPaths : public osgDB::ReaderWriter
{
public:
    ReaderWriterPaths()
    {
        supportsExtension("material","Material animation Ascii file format");
        supportsExtension("path","Animation path Ascii file format");
        supportsExtension("pivot_path","Animation pivot path Ascii file format");
        supportsExtension("rotation_path","Animation rotation path Ascii file format");
    }

    virtual const char* className() const
    {
        return "Path Reader/Writer";
    }

    virtual bool acceptsExtension(const std::string& extension) const
    {
        return osgDB::equalCaseInsensitive(extension,"material") ||
               osgDB::equalCaseInsensitive(extension,"path") ||
               osgDB::equalCaseInsensitive(extension,"pivot_path") ||
               osgDB::equalCaseInsensitive(extension,"rotation_path");
    }

    virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& fileName, const osgDB::Options* options) const;

    virtual osgDB::ReaderWriter::ReadResult readObject(std::istream& fin, const osgDB::Options* options) const;

    virtual osgDB::ReaderWriter::ReadResult read_material(std::istream& fin, const osgDB::Options* options) const;
    virtual osgDB::ReaderWriter::ReadResult read_path(std::istream& fin, const osgDB::Options* options) const;
    virtual osgDB::ReaderWriter::ReadResult read_pivot_path(std::istream& fin, const osgDB::Options* options) const;
    virtual osgDB::ReaderWriter::ReadResult read_rotation_path(std::istream& fin, const osgDB::Options* options) const;
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(paths, ReaderWriterPaths)

osgDB::ReaderWriter::ReadResult ReaderWriterPaths::readObject(const std::string& file, const osgDB::Options* options) const
{


    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    OSG_INFO<<"ReaderWriterPaths::readObject("<<file<<")"<<std::endl;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    OSG_INFO<<"  Found path file :"<<fileName<<std::endl;

    // code for setting up the database path so that internally referenced file are searched for on relative paths.
    osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ? static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->setPluginStringData("filename",fileName);

    osgDB::ifstream input(fileName.c_str());

    return readObject(input, local_opt.get());
}

osgDB::ReaderWriter::ReadResult ReaderWriterPaths::readObject(std::istream& fin, const osgDB::Options* options) const
{
    OSG_INFO<<"ReaderWriterPaths::readObject(std::istream& fin"<<std::endl;

    if (!options) return ReadResult::FILE_NOT_HANDLED;
    if (!fin) return ReadResult::ERROR_IN_READING_FILE;


    std::string filename = options->getPluginStringData("filename");

    std::string ext = osgDB::getLowerCaseFileExtension(filename);

    OSG_INFO<<"   filename found in options: "<<filename<<"  extension="<<ext<<std::endl;


    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;


    if      (ext=="path") return read_path(fin, options);
    else if (ext=="material") return read_material(fin, options);
    else if (ext=="pivot_path") return read_pivot_path(fin, options);
    else if (ext=="rotation_path") return read_rotation_path(fin, options);

    return ReadResult::FILE_NOT_HANDLED;
}

osgDB::ReaderWriter::ReadResult ReaderWriterPaths::read_material(std::istream& fin, const osgDB::Options* options) const
{
    osg::ref_ptr<osgPresentation::AnimationMaterial> animationMaterial = new osgPresentation::AnimationMaterial;
    animationMaterial->read(fin);

    return animationMaterial.get();
}

osgDB::ReaderWriter::ReadResult ReaderWriterPaths::read_path(std::istream& fin, const osgDB::Options* options) const
{
    osg::ref_ptr<osg::AnimationPath> animation = new osg::AnimationPath;
    animation->read(fin);
    return animation.get();
}

osgDB::ReaderWriter::ReadResult ReaderWriterPaths::read_pivot_path(std::istream& fin, const osgDB::Options* options) const
{
    osg::ref_ptr<osg::AnimationPath> animation = new osg::AnimationPath;

    while (!fin.eof())
    {
        double time;
        osg::Vec3 pivot;
        osg::Vec3 position;
        float scale;
        osg::Quat rotation;
        fin >> time >> pivot.x() >> pivot.y() >> pivot.z() >> position.x() >> position.y() >> position.z() >> rotation.x() >> rotation.y() >> rotation.z() >> rotation.w() >> scale;
        if(!fin.eof())
        {
            osg::Matrix SR = osg::Matrix::scale(scale,scale,scale)*
                                osg::Matrixf::rotate(rotation);

            osg::Matrix invSR;
            invSR.invert(SR);

            position += (invSR*pivot)*SR;

            animation->insert(time,osg::AnimationPath::ControlPoint(position,rotation,osg::Vec3(scale,scale,scale)));
        }
    }

    return animation.get();
}

struct RotationPathData
{
    RotationPathData():
        time(0.0),
        scale(1.0f),
        azim(0.0f),
        elevation(0.0f) {}

    double time;
    osg::Vec3 pivot;
    osg::Vec3 position;
    float scale;
    float azim;
    float elevation;

    void addToPath(osg::AnimationPath* animation) const
    {
        osg::Quat Rx, Rz, rotation;

        Rx.makeRotate(osg::DegreesToRadians(elevation),1.0f,0.0f,0.0f);
        Rz.makeRotate(osg::DegreesToRadians(azim),0.0f,0.0f,1.0f);
        rotation = Rz * Rx; // note, I believe this is the wrong way round, but I had to put it in this order to fix the Quat properly.

        osg::Matrix SR = osg::Matrix::scale(scale,scale,scale)*
                         osg::Matrixf::rotate(rotation);

        osg::Matrix invSR;
        invSR.invert(SR);

        osg::Vec3 local_position = position + (invSR*pivot)*SR;

        animation->insert(time,osg::AnimationPath::ControlPoint(local_position,rotation,osg::Vec3(scale,scale,scale)));
    }

};

osgDB::ReaderWriter::ReadResult ReaderWriterPaths::read_rotation_path(std::istream& fin, const osgDB::Options* options) const
{
    osg::ref_ptr<osg::AnimationPath> animation = new osg::AnimationPath;

    RotationPathData prevValue;
    bool first = true;
    while (!fin.eof())
    {
        RotationPathData currValue;
        fin >> currValue.time >> currValue.pivot.x() >> currValue.pivot.y() >> currValue.pivot.z() >> currValue.position.x() >> currValue.position.y() >> currValue.position.z() >> currValue.azim >> currValue.elevation >> currValue.scale;

        if(!fin.eof())
        {

            if (!first)
            {

                unsigned int num = 20;
                float dr = 1.0f/(float)num;
                float r=dr;
                for(unsigned int i=0;
                    i<num;
                    ++i, r+=dr)
                {
                    RotationPathData localValue;
                    localValue.time = currValue.time *r + prevValue.time * (1.0f-r);
                    localValue.pivot = currValue.pivot *r + prevValue.pivot * (1.0f-r);
                    localValue.position = currValue.position *r + prevValue.position * (1.0f-r);
                    localValue.scale = currValue.scale *r + prevValue.scale * (1.0f-r);
                    localValue.azim = currValue.azim *r + prevValue.azim * (1.0f-r);
                    localValue.elevation = currValue.elevation *r + prevValue.elevation * (1.0f-r);

                    localValue.addToPath(animation.get());
                }
            }
            else
            {
                currValue.addToPath(animation.get());
            }
            prevValue = currValue;
            first = false;
        }

    }
    return animation.get();
}
