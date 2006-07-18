/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osg/ClusterCullingCallback>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ImageOptions>
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>

#include <osgProducer/Viewer>

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


osg::Vec3 computePosition(bool leftHemisphere, double x, double y)
{
    double latitude = osg::PI*x;
    double longitude = osg::PI*y;
    double sin_longitude = sin(longitude);

    if (leftHemisphere) return osg::Vec3(cos(latitude)*sin_longitude,sin(latitude)*sin_longitude,-cos(longitude));
    else return osg::Vec3(-cos(latitude)*sin_longitude,-sin(latitude)*sin_longitude,-cos(longitude));
}

bool useCompressedTextures = true;
bool use565 = true;

osg::Node* createTile(const std::string& filename, bool leftHemisphere, double x, double y, double w,double h)
{
    osg::Geode* geode = new osg::Geode;
    
    osg::StateSet* stateset = new osg::StateSet();
    
    
    osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
    options->_sourceImageWindowMode = osgDB::ImageOptions::RATIO_WINDOW;
    options->_sourceRatioWindow.set(x,1-(y+h),w,h);

    options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
    options->_destinationPixelWindow.set(0,0,256,256);

    osgDB::ImageOptions::TexCoordRange* texCoordRange = 0;

    osgDB::Registry::instance()->setOptions(options.get());
    osg::Image* image = osgDB::readImageFile(filename.c_str());
    if (image)
    {
        texCoordRange = dynamic_cast<osgDB::ImageOptions::TexCoordRange*>(image->getUserData());
    
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        texture->setMaxAnisotropy(8);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        
        if (useCompressedTextures)
        {
            texture->setInternalFormatMode(osg::Texture::USE_S3TC_DXT3_COMPRESSION);

            osg::ref_ptr<osg::State> state = new osg::State;
            texture->apply(*state);

            image->readImageFromCurrentTexture(0,true);

            texture->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);
            
        } else if (use565)
        {

            image->scaleImage(image->s(),image->t(),image->r(),GL_UNSIGNED_SHORT_5_6_5);

        }
        
    }
    
    geode->setStateSet( stateset );

//    unsigned int numColumns = 10;
//    unsigned int numRows = 10;
    unsigned int numColumns = 10;
    unsigned int numRows = 10;
    unsigned int r;
    unsigned int c;

    osg::Geometry* geometry = new osg::Geometry;

    osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
    osg::Vec3Array& n = *(new osg::Vec3Array(numColumns*numRows));
    osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
    osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

    color[0].set(255,255,255,255);



    osg::Vec2 tex_orig(0.0f,0.0f);

    float rowTexDelta = 1.0f/(float)(numRows-1);
    float columnTexDelta = 1.0f/(float)(numColumns-1);

    if (texCoordRange)
    {
        tex_orig.set(texCoordRange->_x,texCoordRange->_y);
        rowTexDelta = texCoordRange->_h/(float)(numRows-1);
        columnTexDelta = texCoordRange->_w/(float)(numColumns-1);
        
        std::cout<<"setting tex values to use texCoordRange"<<std::endl;
        std::cout<<"    tex_orig="<<tex_orig<<std::endl;
        std::cout<<"    rowTexDelta"<<rowTexDelta<<std::endl;
        std::cout<<"    columnTexDelta"<<columnTexDelta<<std::endl;
    }

    double orig_latitude = osg::PI*x;
    double delta_latitude = osg::PI*w/(double)(numColumns-1);
    
    // measure as 0 at south pole, postive going north
    double orig_longitude = osg::PI*y;
    double delta_longitude = osg::PI*h/(double)(numRows-1);
    
    osg::Vec2 tex(tex_orig);
    int vi=0;
    double longitude = orig_longitude;
    osg::Vec3 normal;
    for(r=0;r<numRows;++r)
    {
        double latitude = orig_latitude;
        
        tex.x() = tex_orig.x();
        for(c=0;c<numColumns;++c)
        {
            double sin_longitude = sin(longitude);
            if (leftHemisphere)
            {
                normal.set(cos(latitude)*sin_longitude,sin(latitude)*sin_longitude,-cos(longitude));
            }
            else
            {
                normal.set(-cos(latitude)*sin_longitude,-sin(latitude)*sin_longitude,-cos(longitude));
            }
            v[vi] = normal;
            n[vi] = normal;
            
            t[vi].set(tex.x(),tex.y());
            latitude+=delta_latitude;
            tex.x()+=columnTexDelta;
            ++vi;
        }
        longitude += delta_longitude;
        tex.y() += rowTexDelta;
    }

    geometry->setVertexArray(&v);
    geometry->setNormalArray(&n);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->setColorArray(&color);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->setTexCoordArray(0,&t);

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
    
    //geometry->setUseVertexBufferObjects(true);
    
    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(false);


    {
        osg::Vec3 center = computePosition(leftHemisphere, x+w*0.5, y+h*0.5);
        osg::Vec3 normal = center;
        osg::Vec3 n00 = computePosition(leftHemisphere, x, y);
        osg::Vec3 n10 = computePosition(leftHemisphere, x+w, y);
        osg::Vec3 n11 = computePosition(leftHemisphere, x+w, y+h);
        osg::Vec3 n01 = computePosition(leftHemisphere, x, y+h);

        float radius = (center-n00).length();
        radius = osg::maximum((center-n10).length(),radius);
        radius = osg::maximum((center-n11).length(),radius);
        radius = osg::maximum((center-n01).length(),radius);

        float min_dot = normal*n00;
        min_dot = osg::minimum(normal*n10,min_dot);
        min_dot = osg::minimum(normal*n11,min_dot);
        min_dot = osg::minimum(normal*n01,min_dot);

        float angle = acosf(min_dot)+osg::PI*0.5f;
        float deviation = (angle<osg::PI) ? cosf(angle) : -1.0f;

        osg::ClusterCullingCallback* ccc = new osg::ClusterCullingCallback;
        ccc->setControlPoint(center);
        ccc->setNormal(normal);
        ccc->setRadius(radius);
        ccc->setDeviation(deviation);
        geometry->setCullCallback(ccc);
    }    
    

    geode->addDrawable(geometry);

    return geode;
}

osg::Node* createTileAndRecurse(const std::string& filename, const std::string& basename, const std::string& extension, bool leftHemisphere, unsigned int noTilesX, unsigned int noTilesY, double x, double y, double w,double h, unsigned int numLevelsLeft)
{
    osg::Group* group = new osg::Group;
    
    double dx = w / (double) noTilesX;
    double dy = h / (double) noTilesY;
    
    
    if (numLevelsLeft>0)
    {
    
        float cut_off_distance = 4.0f*dy*osg::PI;
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
                    osg::ref_ptr<osg::Node> node = createTileAndRecurse(filename,lbasename,extension,leftHemisphere,2,2,lx,ly,dx,dy,numLevelsLeft-1);
                    osgDB::writeNodeFile(*node, lbasename+extension);
                }
                
                // create PagedLOD for tile.
                osg::PagedLOD* pagedlod = new osg::PagedLOD;
                osg::Node* tile = createTile(filename,leftHemisphere,lx,ly,dx,dy);
                pagedlod->addChild(tile, cut_off_distance,max_visible_distance);
                pagedlod->setRange(1,0.0f,cut_off_distance);
                pagedlod->setFileName(1,lbasename+extension);
                pagedlod->setCenter(computePosition(leftHemisphere,lx+dx*0.5,ly+dy*0.5));

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
                group->addChild(createTile(filename,leftHemisphere,lx,ly,dx,dy));
            }
        }
    }
    
    return group;
    
}

bool createWorld(const std::string& left_hemisphere, const std::string& right_hemisphere, const std::string& baseName, unsigned int numLevels)
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


    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(createTileAndRecurse(left_hemisphere, base+"_west", extension, true, 5,5, 0.0, 0.0, 1.0, 1.0, numLevels));
    group->addChild(createTileAndRecurse(right_hemisphere, base+"_east", extension, false, 5,5, 0.0, 0.0, 1.0, 1.0, numLevels));
   
    osg::StateSet* stateset = group->getOrCreateStateSet();
    stateset->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osgDB::writeNodeFile(*group, baseName);

    
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
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an example which creates and paged database from the Nase blue marble hemisphere's.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-e <filename>","Specify the east hemisphere input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-w <filename>","Specify the west hemisphere input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-o <outputfile>","Specify the output master file to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-l <numOfLevels>","Specify the number of PagedLOD levels to generate");
    arguments.getApplicationUsage()->addCommandLineOption("--compressed","Create compressed textures (default)");
    arguments.getApplicationUsage()->addCommandLineOption("--565","Create R5G5B5A1 textures");
    arguments.getApplicationUsage()->addCommandLineOption("--RGB","Create R8G8B8 textures");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    
    std::string west_hemisphere("land_shallow_topo_west.tif");
    while (arguments.read("-w",west_hemisphere)) {}

    std::string east_hemisphere("land_shallow_topo_east.tif");
    while (arguments.read("-e",east_hemisphere)) {}

    std::string basename("bluemarble.ive");
    while (arguments.read("-o",basename)) {}

    float numLevels=4;
    while (arguments.read("-l",numLevels)) {}

    while (arguments.read("--compressed")) { useCompressedTextures = true; use565 = false; }
    while (arguments.read("--565")) { useCompressedTextures = false; use565 = true;}
    while (arguments.read("--RGB")) { useCompressedTextures = false; use565 = false; }

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

    createWorld(west_hemisphere,east_hemisphere,basename,(unsigned int)numLevels);

    return 0;
}

