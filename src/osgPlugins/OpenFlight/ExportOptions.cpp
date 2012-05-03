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

#include <osg/Notify>
#include <osg/ProxyNode>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include "ExportOptions.h"

#include <fstream>
#include <sstream>

namespace flt
{



/** @name Valid Option Strings
 *  This plugin supports the following \c Option string values.
 */
//@{
/** Value: "version".
 *  Specifies the version of the output OpenFlight file. Supported values
 *  include 15.7, 15.8, and 16.1. Default is 16.1. Example:
 *  "version=15.8".
 */
std::string ExportOptions::_versionOption( "version" );
/** Value: "units".
 *  Specifies the contents of the \c Units field of the OpenFliht header record.
 *  Valid values include INCHES, FEET, METERS, KILOMETERS, and NATICAL_MILES.
 *  Default is METERS. Example: "units=METERS".
 */
std::string ExportOptions::_unitsOption( "units" );
/** Value: "validate".
 *  If present in the Options string, the plugin does not write an OpenFlight file.
 *  Instead, it returns an indication of the scene graph's suitability for
 *  OpenFlight export.
 */
std::string ExportOptions::_validateOption( "validate" );
/** Value: "tempDir".
 *  Specifies the directory to use for creation of temporary files. If not
 *  specified, the directory is taken from the file name. If the file doesn't
 *  contain a path, the current working directory is used. Applications should
 *  set this to the name of their app-specific temp directory. If the path
 *  contains spaces, use double quotes to ensure correct parsing. Examples:
 *  "tempDir=/tmp".
 *  "tempDir=\"C:\\My Temp Dir\"".
 */
std::string ExportOptions::_tempDirOption( "tempDir" );
/** Value: "lighting".
 *  Specifies a default enable/disable state for lighting, for Nodes in the
 *  exported scene graph that don't set it explicitly. By default, the
 *  exporter assumes lighting is enabled (GL_LIGHTING ON). Set this to
 *  either ON or OFF. Example:
 *  "lighting=OFF".
 */
std::string ExportOptions::_lightingOption( "lighting" );
/** Value: "stripTextureFilePath".
 *  If present in the Options string, the exporter strips the path from
 *  texture file names, and writes only the texture file name to the FLT
 *  Texture Palette. By default, the exporter doesn't strip the path,
 *  and the name written to the Texture Palette is taken directly from
 *  the osg::Image object referenced by the osg::Texture2D StateAttribute.
 */
std::string ExportOptions::_stripTextureFilePathOption( "stripTextureFilePath" );
//@}


using namespace osgDB;


const int ExportOptions::VERSION_15_7( 1570 );
const int ExportOptions::VERSION_15_8( 1580 );
const int ExportOptions::VERSION_16_1( 1610 );



ExportOptions::ExportOptions()
  : _version( VERSION_16_1 ),
    _units( METERS ),
    _validate( false ),
    _lightingDefault( true ),
    _stripTextureFilePath( false )
{
}

ExportOptions::ExportOptions( const osgDB::ReaderWriter::Options* opt )
  : _version( VERSION_16_1 ),
    _units( METERS ),
    _validate( false ),
    _lightingDefault( true ),
    _stripTextureFilePath( false )

{
    if (opt)
    {
        const ExportOptions* fltOpt = dynamic_cast<const ExportOptions*>( opt );
        if (fltOpt)
        {
            _version = fltOpt->_version;
            _units = fltOpt->_units;
            _validate = fltOpt->_validate;
            _tempDir = fltOpt->_tempDir;
            _lightingDefault = fltOpt->_lightingDefault;
        }
        setOptionString( opt->getOptionString() );
    }
}

void
ExportOptions::parseOptionsString()
{
    // Parse out the option string and store values directly in
    //   ExportOptions member variables.

    const std::string& str = getOptionString();
    if (str.empty())
        return;

    std::string::size_type pos( 0 );
    while (pos != str.npos)
    {
        // Skip leading spaces.
        while ( (pos < str.length()) &&
            (str[pos] == ' ') )
            pos++;

        // Get the next token
        std::string::size_type count = str.substr( pos ).find_first_of( " =" );
        std::string token = str.substr( pos, count );
        if (count == str.npos)
            pos = str.npos;
        else
            pos += (count+1);

        // See if it's a Boolen/toggle
        if ( token == _validateOption )
        {
            OSG_INFO << "fltexp: Found: " << token << std::endl;
            setValidateOnly( true );
            continue;
        }
        if ( token == _stripTextureFilePathOption )
        {
            OSG_INFO << "fltexp: Found: " << token << std::endl;
            setStripTextureFilePath( true );
            continue;
        }
        // Protect against unrecognized options without values
        if ( pos == str.npos )
        {
            OSG_WARN << "fltexp: Bogus OptionString: " << token << std::endl;
            continue;
        }

        // Not a Boolean/toggle. Must have a value.
        // Get the value of the token, which could be double-quoted.
        if( str[pos] == '"' )
        {
            ++pos;
            count = str.substr( pos ).find_first_of( '"' );
        }
        else
            count = str.substr( pos ).find_first_of( ' ' );
        std::string value = str.substr( pos, count );
        if (count == str.npos)
            pos = str.npos;
        else
            pos += (count+1);

        if (token == _versionOption)
        {
            OSG_INFO << "fltexp: Token: " << token << ", Value: " << value << std::endl;
            int version( VERSION_16_1 );
            if( value == std::string( "15.7" ) )
                version = VERSION_15_7;
            else if( value == std::string( "15.8" ) )
                version = VERSION_15_8;
            else if( value != std::string( "16.1" ) )
                OSG_WARN << "fltexp: Unsupported version: " << value << ". Defaulting to 16.1." << std::endl;
            setFlightFileVersionNumber( version );
        }
        else if (token == _unitsOption)
        {
            OSG_INFO << "fltexp: Token: " << token << ", Value: " << value << std::endl;
            FlightUnits units( METERS );
            if( value == std::string( "KILOMETERS" ) )
                units = KILOMETERS;
            else if( value == std::string( "FEET" ) )
                units = FEET;
            else if( value == std::string( "INCHES" ) )
                units = INCHES;
            else if( value == std::string( "NAUTICAL_MILES" ) )
                units = NAUTICAL_MILES;
            else if( value != std::string( "METERS" ) )
                OSG_WARN << "fltexp: Unsupported units: " << value << ". Defaulting to METERS." << std::endl;
            setFlightUnits( units );
        }
        else if (token == _tempDirOption)
        {
            OSG_INFO << "fltexp: Token: " << token << ", Value: " << value << std::endl;
            setTempDir( value );
        }
        else if (token == _lightingOption)
        {
            OSG_INFO << "fltexp: Token: " << token << ", Value: " << value << std::endl;
            bool lighting( true );
            if (value == std::string( "OFF" ) )
                lighting = false;
            else if (value != std::string( "ON" ) )
                OSG_WARN << "fltexp: Unsupported lighting value: " << value << ". Defaulting to ON." << std::endl;
            setLightingDefault( lighting );
        }
        else
            OSG_WARN << "fltexp: Bogus OptionString: " << token << std::endl;
    }
}


}
