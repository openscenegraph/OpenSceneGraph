#include	"FTCharmap.h"


FTCharmap::FTCharmap( FT_Face face)
:	ftFace( face),
	err(0)
{
	// Check that the default is valid
	if( !face->charmap)
	{
		FT_Set_Charmap( ftFace, ftFace->charmaps[0]);
	}
	
	ftEncoding = face->charmap->encoding;
}


FTCharmap::~FTCharmap()
{
	charMap.clear();
}


bool FTCharmap::CharMap( FT_Encoding encoding)
{
	if( ftEncoding == encoding)
	{
		return true;
	}
	
	err = FT_Select_Charmap( ftFace, encoding );
	
	if( !err)
	{
		ftEncoding = encoding;
		charMap.clear();
	}
	
	return !err;
}


bool FTCharmap::CharMap( FT_UShort platform, FT_UShort encoding)
{
	FT_CharMap  found = 0;
	FT_CharMap  charmap;
 
	for( int n = 0; n < ftFace->num_charmaps; n++ )
	{
		charmap = ftFace->charmaps[n];

		if( charmap->platform_id == platform && charmap->encoding_id == encoding)
		{
			found = charmap;
			break;
		}
	}
 
	if( !found )
	{
		return false;
	}
 
	if( ftEncoding == found->encoding)
	{
		return true;
	}
	
	/* now, select the charmap for the face object */
	err = FT_Set_Charmap( ftFace, found );
	
	if( !err)
	{
		ftEncoding = found->encoding;
		charMap.clear();
	}
	
	return !err;
}


unsigned int FTCharmap::CharIndex( unsigned int index )
{
	CharacterMap::const_iterator result = charMap.find( index);
		
	if( result == charMap.end())
	{
		unsigned int glyph = FT_Get_Char_Index( ftFace, index);
		charMap.insert( CharacterMap::value_type( index, glyph));
		return glyph;
	}
	else
	{
		return result->second;
	}

}
