// ExternalRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "FltFile.h"
#include "ExternalRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          ExternalRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<ExternalRecord> g_ExternalProxy;

ExternalRecord::ExternalRecord()
{
}


// virtual
ExternalRecord::~ExternalRecord()
{
}

/**
 * Return the filename of the external record.  External references are
 * specified as either:
 *    filename              - include the entire file
 *    filename<modelname>   - include only the specified model from the file
 *
 * Therefore, the filename is either the entire string or the portion of the
 * string prior to the first '<' character.
 **/
const std::string ExternalRecord::getFilename( void ) 
{ 
  std::string wholeFilename(getData()->szPath);
  std::string filename;

  size_t pos = wholeFilename.find_first_of( "<" );

  if (pos != std::string::npos)
  {
    filename = wholeFilename.substr( 0, pos );
  }
  else
  {
    filename = wholeFilename;
  }

  return filename;
}



/**
 * Return the modelname of the external record.  External references are
 * specified as either:
 *    filename              - include the entire file
 *    filename<modelname>   - include only the specified model from the file
 *
 * Therefore, the modelname is the portion of the string between '<' and '>'
 * characters when those characters are present.  Otherwise there is no
 * modelname.
 **/
std::string ExternalRecord::getModelName( void ) 
{ 
  std::string wholeFilename(getData()->szPath);
  std::string modelName;

  size_t pos = wholeFilename.find_first_of( "<" );

  if (pos != std::string::npos)
  {
    size_t pos2 = wholeFilename.find_first_of( ">" );

    // Starting after the first '<', return the characters prior to the 
    // first '>'.
    modelName = wholeFilename.substr( pos+1, pos2-pos-1 );
  }

  return modelName;
}


void ExternalRecord::setExternal(FltFile* pExternal)
{
    _fltfile = pExternal;
}


void ExternalRecord::endian()
{
    SExternalReference *pSExternal = (SExternalReference*)getData();

    if (getFlightVersion() > 13)
    {
        ENDIAN( pSExternal->dwFlags );
    }
}
