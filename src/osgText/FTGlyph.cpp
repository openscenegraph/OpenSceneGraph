#include	"FTGlyph.h"

#include <iostream>
#include <string.h>

FTGlyph::FTGlyph()
:	advance(0),
	err(0)	
{
        //cout << "**** FTGlyph() size = "<<sizeof(FTGlyph)<<endl;
        memset(this,0,sizeof(FTGlyph));

        advance=0;
	err=0;
        
        pos.x = 0;
	pos.y = 0;
        
}


FTGlyph::~FTGlyph()
{}
