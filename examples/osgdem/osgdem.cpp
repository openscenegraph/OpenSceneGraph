/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/State>
#include <osg/ShapeDrawable>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ImageOptions>
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TangentSpaceGenerator>

#include <osgFX/BumpMapping>

#include <osgProducer/Viewer>


float VERTICAL_SIZE = 0.0005;

class GraphicsContext {
    public:
        GraphicsContext()
        {
            rs = new Producer::RenderSurface;
            rs->setWindowRectangle(0,0,1,1);
            rs->useBorder(false);
            rs->useConfigEventThread(false);
            rs->realize();
            std::cout<<"Realized window"<<std::endl;
        }

        virtual ~GraphicsContext()
        {
        }
        
    private:
        Producer::ref_ptr<Producer::RenderSurface> rs;
};

osg::Image* createNormalMap(osg::HeightField* grid)
{
    osg::Image* image = new osg::Image;
    image->allocateImage(grid->getNumColumns(),grid->getNumRows(),1,GL_RGB,GL_BYTE);

    char* ptr = (char*) image->data();
    for(unsigned int r=0;r<grid->getNumRows();++r)
    {
	for(unsigned int c=0;c<grid->getNumColumns();++c)
	{
	    osg::Vec3 normal = grid->getNormal(c,r);
            (*ptr++) = (char)((normal.x()+1.0)*0.5*255);
            (*ptr++) = (char)((normal.y()+1.0)*0.5*255);
            (*ptr++) = (char)((normal.z()+1.0)*0.5*255);
	}
    }
    
    return image;

}


template< typename T>
osg::HeightField* _createHeightField(osg::Image* image, float zScale)
{
    unsigned char* ptr = image->data();
    unsigned int rowSizeInBytes = image->getRowSizeInBytes();
    unsigned int pixelSizeInBits = image->getPixelSizeInBits();
    unsigned int pixelSizeInBytes = pixelSizeInBits/8;
    
    unsigned int numColumns = image->s();
    unsigned int numRows = image->t();
    
    osg::HeightField* grid = new osg::HeightField;
    grid->allocateGrid(numColumns,numRows);
    
    for(unsigned int r=0;r<numRows;++r)
    {
        unsigned char* ptr_in_row = ptr;
	for(unsigned int c=0;c<numColumns;++c)
	{
	    grid->setHeight(c,r,zScale*(float)(*((T*)ptr_in_row)));
            ptr_in_row += pixelSizeInBytes;
	}
        ptr += rowSizeInBytes;
    }
    
    return grid;
}

osg::HeightField* createHeightField(osg::Image* image, const osg::Vec3& origin, const osg::Vec3& size)
{
    osg::HeightField* grid = new osg::HeightField;
    switch(image->getDataType())
    {
        case(GL_UNSIGNED_BYTE):     grid = _createHeightField<unsigned char>(image,size.z()); break;
        case(GL_BYTE):              grid = _createHeightField<char>(image,size.z()); break;
        case(GL_UNSIGNED_SHORT):    grid = _createHeightField<unsigned short>(image,size.z()); break;
        case(GL_SHORT):             grid = _createHeightField<short>(image,size.z()); break;
        case(GL_UNSIGNED_INT):      grid = _createHeightField<unsigned int>(image,size.z()); break;
        case(GL_INT):               grid = _createHeightField<int>(image,size.z()); break;
        case(GL_FLOAT):             grid = _createHeightField<float>(image,size.z()); break;
    }
    grid->setOrigin(origin);
    grid->setXInterval(size.x()/(float)(image->s()-1));
    grid->setYInterval(size.y()/(float)(image->s()-1));
    
    return grid;
    
}

osg::Node* createTile(osg::HeightField* grid, 
                          unsigned int columnBegin, unsigned int columnEnd,
                          unsigned int rowBegin, unsigned int rowEnd,
                          float xTexCoordBegin, float xTexCoordEnd,
                          float yTexCoordBegin, float yTexCoordEnd,
                          unsigned int targetNumPolygonsPerTile)
{

   
    int numPolys = (columnEnd-columnBegin)*(rowEnd-rowBegin)*2;
    int numColumns, numRows;
    bool oneToOneMapping = numPolys<=targetNumPolygonsPerTile;
    if (oneToOneMapping)
    {
        numColumns = (columnEnd-columnBegin)+1;
        numRows = (rowEnd-rowBegin)+1;
    }
    else
    {
        numColumns = (int)sqrtf((float)targetNumPolygonsPerTile/2.0) + 1;
        numRows = targetNumPolygonsPerTile/(2*(numColumns)-1) + 1;
    }

    bool createSkirt = true;


    int numVerticesInBody = numColumns*numRows;
    int numVerticesInSkirt = createSkirt ? numColumns*2 + numRows*2 - 4 : 0;
    int numVertices = numVerticesInBody+numVerticesInSkirt;
    
    osg::Vec3 skirtVector(0.0f,0.0f,-0.003f);


    osg::Geometry* geometry = new osg::Geometry;

    osg::Vec3Array& v = *(new osg::Vec3Array(numVertices));
    osg::Vec2Array& t = *(new osg::Vec2Array(numVertices));
    osg::UByte4Array& color = *(new osg::UByte4Array(1));

    color[0].set(255,255,255,255);


    int vi=0;
    int r,c;
    if (!oneToOneMapping)
    {
        for(r=0;r<numRows;++r)
        {
            unsigned int row = rowBegin + (unsigned int)((float)(rowEnd-rowBegin) * ((float)r/(float)(numRows-1)));
	    for(c=0;c<numColumns;++c)
	    {
                unsigned int col = columnBegin + (unsigned int)((float)(columnEnd-columnBegin) * ((float)c/(float)(numColumns-1)));
	        v[vi] = grid->getOrigin()+osg::Vec3(grid->getXInterval()*(float)col,
                                              grid->getYInterval()*(float)row,
                                              grid->getHeight(col,row));  
	        t[vi].x() = xTexCoordBegin + (xTexCoordEnd-xTexCoordBegin)*(float)(col-columnBegin)/(float)(columnEnd-columnBegin);
	        t[vi].y() = yTexCoordBegin + (yTexCoordEnd-yTexCoordBegin)*(float)(row-rowBegin)/(float)(rowEnd-rowBegin);

                ++vi;
	    }
        }
    }
    else
    {
        for(r=rowBegin;r<=rowEnd;++r)
        {
	    for(c=columnBegin;c<=columnEnd;++c)
	    {
	        v[vi] = grid->getOrigin()+osg::Vec3(grid->getXInterval()*c,
                                              grid->getYInterval()*r,
                                              grid->getHeight(c,r));  
	        t[vi].x() = xTexCoordBegin + (xTexCoordEnd-xTexCoordBegin)*(float)(c-columnBegin)/(float)(columnEnd-columnBegin);
	        t[vi].y() = yTexCoordBegin + (yTexCoordEnd-yTexCoordBegin)*(float)(r-rowBegin)/(float)(rowEnd-rowBegin);

                ++vi;
	    }
        }
    }
    

     
    geometry->setVertexArray(&v);
    geometry->setColorArray(&color);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->setTexCoordArray(0,&t);
    geometry->setTexCoordArray(1,&t);

    bool pickOutDiagonals = true;
    if (pickOutDiagonals)
    {
        osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_TRIANGLES,2*3*(numColumns-1)*(numRows-1)));
        geometry->addPrimitiveSet(&drawElements);
        int ei=0;
        for(r=0;r<numRows-1;++r)
        {
	    for(c=0;c<numColumns-1;++c)
	    {
                unsigned short i00 = (r)*numColumns+c;
                unsigned short i10 = (r)*numColumns+c+1;
                unsigned short i01 = (r+1)*numColumns+c;
                unsigned short i11 = (r+1)*numColumns+c+1;

                float diff_00_11 = fabsf(v[i00].z()-v[i11].z());
                float diff_01_10 = fabsf(v[i01].z()-v[i10].z());
                if (diff_00_11<diff_01_10)
                {
                    // diagonal between 00 and 11
	            drawElements[ei++] = i00;
	            drawElements[ei++] = i10;
	            drawElements[ei++] = i11;

	            drawElements[ei++] = i00;
	            drawElements[ei++] = i11;
	            drawElements[ei++] = i01;
                }
                else
                {
                    // diagonal between 01 and 10
	            drawElements[ei++] = i01;
	            drawElements[ei++] = i00;
	            drawElements[ei++] = i10;

	            drawElements[ei++] = i01;
	            drawElements[ei++] = i10;
	            drawElements[ei++] = i11;
                }
    	    }
        }
    }
    else
    {
        for(r=0;r<numRows-1;++r)
        {
            osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numColumns));
            geometry->addPrimitiveSet(&drawElements);
            int ei=0;
	    for(c=0;c<numColumns;++c)
	    {
	        drawElements[ei++] = (r+1)*numColumns+c;
	        drawElements[ei++] = (r)*numColumns+c;
	    }
        }
    }
    
    if (numVerticesInSkirt>0)
    {
        osg::DrawElementsUShort& skirtDrawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numVerticesInSkirt+2));
        geometry->addPrimitiveSet(&skirtDrawElements);
        int ei=0;
        int firstSkirtVertexIndex = vi;
        // create bottom skirt vertices
        r=0;
        for(c=0;c<numColumns-1;++c)
        {
	    skirtDrawElements[ei++] = (r)*numColumns+c;
	    skirtDrawElements[ei++] = vi;
            v[vi] = v[(r)*numColumns+c]+skirtVector;
            t[vi++] = t[(r)*numColumns+c];
        }
        // create right skirt vertices
        c=numColumns-1;
        for(r=0;r<numRows-1;++r)
        {
	    skirtDrawElements[ei++] = (r)*numColumns+c;
	    skirtDrawElements[ei++] = vi;
            v[vi] = v[(r)*numColumns+c]+skirtVector;
            t[vi++] = t[(r)*numColumns+c];
        }
        // create top skirt vertices
        r=numRows-1;
        for(c=numColumns-1;c>0;--c)
        {
	    skirtDrawElements[ei++] = (r)*numColumns+c;
	    skirtDrawElements[ei++] = vi;
            v[vi] = v[(r)*numColumns+c]+skirtVector;
            t[vi++] = t[(r)*numColumns+c];
        }
        // create left skirt vertices
        c=0;
        for(r=numRows-1;r>0;--r)
        {
	    skirtDrawElements[ei++] = (r)*numColumns+c;
	    skirtDrawElements[ei++] = vi;
            v[vi] = v[(r)*numColumns+c]+skirtVector;
            t[vi++] = t[(r)*numColumns+c];
        }
        skirtDrawElements[ei++] = 0;
        skirtDrawElements[ei++] = firstSkirtVertexIndex;
    }    
    



    osgUtil::TriStripVisitor tsv;
    tsv.stripify(*geometry);
// 
//     osgUtil::SmoothingVisitor sv;
//     sv.smooth(*geometry);

    osg::Vec4Array& tsgTangentArray = *(new osg::Vec4Array(1));
    osg::Vec4Array& tsgBinormalArray = *(new osg::Vec4Array(1));
    osg::Vec4Array& tsgNormalArray = *(new osg::Vec4Array(1));
    tsgTangentArray[0].set(1.0f,0.0f,0.0f,0.0f);
    tsgBinormalArray[0].set(0.0f,1.0f,0.0f,0.0f);
    tsgNormalArray[0].set(0.0f,0.0f,1.0f,0.0f);
    geometry->setVertexAttribData(6, osg::Geometry::ArrayData(&tsgTangentArray, osg::Geometry::BIND_OVERALL,GL_FALSE));
    geometry->setVertexAttribData(7, osg::Geometry::ArrayData(&tsgBinormalArray, osg::Geometry::BIND_OVERALL,GL_FALSE));
    geometry->setVertexAttribData(15, osg::Geometry::ArrayData(&tsgNormalArray, osg::Geometry::BIND_OVERALL,GL_FALSE));

    //geometry->setUseVertexBufferObjects(true);

    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(false);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);
    
    return geode;
    
}


osg::Node* createQuadTree(osg::HeightField* grid, 
                          unsigned int columnBegin, unsigned int columnEnd,
                          unsigned int rowBegin, unsigned int rowEnd,
                          float xTexCoordBegin, float xTexCoordEnd,
                          float yTexCoordBegin, float yTexCoordEnd,
                          unsigned int targetNumPolygonsPerTile,
			  unsigned int numLevels)
{
    unsigned int numPolys = (columnEnd-columnBegin)*(rowEnd-rowBegin)*2;
    
    
    std::cout << "createQuadTree "<<columnBegin<<","<<columnEnd<<std::endl;
    
    osg::Node* tile = createTile(grid, 
                          columnBegin, columnEnd,
                          rowBegin, rowEnd,
                          xTexCoordBegin, xTexCoordEnd,
                          yTexCoordBegin, yTexCoordEnd,
                          targetNumPolygonsPerTile);
    
    if (numPolys<=targetNumPolygonsPerTile || numLevels==0) 
    {
        return tile;
    }
    else
    {
        float cut_off_distance = tile->getBound().radius()*4.0f;
        float max_visible_distance = 1e7;

        unsigned int columnCenter = (columnBegin + columnEnd)/2;
        unsigned int rowCenter = (rowBegin + rowEnd)/2;
        float xTexCoordCenter = xTexCoordBegin+
                                (xTexCoordEnd-xTexCoordBegin)*((float)(columnCenter-columnBegin)/(float)(columnEnd-columnBegin));
        float yTexCoordCenter = yTexCoordBegin+
                                (yTexCoordEnd-yTexCoordBegin)*((float)(rowCenter-rowBegin)/(float)(rowEnd-rowBegin));
    
        osg::Group* group = new osg::Group;
        group->addChild(createQuadTree(grid, 
                                  columnBegin, columnCenter,
                                  rowBegin, rowCenter,
                                  xTexCoordBegin, xTexCoordCenter,
                                  yTexCoordBegin, yTexCoordCenter,
                                  targetNumPolygonsPerTile,
				  numLevels-1));

        group->addChild(createQuadTree(grid, 
                                  columnCenter, columnEnd,
                                  rowBegin, rowCenter,
                                  xTexCoordCenter, xTexCoordEnd,
                                  yTexCoordBegin, yTexCoordCenter,
                                  targetNumPolygonsPerTile,
				  numLevels-1));

        group->addChild(createQuadTree(grid, 
                                  columnCenter, columnEnd,
                                  rowCenter, rowEnd,
                                  xTexCoordCenter, xTexCoordEnd,
                                  yTexCoordCenter, yTexCoordEnd,
                                  targetNumPolygonsPerTile,
				  numLevels-1));

        group->addChild(createQuadTree(grid, 
                                  columnBegin, columnCenter,
                                  rowCenter, rowEnd,
                                  xTexCoordBegin, xTexCoordCenter,
                                  yTexCoordCenter, yTexCoordEnd,
                                  targetNumPolygonsPerTile,
				  numLevels-1));
                                  
        osg::LOD* lod = new osg::LOD;
        lod->addChild(tile,cut_off_distance,max_visible_distance);
        lod->addChild(group,0.0f,cut_off_distance);
        
        return lod;
                          
    }
}


template< typename T>
void populate_z(osg::Image* image, const osg::Vec3& zAxis,osg::Vec3Array& v)
{
    unsigned char* ptr = image->data();
    unsigned int rowSizeInBytes = image->getRowSizeInBytes();
    unsigned int pixelSizeInBits = image->getPixelSizeInBits();
    unsigned int pixelSizeInBytes = pixelSizeInBits/8;
    
    unsigned int numColumns = image->s();
    unsigned int numRows = image->t();
    for(unsigned int r=0,vi=0;r<numRows;++r)
    {
        unsigned char* ptr_in_row = ptr;
	for(unsigned int c=0;c<numColumns;++c)
	{
	    v[vi++] += zAxis * (float)(*((T*)ptr_in_row));
            ptr_in_row += pixelSizeInBytes;
	}
        ptr += rowSizeInBytes;
    }
}

osg::Node* computeGeometry(osg::Image* image, const osg::Vec3& origin, const osg::Vec3& xAxis, const osg::Vec3& yAxis, const osg::Vec3& zAxis)
{
    if (!image) return 0;

    osgDB::ImageOptions::TexCoordRange* texCoordRange = dynamic_cast<osgDB::ImageOptions::TexCoordRange*>(image->getUserData());

    unsigned int numColumns = image->s();
    unsigned int numRows = image->t();
    unsigned int r;
    unsigned int c;

    osg::Geode* geode = new osg::Geode;

    bool useGeometry = true;
    if (useGeometry)
    {

        osg::Geometry* geometry = new osg::Geometry;

        osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
        //osg::Vec3Array& n = *(new osg::Vec3Array(numColumns*numRows));
        osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
        osg::UByte4Array& color = *(new osg::UByte4Array(1));

        color[0].set(255,255,255,255);


#if 0
        osg::Vec2 tex_orig(0.0f,0.0f);

        float rowTexDelta = 1.0f/(float)(numRows-1);
        float columnTexDelta = 1.0f/(float)(numColumns-1);
#else

        osg::Vec2 tex_orig(origin.x(),origin.y());

        float columnTexDelta = xAxis.length()/(float)(numColumns-1);
        float rowTexDelta = yAxis.length()/(float)(numRows-1);
#endif

        if (texCoordRange)
        {
//             tex_orig.set(texCoordRange->_x,texCoordRange->_y);
//             rowTexDelta = texCoordRange->_h/(float)(numRows-1);
//             columnTexDelta = texCoordRange->_w/(float)(numColumns-1);

            std::cout<<"setting tex values to use texCoordRange"<<std::endl;
            std::cout<<"    tex_orig="<<tex_orig<<std::endl;
            std::cout<<"    rowTexDelta"<<rowTexDelta<<std::endl;
            std::cout<<"    columnTexDelta"<<columnTexDelta<<std::endl;
        }

        osg::Vec2 tex(tex_orig);
        int vi=0;
        osg::Vec3 normal(0.0f,0.0f,1.0f);
        osg::Vec3 pos = origin;
        osg::Vec3 delta_x = xAxis/(double)(numColumns-1);
        osg::Vec3 delta_y = yAxis/(double)(numRows-1);
        for(r=0;r<numRows;++r)
        {
            tex.x() = tex_orig.x();
            osg::Vec3 pos_in_row = pos;
	    for(c=0;c<numColumns;++c)
	    {
	        v[vi] = pos_in_row; // + zAxis * (sin(pos_in_row.x()*osg::PI*2)*cos(pos_in_row.y()*osg::PI*2));
	        //n[vi] = normal;            
	        t[vi].set(tex.x(),tex.y());

                pos_in_row +=delta_x;
                tex.x()+=columnTexDelta;
                ++vi;
	    }
            pos += delta_y;
            tex.y() += rowTexDelta;
        }

        switch(image->getDataType())
        {
            case(GL_UNSIGNED_BYTE):     populate_z<unsigned char>(image,zAxis,v); break;
            case(GL_BYTE):              populate_z<char>(image,zAxis,v); break;
            case(GL_UNSIGNED_SHORT):    populate_z<unsigned short>(image,zAxis,v); break;
            case(GL_SHORT):             populate_z<short>(image,zAxis,v); break;
            case(GL_UNSIGNED_INT):      populate_z<unsigned int>(image,zAxis,v); break;
            case(GL_INT):               populate_z<int>(image,zAxis,v); break;
            case(GL_FLOAT):             populate_z<float>(image,zAxis,v); break;
        }

        geometry->setVertexArray(&v);
        //geometry->setNormalArray(&n);
        //geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geometry->setColorArray(&color);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        geometry->setTexCoordArray(0,&t);
        geometry->setTexCoordArray(1,&t);

        
        for(r=0;r<numRows-1;++r)
        {
            osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numColumns));
            geometry->addPrimitiveSet(&drawElements);
            int ei=0;
	    for(c=0;c<numColumns;++c)
	    {
	        drawElements[ei++] = (r+1)*numColumns+c;
	        drawElements[ei++] = (r)*numColumns+c;
	    }
        }

        osgUtil::TriStripVisitor tsv;
        tsv.stripify(*geometry);
// 
//         osgUtil::SmoothingVisitor sv;
//         sv.smooth(*geometry);

        osg::Vec4Array& tsgTangentArray = *(new osg::Vec4Array(1));
        osg::Vec4Array& tsgBinormalArray = *(new osg::Vec4Array(1));
        osg::Vec4Array& tsgNormalArray = *(new osg::Vec4Array(1));
        tsgTangentArray[0].set(1.0f,0.0f,0.0f,0.0f);
        tsgBinormalArray[0].set(0.0f,1.0f,0.0f,0.0f);
        tsgNormalArray[0].set(0.0f,0.0f,1.0f,0.0f);
        geometry->setVertexAttribData(6, osg::Geometry::ArrayData(&tsgTangentArray, osg::Geometry::BIND_OVERALL,GL_FALSE));
        geometry->setVertexAttribData(7, osg::Geometry::ArrayData(&tsgBinormalArray, osg::Geometry::BIND_OVERALL,GL_FALSE));
        geometry->setVertexAttribData(15, osg::Geometry::ArrayData(&tsgNormalArray, osg::Geometry::BIND_OVERALL,GL_FALSE));

        //geometry->setUseVertexBufferObjects(true);

        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(false);

        geode->addDrawable(geometry);
    }
    else
    {
    
        osg::HeightField* grid = createHeightField(image,origin,osg::Vec3(xAxis.length(),yAxis.length(),zAxis.length()));
        
        geode->addDrawable(new osg::ShapeDrawable(grid));
    }    
    
    return geode;
}

osg::Node* computeGeometry(osg::Image* image, const osg::Vec3& origin, const osg::Vec3& size)
{
    return computeGeometry(image,origin,osg::Vec3(size.x(),0.0f,0.0f),osg::Vec3(0.0f,size.y(),0.0f),osg::Vec3(0.0f,0.0f,size.z()));
}

osg::Node* createTile(const std::string& filename, double x, double y, double w,double h)
{
    
    osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
    options->_sourceImageWindowMode = osgDB::ImageOptions::RATIO_WINDOW;
    options->_sourceRatioWindow.set(x,1-(y+h),w,h);

    options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
    options->_destinationPixelWindow.set(0,0,20,20);

    osgDB::Registry::instance()->setOptions(options.get());
    osg::Image* image = osgDB::readImageFile(filename.c_str());
    if (image)
    {

        return computeGeometry(image,osg::Vec3(x,y,0.0),osg::Vec3(w,h,VERTICAL_SIZE));
        
    }
    else
    {
        return 0;
    }
}

osg::Node* createTileAndRecurse(const std::string& filename, const std::string& basename, const std::string& extension, unsigned int noTilesX, unsigned int noTilesY, double x, double y, double w,double h, unsigned int numLevelsLeft)
{

    osg::Group* group = new osg::Group;
    
    double dx = w / (double) noTilesX;
    double dy = h / (double) noTilesY;
    
    
    if (numLevelsLeft>0)
    {
    
        float cut_off_distance = 4.0f*dy;
        float max_visible_distance = 1e7;
    
        // create current layer, and write to disk.
        unsigned int numTiles = 0;
        double lx = x;
        for(unsigned i=0;i<noTilesX;++i,lx+=dx)
        {
            double ly = y;
            for(unsigned j=0;j<noTilesY;++j,ly+=dy)
            {
                // create name for tile.
                char char_num = 'A'+numTiles;
                std::string lbasename = basename+"_"+char_num;

                // create the subtiles and write out to disk.
                {
                    osg::ref_ptr<osg::Node> node = createTileAndRecurse(filename,lbasename,extension,2,2,lx,ly,dx,dy,numLevelsLeft-1);
                    osgDB::writeNodeFile(*node, lbasename+extension);
                }
                
                // create PagedLOD for tile.
                osg::PagedLOD* pagedlod = new osg::PagedLOD;
                osg::Node* tile = createTile(filename,lx,ly,dx,dy);
                pagedlod->addChild(tile, cut_off_distance,max_visible_distance);
                pagedlod->setRange(1,0.0f,cut_off_distance);
                pagedlod->setFileName(1,lbasename+extension);
                pagedlod->setCenter(tile->getBound().center());

                group->addChild(pagedlod);
                
                // increment number of tiles.
                ++numTiles;
            }

        }
        
    }
    else
    {
        double lx = x;
        for(unsigned i=0;i<noTilesX;++i,lx+=dx)
        {
            double ly = y;
            for(unsigned j=0;j<noTilesY;++j,ly+=dy)
            {
                group->addChild(createTile(filename,lx,ly,dx,dy));
            }
        }
    }


    return group;
    
}

bool createWorld(const std::string& inputFile, const std::string& baseName, const std::string& diffuseTextureName,unsigned int numLevels)
{

    osgDB::ReaderWriter* readerWriter = osgDB::Registry::instance()->getReaderWriterForExtension("gdal");
    if (!readerWriter)
    {
        std::cout<<"Error: GDAL plugin not available, cannot preceed with database creation"<<std::endl;
        return false;
    }


    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // create the world
    

    std::string path = osgDB::getFilePath(baseName);
    std::string base = path.empty()?osgDB::getStrippedName(baseName):
                                    path +'/'+ osgDB::getStrippedName(baseName);
    std::string extension = '.'+osgDB::getLowerCaseFileExtension(baseName);
    
    std::cout << "baseName = "<<baseName<<std::endl;
    std::cout << "base = "<<base<<std::endl;
    std::cout << "extension = "<<extension<<std::endl;




    bool doBumpMapping = true;
    
    int bumpMapSizeWidth = 1024;
    int bumpMapSizeHeight = 1024;
//    int bumpMapSizeWidth = 512;
//    int bumpMapSizeHeight = 512;
    
    if (doBumpMapping)
    {

        osg::ref_ptr<osg::Node> scene;
        
        // = createTileAndRecurse(inputFile, base, /*extension*/".ive", 2,2, 0.0, 0.0, 1.0, 1.0, numLevels);

        // generate normal map
        osg::Image* normalMap = 0;
        {
            osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
            options->_sourceImageWindowMode = osgDB::ImageOptions::RATIO_WINDOW;
            options->_sourceRatioWindow.set(0,0,1,1);

            options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
            options->_destinationPixelWindow.set(0,0,bumpMapSizeWidth,bumpMapSizeHeight);

            osgDB::Registry::instance()->setOptions(options.get());
	    
	    bool useImage = false;
	    if (useImage)
	    {
        	osg::Image* image = osgDB::readImageFile(inputFile.c_str());
        	if (image)
        	{
                    osg::HeightField* grid = createHeightField(image,osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(1.0,1.0,VERTICAL_SIZE));
                    normalMap = createNormalMap(grid);
                    normalMap->setFileName("normalmap.rgb");
                    osgDB::writeImageFile(*normalMap,"normalmap.rgb");


//                     scene = createQuadTree(grid, 
//                         	  0, grid->getNumColumns()-1,
//                         	  0, grid->getNumRows()-1,
//                         	  0.0f, 1.0f,
//                         	  0.0f, 1.0f,
//                         	  2000, numLevels);



        	}
            }
            else
            {
        	osg::HeightField* grid = osgDB::readHeightFieldFile(inputFile.c_str());
        	if (grid)
        	{
                    osg::HeightField::HeightList& hlist = grid->getHeightList();
                    for(osg::HeightField::HeightList::iterator itr=hlist.begin();
                        itr!=hlist.end();
                        ++itr)
                    {
                        (*itr) *= VERTICAL_SIZE;
                    }
                
                    normalMap = createNormalMap(grid);
                    normalMap->setFileName("normalmap.rgb");
                    osgDB::writeImageFile(*normalMap,"normalmap.rgb");

//                     osg::Geode* geode = new osg::Geode;
//                     geode->addDrawable(new osg::ShapeDrawable(grid));
// 
//                     scene = geode;
                     scene = createQuadTree(grid, 
                        	  0, grid->getNumColumns()-1,
                        	  0, grid->getNumRows()-1,
                        	  0.0f, 1.0f,
                        	  0.0f, 1.0f,
                        	  2000, numLevels);

        	}
            }
        }

        // generate diffuse map
        osg::Image* diffuseMap = 0;
//         {
//             osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
//             options->_sourceImageWindowMode = osgDB::ImageOptions::RATIO_WINDOW;
//             options->_sourceRatioWindow.set(0,0,1,1);
// 
//             options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
//             options->_destinationPixelWindow.set(0,0,2048,2048);
// 
//             osgDB::Registry::instance()->setOptions(options.get());
//             diffuseMap = osgDB::readImageFile(inputFile.c_str());
//             if (diffuseMap)
//             {
//                 diffuseMap->setFileName("diffuse.rgb");
//                 osgDB::writeImageFile(*diffuseMap,"diffuse.rgb");
//             }
// 
//         }
        
        //diffuseMap = osgDB::readImageFile(diffuseTextureName.c_str());
	diffuseMap = new osg::Image;
	diffuseMap->setFileName(diffuseTextureName.c_str());

        
        
        osg::Texture2D* diffuseMapTexture = new osg::Texture2D(diffuseMap);
        osg::Texture2D* normalMapTexture = new osg::Texture2D(normalMap);

        // create osgFX::BumpingMapping
        osg::ref_ptr<osgFX::BumpMapping> bumpMap = new osgFX::BumpMapping;
        bumpMap->setLightNumber(0);
        bumpMap->setNormalMapTextureUnit(0);
        bumpMap->setDiffuseTextureUnit(1);
        bumpMap->selectTechnique(1);
        bumpMap->setOverrideDiffuseTexture(diffuseMapTexture);
        bumpMap->setOverrideNormalMapTexture(normalMapTexture);

        bumpMap->addChild(scene.get());

#ifdef USE_PREPARE
        bumpMap->prepareChildren();
#endif        

        osg::ref_ptr<osg::Group> group = new osg::Group;
        
        
        group->addChild(bumpMap.get());

        osg::StateSet* stateset = group->getOrCreateStateSet();
        stateset->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

        osgDB::writeNodeFile(*group, baseName);

    }
    else
    {

        osg::ref_ptr<osg::Group> group = new osg::Group;
        group->addChild(createTileAndRecurse(inputFile, base, extension, 2,2, 0.0, 0.0, 1.0, 1.0, numLevels));

        osg::StateSet* stateset = group->getOrCreateStateSet();
        stateset->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

        osgDB::writeNodeFile(*group, baseName);
        
    }
    
    osg::Timer_t end_tick = timer.tick();
    std::cout << "Time to create world "<<timer.delta_s(start_tick,end_tick)<<std::endl;
    
    return true;
}


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-i <filename>","Specify the input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-o <outputfile>","Specify the output master file to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-l <numOfLevels>","Specify the number of PagedLOD levels to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    
    std::string inputFile;
    while (arguments.read("-i",inputFile)) {}
    
    std::string basename("output.ive");
    while (arguments.read("-o",basename)) {}

    float numLevels;
    while (arguments.read("-l",numLevels)) {}

    while (arguments.read("-v",VERTICAL_SIZE)) {}

    std::string diffuseTextureName("lz.ive");
    while (arguments.read("-t",diffuseTextureName)) {}

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
//     if (arguments.argc()<=1)
//     {
//         arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
//         return 1;
//     }



    // create a graphics context to allow us to use OpenGL to compress textures.    
    GraphicsContext gfx;

    createWorld(inputFile,basename,diffuseTextureName,(unsigned int)numLevels);

    return 0;
}

