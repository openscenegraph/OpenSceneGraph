/*
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or (at
 * your option) any later version. The full license is in the LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * OpenSceneGraph Public License for more details.
*/

//
// Copyright(c) 2008 Skew Matrix Software LLC.
//

#ifndef __FLT_EXPORT_OPTIONS_H__
#define __FLT_EXPORT_OPTIONS_H__ 1

#include "FltWriteResult.h"

#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Options>
#include <osgDB/FileNameUtils>

#include <string>
#include <utility>
#include <vector>


namespace flt
{


/*!
   Options class for controlling export behavior.
   Features a parser for the Option string as well as getter
   methods for supported options.
 */
class ExportOptions : public osgDB::Options
{
public:
    ExportOptions( const Options* opt );
    ExportOptions();

    static const int VERSION_15_7;
    static const int VERSION_15_8;
    static const int VERSION_16_1;

    enum FlightUnits
    {
        METERS,
        KILOMETERS,
        FEET,
        INCHES,
        NAUTICAL_MILES
    };

    void setFlightFileVersionNumber( int num ) { _version = num; }
    int getFlightFileVersionNumber() const { return _version; }

    void setFlightUnits( FlightUnits units ) { _units = units; }
    FlightUnits getFlightUnits() const { return _units; }

    void setValidateOnly( bool validate ) { _validate = validate; }
    bool getValidateOnly() const { return _validate; }

    void setTempDir( const std::string& dir ) { _tempDir = dir; }
    std::string getTempDir() const { return _tempDir; }

    void setLightingDefault( bool lighting ) { _lightingDefault = lighting; }
    bool getLightingDefault() const { return _lightingDefault; }

    void setStripTextureFilePath( bool strip ) { _stripTextureFilePath = strip; }
    bool getStripTextureFilePath() const { return _stripTextureFilePath; }

    FltWriteResult & getWriteResult() const { return( wr_ ); }

    // Parse the OptionString and override values based on
    //   what was set in the OptionString.
    void parseOptionsString();

protected:
    int _version;
    FlightUnits _units;
    bool _validate;
    std::string _tempDir;
    bool _lightingDefault;
    bool _stripTextureFilePath;

    mutable FltWriteResult wr_;

    static std::string _versionOption;
    static std::string _unitsOption;
    static std::string _validateOption;
    static std::string _tempDirOption;
    static std::string _lightingOption;
    static std::string _stripTextureFilePathOption;
};

}

#endif /* __OPEN_FLIGHT_WRITER_H__ */
