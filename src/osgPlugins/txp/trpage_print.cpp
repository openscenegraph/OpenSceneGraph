/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the President of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   4400 East Broadway #314
   Tucson, AZ  85711
   info@terrex.com
   Tel: (520) 323-7990
   ************************
   */

#include <osgDB/FileUtils>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_print.cpp
   Print out the contents of a TerraPage archive.
   This module provides an example of how to access each of the classes
   within a TerraPage archive.
*/

#include <trpage_print.h>

/* ******************************************
   Print Buffer implementation
   The print buffer is a way to dump debugging data out
   to a file (or console).  You can make your own subclass
   of trpgPrintBuffer if you have specific needs.
   ******************************************
   */

trpgPrintBuffer::trpgPrintBuffer()
{
    curIndent = 0;
    indentStr[0] = 0;
}

// Increase the current indent
void trpgPrintBuffer::IncreaseIndent(int amount)
{
    curIndent+=amount;
    updateIndent();
}

// Decrease the current indent
void trpgPrintBuffer::DecreaseIndent(int amount)
{
    curIndent-=amount;
    curIndent = MAX(0,curIndent);
    updateIndent();
}

// Reprint the indent string
void trpgPrintBuffer::updateIndent()
{
    int i;
    for (i=0;i<MIN(199,curIndent);i++)
        indentStr[i] = ' ';
    indentStr[i] = 0;
}

// Constructors for File Print Buffer

trpgFilePrintBuffer::trpgFilePrintBuffer(FILE *inFp)
{
    isMine = false;
    fp = inFp;
    valid = true;
}

trpgFilePrintBuffer::trpgFilePrintBuffer(char *file)
{
    isMine = true;
    fp = osgDB::fopen(file,"w");
    valid = fp != NULL;
}

// Destructor for File Print Buffer

trpgFilePrintBuffer::~trpgFilePrintBuffer()
{
    if (isMine && fp)
        fclose(fp);
    fp = NULL;
    valid = false;
}

// Print out a line of text

bool trpgFilePrintBuffer::prnLine(const char *str)
{
    if (!fp)
        return false;

    if (str)
    {
        fprintf(fp,"%s",indentStr);
        fprintf(fp,"%s",str);
        fprintf(fp,"\n");
    } else
        fprintf(fp,"\n");

    return true;
}

/* *************************************
   Print Methods for TerraPage constructs
   All the print methods for every TerraPage object that
   is readable/writeable are here.  These are used for
   debugging.
   *************************************
   */

/* Print out the header information.
 */
bool trpgHeader::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];
    buf.prnLine("----Archive Header----");
    buf.IncreaseIndent();
    sprintf(ls,"verMinor = %d, verMajor = %d",verMinor,verMajor); buf.prnLine(ls);
    if((verMajor >= TRPG_NOMERGE_VERSION_MAJOR) && (verMinor >=TRPG_NOMERGE_VERSION_MINOR))
    {
        sprintf(ls,"isMaster = %s, numRows = %d, numCols = %d",GetIsMaster()?"YES":"NO",rows,cols); buf.prnLine(ls);
    }
    sprintf(ls,"dbVerMinor = %d, dbVerMajor = %d",dbVerMinor,dbVerMajor); buf.prnLine(ls);

    sprintf(ls,"maxGroupID = %d",maxGroupID); buf.prnLine(ls);
    sprintf(ls,"sw = (%f,%f), ne = (%f,%f)",sw.x,sw.y,ne.x,ne.y); buf.prnLine(ls);
    sprintf(ls,"tileType = %d, origin = (%f,%f,%f)",tileType,origin.x,origin.y,origin.z); buf.prnLine(ls);

    sprintf(ls,"numLods = %d",numLods); buf.prnLine(ls);
    buf.IncreaseIndent();
    for (int i=0;i<numLods;i++)
    {
        sprintf(ls,"tileSize = (%f,%f), lodSizes = (%d,%d), lodRanges = %f",tileSize[i].x,tileSize[i].y,lodSizes[i].x,lodSizes[i].y,lodRanges[i]); buf.prnLine(ls);
    }
    buf.DecreaseIndent(2);
    buf.prnLine("");

    return true;
}

/* Print out texture environment information
 */
bool trpgTextureEnv::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];
    buf.prnLine("----Texture Environment----");
    buf.IncreaseIndent();
    sprintf(ls,"envMode = %d",envMode); buf.prnLine(ls);
    sprintf(ls,"minFilter = %d, magFilter = %d",minFilter,magFilter);  buf.prnLine(ls);
    sprintf(ls,"wrapS = %d, wrapT = %d",wrapS,wrapT);  buf.prnLine(ls);
    buf.DecreaseIndent();
    buf.prnLine("");

    return true;
}

/* Print out the material information
 */
bool trpgMaterial::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];
    buf.prnLine("----Material----");
    buf.IncreaseIndent();
    sprintf(ls,"isBumpMap = %d",(int)isBump); buf.prnLine(ls);
    sprintf(ls,"color = (%f,%f,%f)",color.red,color.green,color.blue); buf.prnLine(ls);
    sprintf(ls,"ambient = (%f,%f,%f)",ambient.red,ambient.green,ambient.blue); buf.prnLine(ls);
    sprintf(ls,"diffuse = (%f,%f,%f)",diffuse.red,diffuse.green,diffuse.blue); buf.prnLine(ls);
    sprintf(ls,"specular = (%f,%f,%f)",specular.red,specular.green,specular.blue); buf.prnLine(ls);
    sprintf(ls,"emission = (%f,%f,%f)",emission.red,emission.green,emission.blue); buf.prnLine(ls);
    sprintf(ls,"shininess = %f, shadeModel = %d",shininess,shadeModel); buf.prnLine(ls);
    sprintf(ls,"pointSize = %f, lineWidth = %f",pointSize,lineWidth); buf.prnLine(ls);
    sprintf(ls,"cullMode = %d, alphaFunc = %d",cullMode,alphaFunc); buf.prnLine(ls);
    sprintf(ls,"alpha = %f, alphaRef = %f",alpha,alphaRef); buf.prnLine(ls);
    sprintf(ls,"autoNormal = %d",autoNormal); buf.prnLine(ls);
    sprintf(ls,"fid = %d, smc = %d, stp = %d, swc = %d",attrSet.fid,attrSet.smc,attrSet.stp,attrSet.swc);  buf.prnLine(ls);
    sprintf(ls,"numTile = %d",numTile); buf.prnLine(ls);
    sprintf(ls,"numTex = %d",numTex); buf.prnLine(ls);
    buf.IncreaseIndent();
    for (int i=0;i<numTex;i++)
    {
        sprintf(ls,"texID[%d] = %d",i,texids[i]); buf.prnLine(ls);
        buf.IncreaseIndent();
        texEnvs[i].Print(buf);
        buf.DecreaseIndent();
    }
    buf.DecreaseIndent(2);

    buf.prnLine();

    return true;
}

/* Print out the material tables.
 */
bool trpgMatTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];
    buf.prnLine("----Material Table----");
    buf.IncreaseIndent();
    sprintf(ls,"numTable = %d",numTable); buf.prnLine(ls);
    sprintf(ls,"numMat = %d",numMat);  buf.prnLine(ls);
    buf.IncreaseIndent();
    MaterialMapType::const_iterator itr = materialMap.begin();
    for (  ; itr != materialMap.end( ); itr++)
    {
        const trpgMaterial *mat;
        sprintf(ls,"Material %d",itr->first);  buf.prnLine(ls);
        mat = (const_cast<trpgMatTable *>(this))->GetMaterialRef(0,itr->first);
        if(!mat)
        {
            sprintf(ls,"Error: Unable to load material!");  buf.prnLine(ls);
        }
        else
            mat->Print(buf);
    }
    buf.DecreaseIndent(2);

    return true;
}

/* Print out texture.
 */
bool trpgTexture::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Texture----");
    buf.IncreaseIndent();
    sprintf(ls,"mode = %d, type = %d",mode,type);  buf.prnLine(ls);
    sprintf(ls,"Name = %s",name); buf.prnLine(ls);
    sprintf(ls,"useCount = %d",useCount);  buf.prnLine(ls);
    sprintf(ls,"sizeX = %d, sizeY = %d, sizeZ = %d",sizeX,sizeY,numLayer);  buf.prnLine(ls);
//    sprintf(ls,"sensor band organization = %d",org); buf.prnLine(ls); // does this need to be added?
    sprintf(ls,"ismipmap = %d",isMipmap);  buf.prnLine(ls);
    sprintf(ls,"addr.file = %d, addr.offset = %d",addr.file,addr.offset);  buf.prnLine(ls);
    sprintf(ls,"addr.col = %d, addr.row = %d",addr.col,addr.row);  buf.prnLine(ls);
    buf.DecreaseIndent();

    buf.prnLine();

    return true;
}

/* Print out texture table
 */
bool trpgTexTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Texture Table----");
    buf.IncreaseIndent();
    TextureMapType::const_iterator itr = textureMap.begin();
    for (  ; itr != textureMap.end( ); itr++)
    {
        sprintf(ls,"Texture %d",itr->first); buf.prnLine(ls);
        itr->second.Print(buf);
    }
    buf.DecreaseIndent();

    buf.prnLine();

    return true;
}

/* Print out model table
 */
bool trpgModelTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Model Table----");
    buf.IncreaseIndent();
    ModelMapType::const_iterator itr = modelsMap.begin();
    for (  ; itr != modelsMap.end( ); itr++)
    {
        sprintf(ls,"Model %d",itr->first);  buf.prnLine(ls);
        itr->second.Print(buf);
    }
    buf.DecreaseIndent();

    buf.prnLine();

    return true;
}

/* Print out a model
 */
bool trpgModel::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Model----");
    buf.IncreaseIndent();
    sprintf(ls,"type = %d",type); buf.prnLine(ls);
    if (name)
    {
        sprintf(ls,"name = %s",name); buf.prnLine(ls);
    }
    sprintf(ls,"diskRef = %d",(int)diskRef), buf.prnLine(ls);
    sprintf(ls,"useCount = %d",useCount); buf.prnLine(ls);

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print out a tile header
 */
bool trpgTileHeader::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Tile Header----");
    buf.IncreaseIndent();

    sprintf(ls,"matList size = %d",static_cast<int>(matList.size())); buf.prnLine(ls);
    buf.IncreaseIndent();
    unsigned int i;
    for (i=0;i<matList.size();i++)
    {
        sprintf(ls,"matList[%d] = %d",i,matList[i]); buf.prnLine(ls);
    }
    buf.DecreaseIndent();

    sprintf(ls,"modelList size = %d",static_cast<int>(modelList.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    for (i=0;i<modelList.size();i++)
    {
        sprintf(ls,"modelList[%d] = %d",i,modelList[i]); buf.prnLine(ls);
    }
    buf.DecreaseIndent();

    sprintf(ls,"local material list size = %d",static_cast<int>(locMats.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    for (i=0;i<locMats.size();i++)
        locMats[i].Print(buf);
    buf.DecreaseIndent();

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print out color info
 */
bool trpgColorInfo::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Color Info----");
    buf.IncreaseIndent();

    sprintf(ls,"type = %d, bind = %d",type,bind);  buf.prnLine(ls);
    sprintf(ls,"colorData size = %d",static_cast<int>(data.size()));
    buf.IncreaseIndent();
    for (unsigned int i=0;i<data.size();i++)
    {
        sprintf(ls,"color[%d] = (%f,%f,%f)",i,data[i].red,data[i].blue,data[i].green); buf.prnLine(ls);
    }
    buf.DecreaseIndent(2);
    buf.prnLine();

    return true;
}

/* Print out tex data info
 */
bool trpgTexData::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Tex Data----");
    buf.IncreaseIndent();

    sprintf(ls,"bind = %d",bind);  buf.prnLine(ls);
    if (floatData.size())
    {
        sprintf(ls,"tex coords (float) = %d",static_cast<int>(floatData.size())); buf.prnLine(ls);
        buf.IncreaseIndent();
        for (unsigned int i=0;i<floatData.size()/2;i++)
        {
            sprintf(ls,"tex coord[%d] = (%f,%f)",i,floatData[i*2+0],floatData[i*2+1]);  buf.prnLine(ls);
        }
        buf.DecreaseIndent();
    }
    else
    {
        if (doubleData.size())
        {
            sprintf(ls,"tex coords (double) = %d",static_cast<int>(doubleData.size()));
            buf.IncreaseIndent();
            for (unsigned int i=0;i<doubleData.size()/2;i++)
            {
                sprintf(ls,"tex coord[%d] = (%f,%f)",i,doubleData[i*2+0],doubleData[i*2+1]),  buf.prnLine(ls);
            }
            buf.DecreaseIndent();
        }
    }

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print out geometry data
 */
bool trpgGeometry::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Geometry Node----");
    buf.IncreaseIndent();

    sprintf(ls,"Material size = %d",static_cast<int>(materials.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    ls[0] = 0;
    unsigned int i;
    for (i=0;i<materials.size();i++)
    {
        char locStr[100];
        sprintf(locStr,"%d ",materials[i]);
        strcat(ls,locStr);
    }
    buf.prnLine(ls);
    buf.DecreaseIndent();

    sprintf(ls,"primType = %d, numPrim = %d",primType,numPrim);  buf.prnLine(ls);
    sprintf(ls,"primLength size = %d",static_cast<int>(primLength.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    ls[0] = 0;
    for (i=0;i<primLength.size();i++)
    {
        char locStr[100];
        sprintf(locStr,"%d ",primLength[i]);
        strcat(ls,locStr);
    }
    buf.prnLine(ls);
    buf.DecreaseIndent();

    if (vertDataFloat.size())
    {
        sprintf(ls,"vert data (float) length = %d",static_cast<int>(vertDataFloat.size()));
        buf.prnLine(ls);
        buf.IncreaseIndent();
        for (i=0;i<vertDataFloat.size()/3;i++)
        {
            sprintf(ls,"(%f, %f, %f)",vertDataFloat[3*i],vertDataFloat[3*i+1],vertDataFloat[3*i+2]);
            buf.prnLine(ls);
        }
        buf.DecreaseIndent();
    }
    else
    {
        if (vertDataDouble.size())
        {
            sprintf(ls,"vert data (double) length = %d",static_cast<int>(vertDataDouble.size()));
            buf.prnLine(ls);
            buf.IncreaseIndent();
            for (i=0;i<vertDataDouble.size()/3;i++)
            {
                sprintf(ls,"(%f, %f, %f)",vertDataDouble[3*i],vertDataDouble[3*i+1],vertDataDouble[3*i+2]);
                buf.prnLine(ls);
            }
            buf.DecreaseIndent();
        }
    }

    sprintf(ls,"normBind = %d",normBind);  buf.prnLine(ls);

    if (normDataFloat.size())
    {
        sprintf(ls,"norm data (float) length = %d",static_cast<int>(normDataFloat.size()));
        buf.prnLine(ls);
        buf.IncreaseIndent();
        for (i=0;i<normDataFloat.size()/3;i++)
        {
            sprintf(ls,"(%f, %f, %f)",normDataFloat[3*i],normDataFloat[3*i+1],normDataFloat[3*i+2]);
            buf.prnLine(ls);
        }
        buf.DecreaseIndent();
    }
    else
    {
        if (normDataDouble.size())
        {
            sprintf(ls,"norm data (double) length = %d",static_cast<int>(normDataDouble.size()));
            buf.prnLine(ls);
            buf.IncreaseIndent();
            for (i=0;i<normDataDouble.size()/3;i++)
            {
                sprintf(ls,"(%f, %f, %f)",normDataDouble[3*i],normDataDouble[3*i+1],normDataDouble[3*i+2]);
                buf.prnLine(ls);
            }
            buf.DecreaseIndent();
        }
    }

    sprintf(ls,"color info size = %d",static_cast<int>(colors.size())); buf.prnLine(ls);
    buf.IncreaseIndent();
    for (i=0;i<colors.size();i++)
    {
        colors[i].Print(buf);
    }
    buf.DecreaseIndent();

    sprintf(ls,"tex data size = %d",static_cast<int>(texData.size()));
    buf.IncreaseIndent();
    for (i=0;i<texData.size();i++)
    {
        texData[i].Print(buf);
    }
    buf.DecreaseIndent();

    // Note: Do edge flags

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print group info
 */
bool trpgGroup::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Group Node----");
    buf.IncreaseIndent();
    sprintf(ls,"id = %d, numChild = %d",id,numChild);  buf.prnLine(ls);
    sprintf(ls,"name = %s", name ? name : "noname" );    buf.prnLine(ls);


    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print Attach info
 */
bool trpgAttach::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Attach Node----");
    buf.IncreaseIndent();
    sprintf(ls,"id = %d, parentID = %d, childPos = %d",id,parentID,childPos);  buf.prnLine(ls);
    sprintf(ls,"name = %s", name ? name : "noname" );    buf.prnLine(ls);

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print ChildRef info
 */
bool trpgChildRef::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----RefChild Node----");
    buf.IncreaseIndent();
    sprintf(ls,"lod = %d, x = %d, y = %d", lod, x, y);    buf.prnLine(ls);
    sprintf(ls,"file = %d, offset = %d", addr.file, addr.offset);    buf.prnLine(ls);
    sprintf(ls,"zmin = %f, zmax = %f", zmin, zmax);    buf.prnLine(ls);

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print billboard info
 */
bool trpgBillboard::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Billboard Node----");
    buf.IncreaseIndent();
    sprintf(ls,"id = %d,  type = %d, mode = %d",id,type,mode);  buf.prnLine(ls);
    sprintf(ls,"center = (%f,%f,%f)",center.x,center.y,center.z);  buf.prnLine(ls);
    sprintf(ls,"axis = (%f,%f,%f)",axis.x,axis.y,axis.z);  buf.prnLine(ls);
    sprintf(ls,"name = %s", name ? name : "noname" );    buf.prnLine(ls);

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print LOD info
 */
bool trpgLod::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----LOD Node----");
    buf.IncreaseIndent();
    sprintf(ls,"id = %d",id);  buf.prnLine(ls);
    sprintf(ls,"numRange (hint) = %d",numRange);  buf.prnLine(ls);
    sprintf(ls,"switchIn = %f, switchOut = %f, width = %f",switchIn,switchOut,width);  buf.prnLine(ls);
    sprintf(ls,"center = (%f,%f,%f)",center.x,center.y,center.z);
    sprintf(ls,"name = %s", name ? name : "noname" );    buf.prnLine(ls);
    sprintf(ls,"rangeIndex = %d",rangeIndex);  buf.prnLine(ls);

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print Layer info
 */
bool trpgLayer::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Layer Node----");
    buf.IncreaseIndent();
    sprintf(ls,"id = %d",id);  buf.prnLine(ls);
    sprintf(ls,"name = %s", name ? name : "noname" );    buf.prnLine(ls);

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print Transform
 */
bool trpgTransform::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Transform Node----");
    buf.IncreaseIndent();
    sprintf(ls,"id = %d",id);  buf.prnLine(ls);
    buf.IncreaseIndent();
    for (int i=0;i<4;i++)
    {
        sprintf(ls,"%f %f %f %f",m[i][0],m[i][1],m[i][2],m[i][3]);
        buf.prnLine(ls);
    }
    sprintf(ls,"name = %s", name ? name : "noname" );    buf.prnLine(ls);

    buf.DecreaseIndent(2);
    buf.prnLine();

    return true;
}

/* Print Model Reference
 */
bool trpgModelRef::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Model Reference Node----");
    buf.IncreaseIndent();
    sprintf(ls,"modelRef = %d",modelRef);  buf.prnLine(ls);
    buf.IncreaseIndent();
    for (int i=0;i<4;i++)
    {
        sprintf(ls,"%f %f %f %f",m[i][0],m[i][1],m[i][2],m[i][3]);
        buf.prnLine(ls);
    }

    buf.DecreaseIndent(2);
    buf.prnLine();

    return true;
}

/* Tile Table Print
 */
bool trpgTileTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Tile Table----");
    buf.IncreaseIndent();
    switch(mode)
    {
    case Local:
        sprintf(ls,"mode = %d(Local)",mode);  buf.prnLine(ls);
        break;
    case External:
        sprintf(ls,"mode = %d(External)",mode);  buf.prnLine(ls);
        break;
    case ExternalSaved:
        sprintf(ls,"mode = %d(ExternalSaved)",mode);  buf.prnLine(ls);
        break;
    default:
        sprintf(ls,"mode = %d",mode);  buf.prnLine(ls);
    }

    sprintf(ls,"numLod = %d",static_cast<int>(lodInfo.size()));  buf.prnLine(ls);
    for (unsigned int i=0;i<lodInfo.size();i++)
    {
        const LodInfo &li = lodInfo[i];
        sprintf(ls,"LOD %d, numX = %d, numY = %d",i,li.numX,li.numY);  buf.prnLine(ls);
        buf.prnLine("File ID, Offset, Zmin, Zmax");
        buf.IncreaseIndent();
        for (unsigned int j=0;j<li.addr.size();j++)
        {
            sprintf(ls,"%d %d %f %f",li.addr[j].file,li.addr[j].offset,li.elev_min[j],li.elev_max[j]);  buf.prnLine(ls);
        }
        buf.DecreaseIndent();
    }
    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Local Material Print
 */
bool trpgLocalMaterial::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Local Material Definition----");
    buf.IncreaseIndent();
    sprintf(ls,"baseMat = %d",baseMat);  buf.prnLine(ls);
    sprintf(ls,"(sx,sy) -> (ex,ey) = (%d,%d) -> (%d,%d)",sx,sy,ex,ey); buf.prnLine(ls);
    sprintf(ls,"dest (width,height) = (%d,%d)",destWidth,destHeight); buf.prnLine(ls);
    for (unsigned int i=0;i<addr.size();i++)
    {
        sprintf(ls,"addr (file,offset) = (%d,%d)",addr[i].file,addr[i].offset); buf.prnLine(ls);
    }
    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Light Attribute Print
 */
bool trpgLightAttr::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    const char* strType[] = {"Raster","Calligraphic","RASCAL"};
    const char* strDirect[] = {"Omnidirectional","Bidirectional","Unidirectional"};
    const char* strQuality[] = {"Off","Low","Medium","High","Undefined"};

    buf.prnLine("----Light Attribute----");
    buf.IncreaseIndent();
    sprintf(ls,"type = %s",strType[(int)(data.type)]);                buf.prnLine(ls);
    sprintf(ls,"directionality = %s",strDirect[(int)(data.directionality)]);    buf.prnLine(ls);
    sprintf(ls,"front color (RGB) = %.2f, %.2f, %.2f",
            data.frontColor.red, data.frontColor.green,data.frontColor.blue );    buf.prnLine(ls);
    sprintf(ls,"front intensity = %.2f", data.frontIntensity );        buf.prnLine(ls);
    sprintf(ls,"back color (RGB) = %.2f, %.2f, %.2f",
            data.backColor.red, data.backColor.green,data.backColor.blue );    buf.prnLine(ls);
    sprintf(ls,"back intensity = %.2f", data.backIntensity );            buf.prnLine(ls);
    sprintf(ls,"normal (xyz) = %.2f,%.2f,%.2f",
            data.normal.x,data.normal.y,data.normal.z );            buf.prnLine(ls);
    sprintf(ls,"smc = %d",data.smc);                        buf.prnLine(ls);
    sprintf(ls,"fid = %d",data.fid);                        buf.prnLine(ls);
    sprintf(ls,"visible at DAY = %s",
            (data.flags & trpgLightAttr::trpg_Day ? "yes" : "no") );        buf.prnLine(ls);
    sprintf(ls,"visible at DUSK = %s",
            (data.flags & trpgLightAttr::trpg_Dusk ? "yes" : "no") );        buf.prnLine(ls);
    sprintf(ls,"visible at NIGHT = %s",
            (data.flags & trpgLightAttr::trpg_Night ? "yes" : "no") );        buf.prnLine(ls);
    sprintf(ls,"enable directionality = %s",
            (data.flags & trpgLightAttr::trpg_Directional ? "yes" : "no" ));    buf.prnLine(ls);
    sprintf(ls,"enable back color = %s",
            (data.flags & trpgLightAttr::trpg_BackColor ? "yes" : "no" ));    buf.prnLine(ls);
    sprintf(ls,"horizontal lobe angle = %.2f",data.horizontalLobeAngle);    buf.prnLine(ls);
    sprintf(ls,"vertical lobe angle = %.2f",data.verticalLobeAngle);        buf.prnLine(ls);
    sprintf(ls,"lobe roll angle = %.2f",data.lobeRollAngle);            buf.prnLine(ls);
    sprintf(ls,"lobe falloff = %.2f",data.lobeFalloff);            buf.prnLine(ls);
    sprintf(ls,"ambient intensity = %.2f",data.ambientIntensity);        buf.prnLine(ls);
    sprintf(ls,"reflective only = %s",
            (data.flags & trpgLightAttr::trpg_Reflective ? "yes" : "no") );    buf.prnLine(ls);
    sprintf(ls,"quality = %s", strQuality[(int)(data.quality)]);        buf.prnLine(ls);
    sprintf(ls,"significance for RASCAL lights = %.2f",
            data.rascalSignificance );                        buf.prnLine(ls);
    sprintf(ls,"calligraphic draw order = %d",
            data.calligraphicAttr.drawOrder );                    buf.prnLine(ls);
    sprintf(ls,"calligraphic lights maximum defocus = %f",
            data.calligraphicAttr.maxDefocus );                    buf.prnLine(ls);
    sprintf(ls,"calligraphic lights minimum defocus = %f",
            data.calligraphicAttr.minDefocus );                    buf.prnLine(ls);
    sprintf(ls,"randomize intensity = %s",
            strQuality[(int)(data.randomIntensity)]);                buf.prnLine(ls);
    sprintf(ls,"performer perspective mode = %s",
            (data.flags & trpgLightAttr::trpg_Perspective ? "yes" : "no" ) );    buf.prnLine(ls);
    sprintf(ls,"performer fade = %s",
            (data.flags & trpgLightAttr::trpg_Fade ? "yes" : "no" ) );        buf.prnLine(ls);
    sprintf(ls,"performer fog punch = %s",
            (data.flags & trpgLightAttr::trpg_FogPunch ? "yes" : "no" ) );    buf.prnLine(ls);
    sprintf(ls,"performer range mode enable Z buffer = %s",
            (data.flags & trpgLightAttr::trpg_ZBuffer ? "yes" : "no" ) );    buf.prnLine(ls);
    sprintf(ls,"performer maximum pixel size = %.2f",
            data.performerAttr.maxPixelSize );                    buf.prnLine(ls);
    sprintf(ls,"performer minimum pixel size = %.2f",
            data.performerAttr.minPixelSize );                    buf.prnLine(ls);
    sprintf(ls,"performer actual size = %.2f",
            data.performerAttr.actualSize );                    buf.prnLine(ls);
    sprintf(ls,"performer transparent pixel size = %.2f",
            data.performerAttr.transparentPixelSize );                buf.prnLine(ls);
    sprintf(ls,"performer transparent falloff exponent = %.2f",
            data.performerAttr.transparentFallofExp );                buf.prnLine(ls);
    sprintf(ls,"performer transparent scale = %.2f",
            data.performerAttr.transparentScale );                buf.prnLine(ls);
    sprintf(ls,"performer transparent clamp = %.2f",
            data.performerAttr.transparentClamp );                buf.prnLine(ls);
    sprintf(ls,"performer fog scale = %.2f",
            data.performerAttr.fogScale );                    buf.prnLine(ls);
    sprintf(ls,"animation period = %.2f",data.animationAttr.period);        buf.prnLine(ls);
    sprintf(ls,"animation phase delay = %.2f",
            data.animationAttr.phaseDelay );                    buf.prnLine(ls);
    sprintf(ls,"animation time on = %.2f",data.animationAttr.timeOn);        buf.prnLine(ls);
    sprintf(ls,"animation vector (ijk) = %.2f, %.2f, %.2f",
            data.animationAttr.vector.x,data.animationAttr.vector.y,
            data.animationAttr.vector.z);                    buf.prnLine(ls);
    sprintf(ls,"animation - flashing = %s",
            (data.flags & trpgLightAttr::trpg_Flashing ? "yes" : "no" ));    buf.prnLine(ls);
    sprintf(ls,"animation - rotating = %s",
            (data.flags & trpgLightAttr::trpg_Rotating ? "yes" : "no" ));    buf.prnLine(ls);
    sprintf(ls,"animation - counter clockwise = %s",
            (data.flags & trpgLightAttr::trpg_ClockWise ? "yes" : "no" ));    buf.prnLine(ls);
    if (data.commentStr)
    {
        sprintf(ls,"comment = %s",data.commentStr);  buf.prnLine(ls);
    }

    buf.DecreaseIndent();
    buf.prnLine();

    return true;
}

/* Print out light table
 */
bool trpgLightTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Light Table----");
    buf.IncreaseIndent();
    LightMapType::const_iterator itr = lightMap.begin();
    for (  ; itr != lightMap.end( ); itr++)
    {
        sprintf(ls,"Light %d",itr->first); buf.prnLine(ls);
        itr->second.Print(buf);
    }
    buf.DecreaseIndent();

    buf.prnLine();

    return true;
}

/* Print out a light node
 */

bool trpgLight::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Light----");
    buf.IncreaseIndent();

    sprintf(ls,"Light Index = %d",index);                    buf.prnLine(ls);
    sprintf(ls,"# Light Locations = %d",static_cast<int>(lightPoints.size()) ); buf.prnLine(ls);

    buf.DecreaseIndent();

    buf.prnLine();

    return true;
}

/* Print out a single range
 */

bool trpgRange::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.IncreaseIndent();

    sprintf(ls,"category = %s, subCategory = %s",category,subCategory);  buf.prnLine(ls);
    sprintf(ls,"inLod = %f, outLod = %f",inLod,outLod);  buf.prnLine(ls);
    sprintf(ls,"priority = %d",priority);  buf.prnLine(ls);

    buf.DecreaseIndent();

    return true;
}

/* Print out the whole range table
 */

bool trpgRangeTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Range Table----");
    buf.IncreaseIndent();
    RangeMapType::const_iterator itr = rangeMap.begin();
    for (int i = 0; itr != rangeMap.end( ); itr++, i++)
    {
        sprintf(ls,"----Range %d----",i);  buf.prnLine(ls);
        itr->second.Print(buf);
    }

    buf.DecreaseIndent();

    return true;
}

// Print out a label

bool trpgLabel::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Label----");
    buf.IncreaseIndent();
    sprintf(ls,"property ID = %d",propertyId);  buf.prnLine(ls);
    sprintf(ls,"text = %s",text.c_str());  buf.prnLine(ls);
    sprintf(ls,"alignment = %d",alignment);  buf.prnLine(ls);
    sprintf(ls,"tabSize = %d",tabSize);  buf.prnLine(ls);
    sprintf(ls,"scale = %f",scale);  buf.prnLine(ls);
    sprintf(ls,"thickness = %f",thickness);  buf.prnLine(ls);
    sprintf(ls,"desc = %s",desc.c_str());  buf.prnLine(ls);
    sprintf(ls,"url = %s",url.c_str());  buf.prnLine(ls);
    sprintf(ls,"location: (%f %f %f)",location.x,location.y,location.z); buf.prnLine(ls);
    sprintf(ls,"%d support points",static_cast<int>(supports.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    for (unsigned int i=0;i<supports.size();i++)
    {
        sprintf(ls,"%f %f %f",supports[i].x,supports[i].y,supports[i].z); buf.prnLine(ls);
    }
    buf.DecreaseIndent();
    buf.prnLine();
    buf.DecreaseIndent();

    return true;
}

// Print out a text style

bool trpgTextStyle::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Text Style----");
    buf.IncreaseIndent();
    sprintf(ls,"font = %s",font.c_str());  buf.prnLine(ls);
    sprintf(ls,"bold = %d, italic = %d, underline = %d",bold,italic,underline);  buf.prnLine(ls);
    sprintf(ls,"characterSize = %f",characterSize);  buf.prnLine(ls);
    sprintf(ls,"material ID = %d",matId);  buf.prnLine(ls);
    buf.DecreaseIndent();

    return true;
}

// Print out the text style table

bool trpgTextStyleTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine();
    buf.prnLine("----Text Style Table----");
    buf.IncreaseIndent();
    sprintf(ls,"numStyle = %d",static_cast<int>(styleMap.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    StyleMapType::const_iterator itr = styleMap.begin();
    for (int i = 0; itr != styleMap.end( ); itr++, i++)
    {
        sprintf(ls,"Style %d",i);  buf.prnLine(ls);
        itr->second.Print(buf);
    }
    buf.DecreaseIndent();
    buf.DecreaseIndent();

    return true;
}

// Print out a support style

bool trpgSupportStyle::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Support Style----");
    buf.IncreaseIndent();
    sprintf(ls,"Support Type = %d",type);  buf.prnLine(ls);
    sprintf(ls,"material ID = %d",matId);  buf.prnLine(ls);
    buf.DecreaseIndent();

    return true;
}

// Print out the support style table

bool trpgSupportStyleTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine();
    buf.prnLine("----Support Style Table----");
    buf.IncreaseIndent();
    sprintf(ls,"numStyle = %d",static_cast<int>(supportStyleMap.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    SupportStyleMapType::const_iterator itr = supportStyleMap.begin();
    for (int i = 0; itr != supportStyleMap.end( ); itr++, i++)
    {
        sprintf(ls,"Style %d",i);  buf.prnLine(ls);
        itr->second.Print(buf);
    }
    buf.DecreaseIndent();
    buf.DecreaseIndent();

    return true;
}

// Print out a label property

bool trpgLabelProperty::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine("----Label Property----");
    buf.IncreaseIndent();
    sprintf(ls,"font ID = %d",  fontId);  buf.prnLine(ls);
    sprintf(ls,"support ID = %d", supportId);  buf.prnLine(ls);
    sprintf(ls,"label type = %d", type);  buf.prnLine(ls);

    buf.DecreaseIndent();

    return true;
}

// Print out the text style table

bool trpgLabelPropertyTable::Print(trpgPrintBuffer &buf) const
{
    char ls[1024];

    buf.prnLine();
    buf.prnLine("----Label Property Table----");
    buf.IncreaseIndent();
    sprintf(ls,"numProperty = %d",static_cast<int>(labelPropertyMap.size()));  buf.prnLine(ls);
    buf.IncreaseIndent();
    LabelPropertyMapType::const_iterator itr = labelPropertyMap.begin();
    for (int i = 0; itr != labelPropertyMap.end( ); itr++, i++)
    {
        sprintf(ls,"Property %d",i);  buf.prnLine(ls);
        itr->second.Print(buf);
    }
    buf.DecreaseIndent();
    buf.DecreaseIndent();

    return true;
}
