/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#include <osg/Math>

#include <string.h>


double osg::asciiToDouble(const char* str)
{
    const char* ptr = str;

    // check if could be a hex number.
    if (strncmp(ptr,"0x",2)==0)
    {

        double value = 0.0;

        // skip over leading 0x, and then go through rest of string
        // checking to make sure all values are 0...9 or a..f.
        ptr+=2;
        while (
               *ptr!=0 &&
               ((*ptr>='0' && *ptr<='9') ||
                (*ptr>='a' && *ptr<='f') ||
                (*ptr>='A' && *ptr<='F'))
              )
        {
            if (*ptr>='0' && *ptr<='9') value = value*16.0 + double(*ptr-'0');
            else if (*ptr>='a' && *ptr<='f') value = value*16.0 + double(*ptr-'a'+10);
            else if (*ptr>='A' && *ptr<='F') value = value*16.0 + double(*ptr-'A'+10);
            ++ptr;
        }

        // OSG_NOTICE<<"Read "<<str<<" result = "<<value<<std::endl;
        return value;
    }

    ptr = str;

    bool    hadDecimal[2];
    double  value[2];
    double  sign[2];
    double  decimalMultiplier[2];

    hadDecimal[0] = hadDecimal[1] = false;
    sign[0] = sign[1] = 1.0;
    value[0] = value[1] = 0.0;
    decimalMultiplier[0] = decimalMultiplier[1] = 0.1;
    int pos = 0;

    // compute mantissa and exponent parts
    while (*ptr!=0 && pos<2)
    {
        if (*ptr=='+')
        {
            sign[pos] = 1.0;
        }
        else if (*ptr=='-')
        {
            sign[pos] = -1.0;
        }
        else if (*ptr>='0' && *ptr<='9')
        {
            if (!hadDecimal[pos])
            {
                value[pos] = value[pos]*10.0 + double(*ptr-'0');
            }
            else
            {
                value[pos] = value[pos] + decimalMultiplier[pos] * double(*ptr-'0');
                decimalMultiplier[pos] *= 0.1;
            }
        }
        else if (*ptr=='.')
        {
            hadDecimal[pos] = true;
        }
        else if (*ptr=='e' || *ptr=='E')
        {
            if (pos==1) break;

            pos = 1;
        }
        else
        {
            break;
        }
        ++ptr;
    }

    if (pos==0)
    {
        // OSG_NOTICE<<"Read "<<str<<" result = "<<value[0]*sign[0]<<std::endl;
        return value[0]*sign[0];
    }
    else
    {
        double mantissa = value[0]*sign[0];
        double exponent = value[1]*sign[1];
        //OSG_NOTICE<<"Read "<<str<<" mantissa = "<<mantissa<<" exponent="<<exponent<<" result = "<<mantissa*pow(10.0,exponent)<<std::endl;
        return mantissa*pow(10.0,exponent);
    }
}

double osg::findAsciiToDouble(const char* str)
{
   const char* ptr = str;
   double value = 0.0;

   while(*ptr != 0) {

       if(*ptr>='0' && *ptr<='9') {
           value = asciiToDouble(ptr);
           return value;
       }

       ++ptr;
   }

   return 0.0;
}
