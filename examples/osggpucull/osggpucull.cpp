/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
 *
*/

 /** osggpucull example.

  A geometry instancing rendering algorithm consisting of two consequent phases :

    - first phase is a GLSL shader performing object culling and LOD picking ( a culling shader ).
      Every culled object is represented as GL_POINT in the input osg::Geometry.
      The output of the culling shader is a set of object LODs that need to be rendered.
      The output is stored in texture buffer objects. No pixel is drawn to the screen
      because GL_RASTERIZER_DISCARD mode is used.

    - second phase draws osg::Geometry containing merged LODs using glDrawArraysIndirect()
      function. Information about quantity of instances to render, its positions and other
      parameters is sourced from texture buffer objects filled in the first phase.

    The example uses various OpenGL 4.2 features such as texture buffer objects,
    atomic counters, image units and functions defined in GL_ARB_shader_image_load_store
    extension to achieve its goal and thus will not work on graphic cards with older OpenGL
    versions.

    The example was tested on Linux and Windows with NVidia 570 and 580 cards.
    The tests on AMD cards were not conducted ( due to lack of it ).
    The tests were performed using OSG revision 14088.

    The main advantages of this rendering method :
    - instanced rendering capable of drawing thousands of different objects with
      almost no CPU intervention  ( cull and draw times are close to 0 ms ).
    - input objects may be sourced from any OSG graph ( for example - information about
      object points may be stored in a PagedLOD graph. This way we may cover the whole
      countries with trees, buildings and other objects ).
      Furthermore if we create osgDB plugins that generate data on the fly, we may
      generate information for every grass blade for that country.
    - every object may have its own parameters and thus may be distinct from other objects
      of the same type.
    - relatively low memory footprint ( single object information is stored in a few
      vertex attributes ).
    - no GPU->CPU roundtrip typical for such methods ( method uses atomic counters
      and glDrawArraysIndirect() function instead of OpenGL queries. This way
      information about quantity of rendered objects never goes back to CPU.
      The typical GPU->CPU roundtrip cost is about 2 ms ).
    - this example also shows how to render dynamic objects ( objects that may change
      its position ) with moving parts ( like car wheels or airplane propellers ) .
      The obvious extension to that dynamic method would be the animated crowd rendering.
    - rendered objects may be easily replaced ( there is no need to process the whole
      OSG graphs, because these graphs store only positional information ).

    The main disadvantages of a method :
    - the maximum quantity of objects to render must be known beforehand
      ( because texture buffer objects holding data between phases have constant size ).
    - OSG statistics are flawed ( they don't know anymore how many objects are drawn ).
    - osgUtil::Intersection does not work

    Example application may be used to make some performance tests, so below you
    will find some extended parameter description :
    --skip-dynamic       - skip rendering of dynamic objects if you only want to
                           observe static object statistics
    --skip-static        - the same for static objects
    --dynamic-area-size  - size of the area for dynamic rendering. Default = 1000 meters
                           ( square 1000m x 1000m ). Along with density defines
                           how many dynamic objects is there in the example.
    --static-area-size   - the same for static objects. Default = 2000 meters
                           ( square 2000m x 2000m ).

    Example application defines some parameters (density, LOD ranges, object's triangle count).
    You may manipulate its values using below described modifiers:
    --density-modifier   - density modifier in percent. Default = 100%.
                           Density ( along with LOD ranges ) defines maximum
                           quantity of rendered objects. registerType() function
                           accepts maximum density ( in objects per square kilometer )
                           as its parameter.
    --lod-modifier       - defines the LOD ranges. Default = 100%.
    --triangle-modifier  - defines the number of triangles in finally rendered objects.
                           Default = 100 %.
    --instances-per-cell - for static rendering the application builds OSG graph using
                           InstanceCell class ( this class is a modified version of Cell class
                           from osgforest example - it builds simple quadtree from a list
                           of static instances ). This parameter defines maximum number
                           of instances in a single osg::Group in quadtree.
                           If, for example, you modify it to value=100, you will see
                           really big cull time in OSG statistics ( because resulting
                           tree generated by InstanceCell will be very deep ).
                           Default value = 4096 .
    --export-objects     - write object geometries and quadtree of instances to osgt files
                           for later analysis.
    --use-multi-draw     - use glMultiDrawArraysIndirect() instead of glDrawArraysIndirect() in a
                           draw shader. Thanks to this we may render all ( different ) objects
                           using only one draw call. Requires OpenGL version 4.3.

    This application is inspired by Daniel Rákos work : "GPU based dynamic geometry LOD" that
    may be found under this address : http://rastergrid.com/blog/2010/10/gpu-based-dynamic-geometry-lod/
    There are however some differences :
    - Daniel Rákos uses GL queries to count objects to render, while this example
      uses atomic counters ( no GPU->CPU roundtrip )
    - this example does not use transform feedback buffers to store intermediate data
      ( it uses texture buffer objects instead ).
    - I use only the vertex shader to cull objects, whereas Daniel Rákos uses vertex shader
      and geometry shader ( because only geometry shader can send more than one primitive
      to transform feedback buffers ).
    - objects in the example are drawn using glDrawArraysIndirect() function,
      instead of glDrawElementsInstanced().

    Finally there are some things to consider/discuss  :
    - the whole algorithm exploits nice OpenGL feature that any GL buffer
      may be bound as any type of buffer ( in our example a buffer is once bound
      as a texture buffer object, and later is bound as GL_DRAW_INDIRECT_BUFFER ).
      osg::TextureBuffer class has one handy method to do that trick ( bindBufferAs() ),
      and new primitive sets use osg::TextureBuffer as input.
      For now I added new primitive sets to example ( DrawArraysIndirect and
      MultiDrawArraysIndirect defined in examples/osggpucull/DrawIndirectPrimitiveSet.h ),
      but if Robert will accept its current implementations ( I mean - primitive
      sets that have osg::TextureBuffer in constructor ), I may add it to
      osg/include/PrimitiveSet header.
    - I used BufferTemplate class written and published by Aurelien in submission forum
      some time ago. For some reason this class never got into osg/include, but is
      really needed during creation of UBOs, TBOs, and possibly SSBOs in the future.
      I added std::vector specialization to that template class.
    - I needed to create similar osg::Geometries with variable number of vertices
      ( to create different LODs in my example ). For this reason I've written
      some code allowing me to create osg::Geometries from osg::Shape descendants.
      This code may be found in ShapeToGeometry.* files. Examples of use are in
      osggpucull.cpp . The question is : should this code stay in example, or should
      it be moved to osgUtil ?
    - this remark is important for NVidia cards on Linux and Windows : if
      you have "Sync to VBlank" turned ON in nvidia-settings and you want to see
      real GPU times in OSG statistics window, you must set the power management
      settings to "Prefer maximum performance", because when "Adaptive mode" is used,
      the graphic card's clock may be slowed down by the driver during program execution
      ( On Linux when OpenGL application starts in adaptive mode, clock should work
      as fast as possible, but after one minute of program execution, the clock slows down ).
      This happens when GPU time in OSG statistics window is shorter than 3 ms.
*/

#include <osg/Vec4i>
#include <osg/Quat>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/Image>
#include <osg/Texture>
#include <osg/TextureBuffer>
#include <osg/BindImageTexture>
#include <osg/BufferIndexBinding>
#include <osg/ComputeBoundsVisitor>
#include <osg/LightSource>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/BufferTemplate>
#include <osg/PrimitiveSetIndirect>

#include "ShapeToGeometry.h"
#include "AggregateGeometryVisitor.h"
#include "GpuCullShaders.h"



// each instance type may have max 8 LODs ( if you change
// this value, don't forget to change it in vertex shaders accordingly )
const unsigned int OSGGPUCULL_MAXIMUM_LOD_NUMBER = 8;
// during culling each instance may be sent to max 4 indirect targets
const unsigned int OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER = 4;

// Struct defining LOD data for particular instance type
struct InstanceLOD
{
    InstanceLOD()
        : bbMin(FLT_MAX,FLT_MAX,FLT_MAX,1.0f), bbMax(-FLT_MAX,-FLT_MAX,-FLT_MAX,1.0f)
    {
    }
    InstanceLOD( const InstanceLOD& iLod )
        : bbMin(iLod.bbMin), bbMax(iLod.bbMax), indirectTargetParams(iLod.indirectTargetParams), distances(iLod.distances)
    {
    }
    InstanceLOD& operator=( const InstanceLOD& iLod )
    {
        if( &iLod != this )
        {
            bbMin                   = iLod.bbMin;
            bbMax                   = iLod.bbMax;
            indirectTargetParams    = iLod.indirectTargetParams;
            distances               = iLod.distances;
        }
        return *this;
    }

    inline void setBoundingBox( const osg::BoundingBox& bbox )
    {
        bbMin = osg::Vec4f( bbox.xMin(), bbox.yMin(), bbox.zMin(), 1.0f );
        bbMax = osg::Vec4f( bbox.xMax(), bbox.yMax(), bbox.zMax(), 1.0f );
    }
    inline osg::BoundingBox getBoundingBox()
    {
        return osg::BoundingBox( bbMin.x(), bbMin.y(), bbMin.z(), bbMax.x(), bbMax.y(), bbMax.z() );
    }

    osg::Vec4f bbMin;                   // LOD bounding box
    osg::Vec4f bbMax;
    osg::Vec4i indirectTargetParams;    // x=lodIndirectCommand, y=lodIndirectCommandIndex, z=offsetsInTarget, w=lodMaxQuantity
    osg::Vec4f distances;               // x=minDistance, y=minFadeDistance, z=maxFadeDistance, w=maxDistance
};

// Struct defining information about specific instance type : bounding box, lod ranges, indirect target indices etc
struct InstanceType
{
    InstanceType()
        : bbMin(FLT_MAX,FLT_MAX,FLT_MAX,1.0f), bbMax(-FLT_MAX,-FLT_MAX,-FLT_MAX,1.0f)
    {
        params.x() = 0; // this variable defines the number of LODs
        for(unsigned int i=0;i<OSGGPUCULL_MAXIMUM_LOD_NUMBER;++i)
            lods[i] = InstanceLOD();
    }
    InstanceType( const InstanceType& iType )
        : bbMin(iType.bbMin), bbMax(iType.bbMax), params(iType.params)
    {
        for(unsigned int i=0;i<OSGGPUCULL_MAXIMUM_LOD_NUMBER;++i)
            lods[i] = iType.lods[i];
    }
    InstanceType& operator=( const InstanceType& iType )
    {
        if( &iType != this )
        {
            bbMin   = iType.bbMin;
            bbMax   = iType.bbMax;
            params  = iType.params;
            for(unsigned int i=0;i<OSGGPUCULL_MAXIMUM_LOD_NUMBER;++i)
                lods[i] = iType.lods[i];
        }
        return *this;
    }
    inline void setLodDefinition( unsigned int i, unsigned int targetID, unsigned int indexInTarget, const osg::Vec4& distance, unsigned int offsetInTarget, unsigned int maxQuantity, const osg::BoundingBox& lodBBox)
    {
        if( i >= OSGGPUCULL_MAXIMUM_LOD_NUMBER)
            return;
        params.x()                      = osg::maximum<int>( params.x(), i+1);
        lods[i].indirectTargetParams    = osg::Vec4i( targetID, indexInTarget, offsetInTarget, maxQuantity );
        lods[i].distances               = distance;
        lods[i].setBoundingBox( lodBBox );
        expandBy(lodBBox);
    }
    inline void expandBy( const osg::BoundingBox& bbox )
    {
        osg::BoundingBox myBBox = getBoundingBox();
        myBBox.expandBy(bbox);
        setBoundingBox(myBBox);
    }
    inline void setBoundingBox( const osg::BoundingBox& bbox )
    {
        bbMin = osg::Vec4f( bbox.xMin(), bbox.yMin(), bbox.zMin(), 1.0f );
        bbMax = osg::Vec4f( bbox.xMax(), bbox.yMax(), bbox.zMax(), 1.0f );
    }
    inline osg::BoundingBox getBoundingBox()
    {
        return osg::BoundingBox( bbMin.x(), bbMin.y(), bbMin.z(), bbMax.x(), bbMax.y(), bbMax.z() );
    }

    osg::Vec4f  bbMin;                              // bounding box that includes all LODs
    osg::Vec4f  bbMax;
    osg::Vec4i  params;                             // x=number of active LODs
    InstanceLOD lods[OSGGPUCULL_MAXIMUM_LOD_NUMBER]; // information about LODs
};

// CPU side representation of a struct defined in ARB_draw_indirect extension
struct DrawArraysIndirectCommand
{
    DrawArraysIndirectCommand()
        : count(0), primCount(0), first(0), baseInstance(0)
    {
    }
    DrawArraysIndirectCommand( unsigned int aFirst, unsigned int aCount )
        : count(aCount), primCount(0), first(aFirst), baseInstance(0)
    {
    }
    unsigned int count;
    unsigned int primCount;
    unsigned int first;
    unsigned int baseInstance;
};

// During the first phase of instance rendering cull shader places information about
// instance LODs in texture buffers called "indirect targets"
// All data associated with the indirect target is placed in struct defined below
// ( like for example - draw shader associated with specific indirect target.
// Draw shader performs second phase of instance rendering - the actual rendering of objects
// to screen or to frame buffer object ).
struct IndirectTarget
{
    IndirectTarget()
        : maxTargetQuantity(0)
    {
        indirectCommands    = new osg::DefaultIndirectCommandDrawArrays;
        indirectCommands->getBufferObject()->setUsage(GL_DYNAMIC_DRAW);
    }
    IndirectTarget( AggregateGeometryVisitor* agv, osg::Program* program )
        : geometryAggregator(agv), drawProgram(program), maxTargetQuantity(0)
    {
        indirectCommands    = new osg::DefaultIndirectCommandDrawArrays;
        indirectCommands->getBufferObject()->setUsage(GL_DYNAMIC_DRAW);
    }
    void endRegister(unsigned int index, unsigned int rowsPerInstance, GLenum pixelFormat, GLenum type, GLint internalFormat, bool useMultiDrawArraysIndirect )
    {
        indirectCommandTextureBuffer = new osg::TextureBuffer(indirectCommands.get());
        indirectCommandTextureBuffer->setInternalFormat( GL_R32I );
        indirectCommandTextureBuffer->setUnRefImageDataAfterApply(false);

        indirectCommandImageBinding=new osg::BindImageTexture(index, indirectCommandTextureBuffer.get(), osg::BindImageTexture::READ_WRITE, GL_R32I);

        // add proper primitivesets to geometryAggregators
        if( !useMultiDrawArraysIndirect ) // use glDrawArraysIndirect()
        {
            std::vector<osg::DrawArraysIndirect*> newPrimitiveSets;

            for(unsigned int j=0;j<indirectCommands->size(); ++j)
            {
                osg::DrawArraysIndirect *ipr=new osg::DrawArraysIndirect( GL_TRIANGLES, j );
                ipr->setIndirectCommandArray( indirectCommands.get());
                newPrimitiveSets.push_back(ipr);
            }

            geometryAggregator->getAggregatedGeometry()->removePrimitiveSet(0,geometryAggregator->getAggregatedGeometry()->getNumPrimitiveSets() );

            for(unsigned int j=0;j<indirectCommands->size(); ++j)
                geometryAggregator->getAggregatedGeometry()->addPrimitiveSet( newPrimitiveSets[j] );


        }
        else // use glMultiDrawArraysIndirect()
        {
            osg::MultiDrawArraysIndirect *ipr=new osg::MultiDrawArraysIndirect( GL_TRIANGLES );
            ipr->setIndirectCommandArray( indirectCommands.get() );
            geometryAggregator->getAggregatedGeometry()->removePrimitiveSet(0,geometryAggregator->getAggregatedGeometry()->getNumPrimitiveSets() );
            geometryAggregator->getAggregatedGeometry()->addPrimitiveSet( ipr );
        }

        geometryAggregator->getAggregatedGeometry()->setUseDisplayList(false);
        geometryAggregator->getAggregatedGeometry()->setUseVertexBufferObjects(true);


        osg::Image* instanceTargetImage = new osg::Image;
        instanceTargetImage->allocateImage( maxTargetQuantity*rowsPerInstance, 1, 1, pixelFormat, type );

        osg::VertexBufferObject * instanceTargetImageBuffer=new osg::VertexBufferObject();
        instanceTargetImageBuffer->setUsage(GL_DYNAMIC_DRAW);
        instanceTargetImage->setBufferObject(instanceTargetImageBuffer);

        instanceTarget = new osg::TextureBuffer(instanceTargetImage);
        instanceTarget->setInternalFormat( internalFormat );

        instanceTargetimagebinding = new osg::BindImageTexture(OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER+index, instanceTarget.get(), osg::BindImageTexture::READ_WRITE, internalFormat);

    }

    void addIndirectCommandData( const std::string& uniformNamePrefix, int index, osg::StateSet* stateset )
    {
        std::string uniformName = uniformNamePrefix + char( '0' + index );
        osg::Uniform* uniform = new osg::Uniform(uniformName.c_str(), (int)index );
        stateset->addUniform( uniform );
        stateset->setAttribute(indirectCommandImageBinding);
        stateset->setTextureAttribute( index, indirectCommandTextureBuffer.get() );


    }

    void addIndirectTargetData( bool cullPhase, const std::string& uniformNamePrefix, int index, osg::StateSet* stateset )
    {
        std::string uniformName;
        if( cullPhase )
            uniformName = uniformNamePrefix + char( '0' + index );
        else
            uniformName = uniformNamePrefix;

        osg::Uniform* uniform = new osg::Uniform(uniformName.c_str(), (int)(OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER+index) );
        stateset->addUniform( uniform );

        stateset->setAttribute(instanceTargetimagebinding);
        stateset->setTextureAttribute( OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER+index, instanceTarget.get() );
    }

    void addDrawProgram( const std::string& uniformBlockName, osg::StateSet* stateset )
    {
        drawProgram->addBindUniformBlock(uniformBlockName, 1);
        stateset->setAttributeAndModes( drawProgram.get(), osg::StateAttribute::ON );
    }

    osg::ref_ptr< osg::DefaultIndirectCommandDrawArrays >        indirectCommands;
    osg::ref_ptr<osg::TextureBuffer>                                indirectCommandTextureBuffer;
    osg::ref_ptr<osg::BindImageTexture>                             indirectCommandImageBinding;
    osg::ref_ptr< AggregateGeometryVisitor >                        geometryAggregator;
    osg::ref_ptr<osg::Program>                                      drawProgram;
    osg::ref_ptr< osg::TextureBuffer >                              instanceTarget;
    osg::ref_ptr<osg::BindImageTexture>                             instanceTargetimagebinding;
    unsigned int                                                    maxTargetQuantity;
};

// This is the main structure holding all information about particular 2-phase instance rendering
// ( instance types, indirect targets, etc ).
struct GPUCullData
{
    GPUCullData()
    {
        useMultiDrawArraysIndirect = false;
        instanceTypes = new osg::BufferTemplate< std::vector<InstanceType> >;
        // build Uniform BufferObject with instanceTypes data
        instanceTypesUBO = new osg::UniformBufferObject;
//        instanceTypesUBO->setUsage( GL_STREAM_DRAW );
        instanceTypes->setBufferObject( instanceTypesUBO.get() );
        instanceTypesUBB = new osg::UniformBufferBinding(1, instanceTypes.get(), 0, 0);

    }

    void setUseMultiDrawArraysIndirect( bool value )
    {
        useMultiDrawArraysIndirect = value;
    }

    void registerIndirectTarget( unsigned int index, AggregateGeometryVisitor* agv, osg::Program* targetDrawProgram )
    {
        if(index>=OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER || agv==NULL || targetDrawProgram==NULL )
            return;
        targets[index] = IndirectTarget( agv, targetDrawProgram );
    }

    bool registerType(unsigned int typeID, unsigned int targetID, osg::Node* node, const osg::Vec4& lodDistances, float maxDensityPerSquareKilometer )
    {
        if( typeID >= instanceTypes->getData().size() )
            instanceTypes->getData().resize(typeID+1);
        InstanceType& itd = instanceTypes->getData().at(typeID);
        unsigned int lodNumber = (unsigned int)itd.params.x();
        if( lodNumber >= OSGGPUCULL_MAXIMUM_LOD_NUMBER )
            return false;

        std::map<unsigned int, IndirectTarget>::iterator target = targets.find(targetID);
        if( target==targets.end() )
            return false;

        // AggregateGeometryVisitor creates single osg::Geometry from all objects used by specific indirect target
        AggregateGeometryVisitor::AddObjectResult aoResult = target->second.geometryAggregator->addObject( node , typeID, lodNumber );
        // Information about first vertex and a number of vertices is stored for later primitiveset creation
        target->second.indirectCommands->push_back( osg::DrawArraysIndirectCommand( aoResult.count,1, aoResult.first ) );

        osg::ComputeBoundsVisitor cbv;
        node->accept(cbv);

        // Indirect target texture buffers have finite size, therefore each instance LOD has maximum number that may be rendered in one frame.
        // This maximum number of rendered instances is estimated from the area that LOD covers and maximum density of instances per square kilometer.
        float lodArea = osg::PI * ( lodDistances.w() * lodDistances.w() - lodDistances.x() * lodDistances.x() ) / 1000000.0f;
        // calculate max quantity of objects in lodArea using maximum density per square kilometer
        unsigned int maxQuantity   = (unsigned int) ceil( lodArea * maxDensityPerSquareKilometer );

        itd.setLodDefinition( lodNumber, targetID, aoResult.index, lodDistances, target->second.maxTargetQuantity, maxQuantity, cbv.getBoundingBox() ) ;
        target->second.maxTargetQuantity += maxQuantity;
        return true;
    }

    // endRegister() method is called after all indirect targets and instance types are registered.
    // It creates indirect targets with pixel format and data type provided by user ( indirect targets may hold
    // different information about single instance depending on user's needs ( in our example : static rendering
    // sends all vertex data to indirect target during GPU cull phase, while dynamic rendering sends only a "pointer"
    // to texture buffer containing instance data ( look at endRegister() invocations in createStaticRendering() and
    // createDynamicRendering() )
    void endRegister( unsigned int rowsPerInstance, GLenum pixelFormat, GLenum type, GLint internalFormat )
    {
        OSG_INFO<<"instance types"<<std::endl;
        for( unsigned int i=0; i<instanceTypes->getData().size(); ++i)
        {
            InstanceType& iType = instanceTypes->getData().at(i);
            int sum = 0;
            OSG_INFO<<"Type "<<i<<" : ( " ;
            int lodCount = iType.params.x();
            for(int j=0; j<lodCount; ++j )
            {
                OSG_INFO << "{" << iType.lods[j].indirectTargetParams.x() << "}"<<iType.lods[j].indirectTargetParams.z() << "->" << iType.lods[j].indirectTargetParams.w() << " ";
                sum += iType.lods[j].indirectTargetParams.w();
            }
            OSG_INFO<< ") => " << sum << " elements"<<std::endl;
        }

        OSG_INFO<<"indirect targets"<<std::endl;
        std::map<unsigned int, IndirectTarget>::iterator it,eit;
        for(it=targets.begin(), eit=targets.end(); it!=eit; ++it)
        {
            for(unsigned j=0; j<it->second.indirectCommands->size(); ++j)
            {
                osg::DrawArraysIndirectCommand& iComm = it->second.indirectCommands->at(j);
                OSG_INFO<<"("<<iComm.first<<" "<<iComm.instanceCount<<" "<<iComm.count<<") ";
            }
            unsigned int sizeInBytes = (unsigned int ) it->second.maxTargetQuantity * sizeof(osg::Vec4);
            OSG_INFO<<" => Maximum elements in target : "<< it->second.maxTargetQuantity <<" ( "<< sizeInBytes <<" bytes, " << sizeInBytes/1024<< " kB )" << std::endl;
        }

        instanceTypesUBB->setSize( instanceTypes->getTotalDataSize() );
        for(it=targets.begin(), eit=targets.end(); it!=eit; ++it)
            it->second.endRegister(it->first,rowsPerInstance,pixelFormat,type,internalFormat,useMultiDrawArraysIndirect);

    }

    bool                                                useMultiDrawArraysIndirect;
    osg::ref_ptr< osg::BufferTemplate< std::vector<InstanceType> > >   instanceTypes;
    osg::ref_ptr<osg::UniformBufferObject>              instanceTypesUBO;
    osg::ref_ptr<osg::UniformBufferBinding>             instanceTypesUBB;

    std::map<unsigned int, IndirectTarget>               targets;
};

// StaticInstance struct represents data of a single static instance :
// its position, type, identification and additional params
// ( params are user dependent. In our example params are used
// to describe color, waving amplitude, waving frequency and waving offset )
struct StaticInstance
{
   StaticInstance( unsigned int typeID, unsigned int id, const osg::Matrixf& m, const osg::Vec4& params )
    : position(m), extraParams(params), idParams(typeID,id,0,0)
   {
   }

   osg::Vec3d getPosition() const
   {
       return position.getTrans();
   }

   osg::Matrixf position;
   osg::Vec4f   extraParams;
   osg::Vec4i   idParams;
};

// DynamicInstance ( compared to StaticInstance ) holds additional data, like "bones" used to
// animate wheels and propellers in the example

const unsigned int OSGGPUCULL_MAXIMUM_BONES_NUMBER = 8;

struct DynamicInstance
{
   DynamicInstance( unsigned int typeID, unsigned int id, const osg::Matrixf& m, const osg::Vec4& params )
    : position(m), extraParams(params), idParams(typeID,id,0,0)
   {
       for(unsigned int i=0; i<OSGGPUCULL_MAXIMUM_BONES_NUMBER; ++i)
           bones[i] = osg::Matrixf::identity();
   }
   osg::Vec3d getPosition() const
   {
       return position.getTrans();
   }
   osg::Matrixf position;
   osg::Vec4f   extraParams;
   osg::Vec4i   idParams;
   osg::Matrixf bones[OSGGPUCULL_MAXIMUM_BONES_NUMBER];
};

// This class has been taken from osgforest example and modified a little bit.
// Its purpose is to store instances and to divide it into osg tree.
template<typename T>
class InstanceCell : public osg::Referenced
{
public:
    typedef std::vector< osg::ref_ptr<InstanceCell<T> > > InstanceCellList;

    InstanceCell(): _parent(0) {}

    InstanceCell(osg::BoundingBox& bb):_parent(0), _bb(bb) {}

    void addCell(InstanceCell* cell) { cell->_parent=this; _cells.push_back(cell); }

    void computeBound();

    bool contains(const osg::Vec3& position) const { return _bb.contains(position); }

    bool divide(unsigned int maxNumInstancesPerCell=100);

    bool divide(bool xAxis, bool yAxis, bool zAxis);

    void bin();

    InstanceCell*       _parent;
    osg::BoundingBox    _bb;
    InstanceCellList    _cells;
    std::vector<T>      _instances;
};

template<typename T>
void InstanceCell<T>::computeBound()
{
    _bb.init();
    for(typename InstanceCellList::iterator citr=_cells.begin();
        citr!=_cells.end();
        ++citr)
    {
        (*citr)->computeBound();
        _bb.expandBy((*citr)->_bb);
    }

    for(typename std::vector<T>::iterator titr=_instances.begin();
        titr!=_instances.end(); ++titr)
    {
        _bb.expandBy( titr->getPosition() );
    }
}

template<typename T>
bool InstanceCell<T>::divide(unsigned int maxNumInstancesPerCell)
{
    if (_instances.size()<=maxNumInstancesPerCell) return false;

    computeBound();

    float radius = _bb.radius();
    float divide_distance = radius*0.7f;
    if (divide((_bb.xMax()-_bb.xMin())>divide_distance,(_bb.yMax()-_bb.yMin())>divide_distance,(_bb.zMax()-_bb.zMin())>divide_distance))
    {
        // recusively divide the new cells till maxNumInstancesPerCell is met.
        for(typename InstanceCellList::iterator citr=_cells.begin(); citr!=_cells.end(); ++citr)
        {
            (*citr)->divide(maxNumInstancesPerCell);
        }
        return true;
   }
   else
   {
        return false;
   }
}

template<typename T>
bool InstanceCell<T>::divide(bool xAxis, bool yAxis, bool zAxis)
{
    if (!(xAxis || yAxis || zAxis)) return false;

    if (_cells.empty())
        _cells.push_back(new InstanceCell(_bb));

    if (xAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            InstanceCell* orig_cell = _cells[i].get();
            InstanceCell* new_cell = new InstanceCell(orig_cell->_bb);

            float xCenter = (orig_cell->_bb.xMin()+orig_cell->_bb.xMax())*0.5f;
            orig_cell->_bb.xMax() = xCenter;
            new_cell->_bb.xMin() = xCenter;

            _cells.push_back(new_cell);
        }
    }

    if (yAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            InstanceCell* orig_cell = _cells[i].get();
            InstanceCell* new_cell = new InstanceCell(orig_cell->_bb);

            float yCenter = (orig_cell->_bb.yMin()+orig_cell->_bb.yMax())*0.5f;
            orig_cell->_bb.yMax() = yCenter;
            new_cell->_bb.yMin() = yCenter;

            _cells.push_back(new_cell);
        }
    }

    if (zAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            InstanceCell* orig_cell = _cells[i].get();
            InstanceCell* new_cell = new InstanceCell(orig_cell->_bb);

            float zCenter = (orig_cell->_bb.zMin()+orig_cell->_bb.zMax())*0.5f;
            orig_cell->_bb.zMax() = zCenter;
            new_cell->_bb.zMin() = zCenter;

            _cells.push_back(new_cell);
        }
    }

    bin();

    return true;

}

template<typename T>
void InstanceCell<T>::bin()
{
    // put treeste cells.
    std::vector<T> instancesNotAssigned;
    for(typename std::vector<T>::iterator titr=_instances.begin(); titr!=_instances.end(); ++titr)
    {
        osg::Vec3 iPosition = titr->getPosition();
        bool assigned = false;
        for(typename InstanceCellList::iterator citr=_cells.begin();
            citr!=_cells.end() && !assigned;
            ++citr)
        {
            if ((*citr)->contains(iPosition))
            {
                (*citr)->_instances.push_back(*titr);
                assigned = true;
            }
        }
        if (!assigned) instancesNotAssigned.push_back(*titr);
    }

    // put the unassigned trees back into the original local tree list.
    _instances.swap(instancesNotAssigned);

    // prune empty cells.
    InstanceCellList cellsNotEmpty;
    for(typename InstanceCellList::iterator citr=_cells.begin(); citr!=_cells.end(); ++citr)
    {
        if (!((*citr)->_instances.empty()))
        {
            cellsNotEmpty.push_back(*citr);
        }
    }
    _cells.swap(cellsNotEmpty);

}

// Every geometry in the static instance tree stores matrix and additional parameters on the vertex attributtes number 10-15.
osg::Geometry* buildGPUCullGeometry( const std::vector<StaticInstance>& instances  )
{
    osg::Vec3Array* vertexArray = new osg::Vec3Array;

    osg::Vec4Array* attrib10 = new osg::Vec4Array;
    osg::Vec4Array* attrib11 = new osg::Vec4Array;
    osg::Vec4Array* attrib12 = new osg::Vec4Array;
    osg::Vec4Array* attrib13 = new osg::Vec4Array;
    osg::Vec4Array* attrib14 = new osg::Vec4Array;
    osg::Vec4Array* attrib15 = new osg::Vec4Array;

    osg::BoundingBox bbox;
    std::vector<StaticInstance>::const_iterator it,eit;
    for(it=instances.begin(), eit=instances.end(); it!=eit; ++it)
    {
        vertexArray->push_back( it->getPosition() );
        attrib10->push_back( osg::Vec4( it->position(0,0), it->position(0,1), it->position(0,2), it->position(0,3) ) );
        attrib11->push_back( osg::Vec4( it->position(1,0), it->position(1,1), it->position(1,2), it->position(1,3) ) );
        attrib12->push_back( osg::Vec4( it->position(2,0), it->position(2,1), it->position(2,2), it->position(2,3) ) );
        attrib13->push_back( osg::Vec4( it->position(3,0), it->position(3,1), it->position(3,2), it->position(3,3) ) );
        attrib14->push_back( it->extraParams );
        attrib15->push_back( osg::Vec4( it->idParams.x(), it->idParams.y(), 0.0, 0.0 ) );

        bbox.expandBy( it->getPosition() );
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray(vertexArray);
    geom->setVertexAttribArray(10, attrib10, osg::Array::BIND_PER_VERTEX);
    geom->setVertexAttribArray(11, attrib11, osg::Array::BIND_PER_VERTEX);
    geom->setVertexAttribArray(12, attrib12, osg::Array::BIND_PER_VERTEX);
    geom->setVertexAttribArray(13, attrib13, osg::Array::BIND_PER_VERTEX);
    geom->setVertexAttribArray(14, attrib14, osg::Array::BIND_PER_VERTEX);
    geom->setVertexAttribArray(15, attrib15, osg::Array::BIND_PER_VERTEX);

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, instances.size() ) );

    geom->setInitialBound( bbox );

    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);

    return geom.release();
}

template<typename T>
osg::Node* createInstanceGraph(InstanceCell<T>* cell, const osg::BoundingBox& objectsBBox )
{
    bool needGroup = !(cell->_cells.empty());
    bool needInstances = !(cell->_instances.empty());

    osg::Geode* geode = 0;
    osg::Group* group = 0;

    if (needInstances)
    {
        osg::Geometry* geometry = buildGPUCullGeometry(cell->_instances);
        // buildGPUCullGeometry() function creates initial bound using points
        // it must be enlarged by bounding boxes of all instance types
        osg::BoundingBox bbox = geometry->getInitialBound();
        bbox.xMin() += objectsBBox.xMin();
        bbox.xMax() += objectsBBox.xMax();
        bbox.yMin() += objectsBBox.yMin();
        bbox.yMax() += objectsBBox.yMax();
        bbox.zMin() += objectsBBox.zMin();
        bbox.zMax() += objectsBBox.zMax();
        geometry->setInitialBound(bbox);
        geode = new osg::Geode;
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

        geode->addDrawable( geometry );
    }

    if (needGroup)
    {
        group = new osg::Group;
        for(typename InstanceCell<T>::InstanceCellList::iterator itr=cell->_cells.begin(); itr!=cell->_cells.end(); ++itr)
        {
            group->addChild(createInstanceGraph(itr->get(),objectsBBox));
        }

        if (geode) group->addChild(geode);

    }
    if (group) return group;
    else return geode;
}

template <typename T>
osg::Node* createInstanceTree(const std::vector<T>& instances, const osg::BoundingBox& objectsBBox, unsigned int maxNumInstancesPerCell )
{
    osg::ref_ptr<InstanceCell<T> > rootCell = new InstanceCell<T>();
    rootCell->_instances = instances;
    rootCell->divide( maxNumInstancesPerCell );

    osg::ref_ptr<osg::Node> resultNode = createInstanceGraph<T>( rootCell.get(), objectsBBox );
    return resultNode.release();
}

// Texture buffers holding information about the number of instances to render ( named "indirect command
// texture buffers", or simply - indirect commands ) must reset instance number to 0 in the beginning of each frame.
// It is done by simple texture reload from osg::Image.
// Moreover - texture buffers that use texture images ( i mean "images" as defined in ARB_shader_image_load_store extension )
// should call glBindImageTexture() before every shader that uses imageLoad(), imageStore() and imageAtomic*() GLSL functions.
// It looks like glBindImageTexture() should be used the same way the glBindTexture() is used.
struct ResetTexturesCallback : public osg::StateSet::Callback
{
    ResetTexturesCallback()
    {
    }

    void addTextureDirty( unsigned int texUnit )
    {
        texUnitsDirty.push_back(texUnit);
    }

    void addTextureDirtyParams( unsigned int texUnit )
    {
        texUnitsDirtyParams.push_back(texUnit);
    }

    virtual void operator() (osg::StateSet* stateset, osg::NodeVisitor* /*nv*/)
    {
        std::vector<unsigned int>::iterator it,eit;
        for(it=texUnitsDirty.begin(), eit=texUnitsDirty.end(); it!=eit; ++it)
        {
            osg::TextureBuffer* tex = dynamic_cast<osg::TextureBuffer*>( stateset->getTextureAttribute(*it,osg::StateAttribute::TEXTURE) );
            if(tex==NULL)
                continue;
            osg::BufferData* img =const_cast<osg::BufferData*>(tex->getBufferData());
            if(img!=NULL)
                img->dirty();
        }
        for(it=texUnitsDirtyParams.begin(), eit=texUnitsDirtyParams.end(); it!=eit; ++it)
        {
            osg::TextureBuffer* tex = dynamic_cast<osg::TextureBuffer*>( stateset->getTextureAttribute(*it,osg::StateAttribute::TEXTURE) );
            if(tex!=NULL)
                tex->dirtyTextureParameters();
        }
    }

    std::vector<unsigned int> texUnitsDirty;
    std::vector<unsigned int> texUnitsDirtyParams;
};

// We must ensure that cull shader finished filling indirect commands and indirect targets, before draw shader
// starts using them. We use glMemoryBarrier() barrier to achieve that.
// It is also possible that we should use glMemoryBarrier() after resetting textures, but i implemented that only for
// dynamic rendering.
struct InvokeMemoryBarrier : public osg::Drawable::DrawCallback
{
    InvokeMemoryBarrier( GLbitfield barriers )
        : _barriers(barriers)
    {
    }

    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        //DrawIndirectGLExtensions *ext = DrawIndirectGLExtensions::getExtensions( renderInfo.getContextID(), true );
        renderInfo.getState()->get<osg::GLExtensions>()->glMemoryBarrier( _barriers );
        drawable->drawImplementation(renderInfo);
    }
    GLbitfield _barriers;
};

osg::Program* createProgram( const std::string& name, const std::string& vertexSource, const std::string& fragmentSource  )
{
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->setName( name );

    osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);
    vertexShader->setName( name + "_vertex" );
    program->addShader(vertexShader.get());

    osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);
    fragmentShader->setName( name + "_fragment" );
    program->addShader(fragmentShader.get());

    return program.release();
}

float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }

osg::Group* createConiferTree( float detailRatio, const osg::Vec4& leafColor, const osg::Vec4& trunkColor )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::Group> root           = new osg::Group;

    osg::ref_ptr<osg::Cylinder> trunk       = new osg::Cylinder( osg::Vec3( 0.0, 0.0, 1.0 ), 0.25, 2.0 );
    osg::ref_ptr<osg::Geode> trunkGeode     = convertShapeToGeode( *trunk.get(), tessHints.get(), trunkColor );
    root->addChild( trunkGeode.get() );

    osg::ref_ptr<osg::Cone> shapeCone       = new osg::Cone( osg::Vec3( 0.0, 0.0, 4.0 ), 2.0, 8.0 );
    osg::ref_ptr<osg::Geode> shapeGeode     = convertShapeToGeode( *shapeCone.get(), tessHints.get(), leafColor );
    root->addChild( shapeGeode.get() );

    return root.release();
}

// Few functions that create geometries of objects used in example.
// Vertex size in all objects is controlled using single float parameter ( detailRatio )
// Thanks to this ( and "--triangle-modifier" application parameter ) we may experiment with
// triangle quantity of the scene and how it affects the time statistics
osg::Group* createDecidousTree( float detailRatio, const osg::Vec4& leafColor, const osg::Vec4& trunkColor )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::Group> root           = new osg::Group;

    osg::ref_ptr<osg::Cylinder> trunk       = new osg::Cylinder( osg::Vec3( 0.0, 0.0, 1.0 ), 0.4, 2.0 );
    osg::ref_ptr<osg::Geode> trunkGeode     = convertShapeToGeode( *trunk.get(), tessHints.get(), trunkColor );
    root->addChild( trunkGeode.get() );

    osg::ref_ptr<osg::Capsule> shapeCapsule = new osg::Capsule( osg::Vec3( 0.0, 0.0, 7.4 ), 3.0, 5.0 );
    osg::ref_ptr<osg::Geode> shapeGeode     = convertShapeToGeode( *shapeCapsule.get(), tessHints.get(), leafColor );
    root->addChild( shapeGeode.get() );

    return root.release();
}


osg::Group* createSimpleHouse( float detailRatio, const osg::Vec4& buildingColor, const osg::Vec4& chimneyColor )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::Group> root           = new osg::Group;

    osg::ref_ptr<osg::Box> building         = new osg::Box( osg::Vec3( 0.0, 0.0, 8.0 ), 15.0, 9.0, 16.0 );
    osg::ref_ptr<osg::Geode> buildingGeode  = convertShapeToGeode( *building.get(), tessHints.get(), buildingColor );
    root->addChild( buildingGeode.get() );

    // osg::Box always creates geometry with 12 triangles, so to differentiate building LODs we will add three "chimneys"
    {
        osg::ref_ptr<osg::Cylinder> chimney       = new osg::Cylinder( osg::Vec3( -6.0, 3.0, 16.75 ), 0.1, 1.5 );
        osg::ref_ptr<osg::Geode> chimneyGeode     = convertShapeToGeode( *chimney.get(), tessHints.get(), chimneyColor );
        root->addChild( chimneyGeode.get() );
    }

    {
        osg::ref_ptr<osg::Cylinder> chimney       = new osg::Cylinder( osg::Vec3( -5.5, 3.0, 16.5 ), 0.1, 1.0 );
        osg::ref_ptr<osg::Geode> chimneyGeode     = convertShapeToGeode( *chimney.get(), tessHints.get(), chimneyColor );
        root->addChild( chimneyGeode.get() );
    }

    {
        osg::ref_ptr<osg::Cylinder> chimney       = new osg::Cylinder( osg::Vec3( -5.0, 3.0, 16.25 ), 0.1, 0.5 );
        osg::ref_ptr<osg::Geode> chimneyGeode     = convertShapeToGeode( *chimney.get(), tessHints.get(), chimneyColor );
        root->addChild( chimneyGeode.get() );
    }

    return root.release();
}

osg::MatrixTransform* createPropeller( float detailRatio, int propNum, float propRadius, const osg::Vec4& color )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    osg::ref_ptr<osg::Cone> center          = new osg::Cone( osg::Vec3( 0.0, 0.0, 0.05 ), 0.1*propRadius, 0.25*propRadius );
    osg::ref_ptr<osg::Geode> centerGeode    = convertShapeToGeode( *center.get(), tessHints.get(), color );
    osg::ref_ptr<osg::Cone> prop            = new osg::Cone( osg::Vec3( 0.0, 0.0, -0.75*propRadius ), 0.1*propRadius, 1.0*propRadius );
    osg::ref_ptr<osg::Geode> propGeode      = convertShapeToGeode( *prop.get(), tessHints.get(), color );

    root->addChild( centerGeode.get() );
    for( int i=0; i<propNum; ++i )
    {
        float angle = (float)i * 2.0f * osg::PI / (float)propNum;
        osg::ref_ptr<osg::MatrixTransform> propMt = new osg::MatrixTransform( osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0) ) * osg::Matrix::scale(1.0,1.0,0.3) * osg::Matrix::rotate( angle, osg::Vec3(0.0,0.0,1.0) ) );
        propMt->addChild( propGeode.get() );
        root->addChild( propMt.get() );
    }
    return root.release();
}


osg::Group* createBlimp( float detailRatio, const osg::Vec4& hullColor, const osg::Vec4& propColor )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::Group> root           = new osg::Group;
    osg::ref_ptr<osg::Capsule> hull         = new osg::Capsule( osg::Vec3( 0.0, 0.0, 0.0 ), 5.0, 10.0 );
    osg::ref_ptr<osg::Geode> hullGeode      = convertShapeToGeode( *hull.get(), tessHints.get(), hullColor );

    osg::ref_ptr<osg::Capsule> gondola      = new osg::Capsule( osg::Vec3( 5.5, 0.0, 0.0 ), 1.0, 6.0 );
    osg::ref_ptr<osg::Geode> gondolaGeode   = convertShapeToGeode( *gondola.get(), tessHints.get(), hullColor );

    osg::ref_ptr<osg::Box> rudderV          = new osg::Box( osg::Vec3( 0.0, 0.0, -10.0 ), 8.0, 0.3, 4.0 );
    osg::ref_ptr<osg::Geode> rudderVGeode   = convertShapeToGeode( *rudderV.get(), tessHints.get(), hullColor );
    osg::ref_ptr<osg::Box> rudderH          = new osg::Box( osg::Vec3( 0.0, 0.0, -10.0 ), 0.3, 8.0, 4.0 );
    osg::ref_ptr<osg::Geode> rudderHGeode   = convertShapeToGeode( *rudderH.get(), tessHints.get(), hullColor );

    osg::Matrix m;
    m = osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0));
    osg::ref_ptr<osg::MatrixTransform> hullMt = new osg::MatrixTransform(m);
    hullMt->addChild( hullGeode.get() );
    hullMt->addChild( gondolaGeode.get() );
    hullMt->addChild( rudderVGeode.get() );
    hullMt->addChild( rudderHGeode.get() );
    root->addChild( hullMt.get() );

    osg::ref_ptr<osg::MatrixTransform> propellerLeft = createPropeller( detailRatio, 4, 1.0, propColor );
    propellerLeft->setMatrix( osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0)) * osg::Matrix::translate(0.0,2.0,-6.0) );
    propellerLeft->setName("prop0"); root->addChild( propellerLeft.get() );

    osg::ref_ptr<osg::MatrixTransform> propellerRight = createPropeller( detailRatio, 4, 1.0, propColor );
    propellerRight->setMatrix( osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0)) * osg::Matrix::translate(0.0,-2.0,-6.0) );
    propellerRight->setName("prop1"); root->addChild( propellerRight.get() );

    return root.release();
}

osg::Group* createCar( float detailRatio, const osg::Vec4& hullColor, const osg::Vec4& wheelColor )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::Group> root           = new osg::Group;
    osg::ref_ptr<osg::Cylinder> wheel       = new osg::Cylinder( osg::Vec3( 0.0, 0.0, 0.0 ), 1.0, 0.6 );
    osg::ref_ptr<osg::Geometry> wheelGeom   = osg::convertShapeToGeometry( *wheel.get(), tessHints.get(), wheelColor, osg::Array::BIND_PER_VERTEX );
    // one random triangle on every wheel will use black color to show that wheel is rotating
    osg::Vec4Array* colorArray = dynamic_cast<osg::Vec4Array*>( wheelGeom->getColorArray() );
    if(colorArray!=NULL)
    {
        unsigned int triCount = colorArray->size() / 3;
        unsigned int randomTriangle = random(0,triCount);
        for(unsigned int i=0;i<3;++i)
            (*colorArray)[3*randomTriangle+i] = osg::Vec4(0.0,0.0,0.0,1.0);
    }
    osg::ref_ptr<osg::Geode> wheelGeode     = new osg::Geode;
    wheelGeode->addDrawable( wheelGeom.get() );
    osg::ref_ptr<osg::Box> hull             = new osg::Box( osg::Vec3( 0.0, 0.0, 1.3 ), 5.0, 3.0, 1.4 );
    osg::ref_ptr<osg::Geode> hullGeode      = convertShapeToGeode( *hull.get(), tessHints.get(), hullColor );
    root->addChild( hullGeode.get() );

    osg::Matrix m;
    m = osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(2.0,1.8,1.0);
    osg::ref_ptr<osg::MatrixTransform> wheel0Mt = new osg::MatrixTransform(m);
    wheel0Mt->setName("wheel0"); wheel0Mt->addChild( wheelGeode.get() ); root->addChild( wheel0Mt.get() );

    m = osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(-2.0,1.8,1.0);
    osg::ref_ptr<osg::MatrixTransform> wheel1Mt = new osg::MatrixTransform(m);
    wheel1Mt->setName("wheel1"); wheel1Mt->addChild( wheelGeode.get() ); root->addChild( wheel1Mt.get() );

    m = osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(2.0,-1.8,1.0);
    osg::ref_ptr<osg::MatrixTransform> wheel2Mt = new osg::MatrixTransform(m);
    wheel2Mt->setName("wheel2"); wheel2Mt->addChild( wheelGeode.get() ); root->addChild( wheel2Mt.get() );

    m = osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(-2.0,-1.8,1.0);
    osg::ref_ptr<osg::MatrixTransform> wheel3Mt = new osg::MatrixTransform(m);
    wheel3Mt->setName("wheel3"); wheel3Mt->addChild( wheelGeode.get() ); root->addChild( wheel3Mt.get() );

    return root.release();
}

osg::Group* createAirplane( float detailRatio, const osg::Vec4& hullColor, const osg::Vec4& propColor )
{
    osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
    tessHints->setCreateTextureCoords(true);
    tessHints->setDetailRatio(detailRatio);

    osg::ref_ptr<osg::Group> root           = new osg::Group;
    osg::ref_ptr<osg::Capsule> hull         = new osg::Capsule( osg::Vec3( 0.0, 0.0, 0.0 ), 0.8, 6.0 );
    osg::ref_ptr<osg::Geode> hullGeode      = convertShapeToGeode( *hull.get(), tessHints.get(), hullColor );

    osg::ref_ptr<osg::Box> wing0          = new osg::Box( osg::Vec3( 0.4, 0.0, 1.3 ), 0.1, 7.0, 1.6 );
    osg::ref_ptr<osg::Geode> wing0Geode   = convertShapeToGeode( *wing0.get(), tessHints.get(), hullColor );
    osg::ref_ptr<osg::Box> wing1          = new osg::Box( osg::Vec3( -1.4, 0.0, 1.5 ), 0.1, 10.0, 1.8 );
    osg::ref_ptr<osg::Geode> wing1Geode   = convertShapeToGeode( *wing1.get(), tessHints.get(), hullColor );

    osg::ref_ptr<osg::Box> rudderV          = new osg::Box( osg::Vec3( -0.8, 0.0, -3.9 ), 1.5, 0.05, 1.0 );
    osg::ref_ptr<osg::Geode> rudderVGeode   = convertShapeToGeode( *rudderV.get(), tessHints.get(), hullColor );
    osg::ref_ptr<osg::Box> rudderH          = new osg::Box( osg::Vec3( -0.2, 0.0, -3.9 ), 0.05, 4.0, 1.0 );
    osg::ref_ptr<osg::Geode> rudderHGeode   = convertShapeToGeode( *rudderH.get(), tessHints.get(), hullColor );

    osg::Matrix m;
    m = osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0));
    osg::ref_ptr<osg::MatrixTransform> hullMt = new osg::MatrixTransform(m);
    hullMt->addChild( hullGeode.get() );
    hullMt->addChild( wing0Geode.get() );
    hullMt->addChild( wing1Geode.get() );
    hullMt->addChild( rudderVGeode.get() );
    hullMt->addChild( rudderHGeode.get() );
    root->addChild( hullMt.get() );

    osg::ref_ptr<osg::MatrixTransform> propeller = createPropeller( detailRatio, 3, 1.6, propColor );
    propeller->setMatrix( osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0)) * osg::Matrix::translate(3.8,0.0,0.0) );
    propeller->setName("prop0"); root->addChild( propeller.get() );

    return root.release();
}

// createStaticRendering() shows how to use any OSG graph ( whether it is single osg::Geode, or sophisticated osg::PagedLOD tree covering whole earth )
// as a source of  instance data. This way, the OSG graph of arbitrary size is at first culled using typical OSG mechanisms, then remaining osg::Geometries
// are sent to cull shader ( cullProgram ). Cull shader does not draw anything to screen ( thanks to GL_RASTERIZER_DISCARD mode ), but calculates if particular
// instances - sourced from above mentioned osg::Geometries - are visible and what LODs for these instances should be rendered.
// Information about instances is stored into indirect commands ( the number of instances sent to indirect target ) and in indirect targets
// ( static rendering sends all instance data from vertex attributes to indirect target : position, id, extra params, etc ).
// Next the draw shader ( associated with indirect target ) plays its role. The main trick at draw shader invocation is a right use of indirect command.
// Indirect command is bound as a GL_DRAW_INDIRECT_BUFFER and glDrawArraysIndirect() function is called. Thanks to this - proper number
// of instances is rendered without time-consuming GPU->CPU roundtrip. Draw shader renders an aggregate geometry - an osg::Geometry object
// that contains all objects to render ( for specific indirect target ).
void createStaticRendering( osg::Group* root, GPUCullData& gpuData, const osg::Vec2& minArea, const osg::Vec2& maxArea, unsigned int maxNumInstancesPerCell, float lodModifier, float densityModifier, float triangleModifier, bool exportInstanceObjects )
{
    // Creating objects using ShapeToGeometry - its vertex count my vary according to triangleModifier variable.
    // Every object may be stored to file if you want to inspect it ( to show how many vertices it has for example ).
    // To do it - use "--export-objects" parameter in commandline.
    // Every object LOD has different color to show the place where LODs switch.
    osg::ref_ptr<osg::Node> coniferTree0 = createConiferTree( 0.75f * triangleModifier, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec4(0.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*coniferTree0.get(),"coniferTree0.osgt");
    osg::ref_ptr<osg::Node> coniferTree1 = createConiferTree( 0.45f * triangleModifier, osg::Vec4(0.0,0.0,1.0,1.0), osg::Vec4(1.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*coniferTree1.get(),"coniferTree1.osgt");
    osg::ref_ptr<osg::Node> coniferTree2 = createConiferTree( 0.15f * triangleModifier, osg::Vec4(1.0,0.0,0.0,1.0), osg::Vec4(0.0,0.0,1.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*coniferTree2.get(),"coniferTree2.osgt");

    osg::ref_ptr<osg::Node> decidousTree0 = createDecidousTree( 0.75f * triangleModifier, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec4(0.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*decidousTree0.get(),"decidousTree0.osgt");
    osg::ref_ptr<osg::Node> decidousTree1 = createDecidousTree( 0.45f * triangleModifier, osg::Vec4(0.0,0.0,1.0,1.0), osg::Vec4(1.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*decidousTree1.get(),"decidousTree1.osgt");
    osg::ref_ptr<osg::Node> decidousTree2 = createDecidousTree( 0.15f * triangleModifier, osg::Vec4(1.0,0.0,0.0,1.0), osg::Vec4(0.0,0.0,1.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*decidousTree2.get(),"decidousTree2.osgt");

    osg::ref_ptr<osg::Node> simpleHouse0 = createSimpleHouse( 0.75f * triangleModifier, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec4(0.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*simpleHouse0.get(),"simpleHouse0.osgt");
    osg::ref_ptr<osg::Node> simpleHouse1 = createSimpleHouse( 0.45f * triangleModifier, osg::Vec4(0.0,0.0,1.0,1.0), osg::Vec4(1.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*simpleHouse1.get(),"simpleHouse1.osgt");
    osg::ref_ptr<osg::Node> simpleHouse2 = createSimpleHouse( 0.15f * triangleModifier, osg::Vec4(1.0,0.0,0.0,1.0), osg::Vec4(0.0,0.0,1.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*simpleHouse2.get(),"simpleHouse2.osgt");

    // Two indirect targets are registered : the first one renders static objects. The second one is used to render trees waving in the wind.
    gpuData.registerIndirectTarget( 0, new AggregateGeometryVisitor( new ConvertTrianglesOperatorClassic() ), createProgram( "static_draw_0", SHADER_STATIC_DRAW_0_VERTEX, SHADER_STATIC_DRAW_0_FRAGMENT ) );
    gpuData.registerIndirectTarget( 1, new AggregateGeometryVisitor( new ConvertTrianglesOperatorClassic() ), createProgram( "static_draw_1", SHADER_STATIC_DRAW_1_VERTEX, SHADER_STATIC_DRAW_1_FRAGMENT ) );

    float objectDensity[3];
    objectDensity[0] = 10000.0f * densityModifier;
    objectDensity[1] = 1000.0f * densityModifier;
    objectDensity[2] = 100.0f * densityModifier;

    // Three types of instances are registered - each one with three LODs, but first two LODs for trees will be sent to second indirect target
    gpuData.registerType( 0, 1, coniferTree0.get(),  osg::Vec4(0.0f,0.0f,100.0f,110.0f ) * lodModifier,         objectDensity[0] );
    gpuData.registerType( 0, 1, coniferTree1.get(),  osg::Vec4(100.0f,110.0f,500.0f,510.0f) * lodModifier,      objectDensity[0] );
    gpuData.registerType( 0, 0, coniferTree2.get(),  osg::Vec4(500.0f,510.0f,1200.0f,1210.0f) * lodModifier,    objectDensity[0] );
    gpuData.registerType( 1, 1, decidousTree0.get(),  osg::Vec4(0.0f,0.0f,120.0f,130.0f ) * lodModifier,        objectDensity[1] );
    gpuData.registerType( 1, 1, decidousTree1.get(),  osg::Vec4(120.0f,130.0f,600.0f,610.0f) * lodModifier,     objectDensity[1] );
    gpuData.registerType( 1, 0, decidousTree2.get(),  osg::Vec4(600.0f,610.0f,1400.0f,1410.0f) * lodModifier,   objectDensity[1] );
    gpuData.registerType( 2, 0, simpleHouse0.get(),  osg::Vec4(0.0f,0.0f,140.0f,150.0f ) * lodModifier,         objectDensity[2] );
    gpuData.registerType( 2, 0, simpleHouse1.get(),  osg::Vec4(140.0f,150.0f,700.0f,710.0f) * lodModifier,      objectDensity[2] );
    gpuData.registerType( 2, 0, simpleHouse2.get(),  osg::Vec4(700.0f,710.0f,2000.0f,2010.0f) * lodModifier,    objectDensity[2] );
    // every target will store 6 rows of data in GL_RGBA32F_ARB format ( the whole StaticInstance structure )
    gpuData.endRegister(6,GL_RGBA,GL_FLOAT,GL_RGBA32F_ARB);

    // Now we are going to build the tree of instances. Each instance type will be recognized by its TypeID
    // All instances will be placed in area (minArea x maxArea) with densities registered in GPUCullData
    std::vector<StaticInstance> instances;
    osg::BoundingBox bbox( minArea.x(), minArea.y(), 0.0, maxArea.x(), maxArea.y(), 0.0 );
    float fullArea = ( bbox.xMax() - bbox.xMin() ) * ( bbox.yMax() - bbox.yMin() );
    unsigned int currentID = 0;
    // First - all instances are stored in std::vector
    for( unsigned int i=0; i<3; ++i)
    {
        // using LOD area and maximum density - the maximum instance quantity is calculated
        int objectQuantity = (int) floor ( objectDensity[i] * fullArea / 1000000.0f );
        for(int j=0; j<objectQuantity; ++j )
        {
            osg::Vec3 position( random(minArea.x(),maxArea.x()), random(minArea.y(),maxArea.y()), 0.0 );
            float rotation          = random(0.0, 2*osg::PI);
            float scale             = random(0.8, 1.2);
            float brightness        = random(0.5,1.0);
            float wavingAmplitude   = random(0.1,0.5) / 10.0;
            float wavingFrequency   = random(0.2,0.8) * 2.0 * osg::PI;
            float wavingOffset      = random(0.0,1.0) * 2.0 * osg::PI;
            instances.push_back(StaticInstance(i,currentID, osg::Matrixf::scale( scale, scale, scale ) * osg::Matrixf::rotate(rotation, osg::Vec3(0.0,0.0,1.0) ) * osg::Matrixf::translate(position) , osg::Vec4( brightness, wavingAmplitude,wavingFrequency,wavingOffset ) ) );
            currentID++;
        }
    }
    // Second - instance OSG tree is created from above mentioned vector
    osg::BoundingBox allObjectsBbox;
    std::vector<InstanceType>::iterator iit,ieit;
    for(iit=gpuData.instanceTypes->getData().begin(), ieit=gpuData.instanceTypes->getData().end(); iit!=ieit; ++iit)
        allObjectsBbox.expandBy(iit->getBoundingBox());
    osg::ref_ptr<osg::Node> instancesTree = createInstanceTree(instances, allObjectsBbox, maxNumInstancesPerCell);
    if( exportInstanceObjects )
        osgDB::writeNodeFile(*instancesTree.get(),"staticInstancesTree.osgt");
    root->addChild( instancesTree.get() );
    // instance OSG tree is connected to cull shader with all necessary data ( all indirect commands, all
    // indirect targets, necessary OpenGl modes etc. )
    {
        osg::ref_ptr<ResetTexturesCallback> resetTexturesCallback = new ResetTexturesCallback;

        osg::ref_ptr<osg::Program> cullProgram = createProgram( "static_cull", SHADER_STATIC_CULL_VERTEX, SHADER_STATIC_CULL_FRAGMENT );
        cullProgram->addBindUniformBlock("instanceTypesData", 1);
        instancesTree->getOrCreateStateSet()->setAttributeAndModes( cullProgram.get(), osg::StateAttribute::ON );
        instancesTree->getOrCreateStateSet()->setAttributeAndModes( gpuData.instanceTypesUBB.get() );
        instancesTree->getOrCreateStateSet()->setMode( GL_RASTERIZER_DISCARD, osg::StateAttribute::ON );

        std::map<unsigned int, IndirectTarget>::iterator it,eit;
        for(it=gpuData.targets.begin(), eit=gpuData.targets.end(); it!=eit; ++it)
        {
            it->second.addIndirectCommandData( "indirectCommand", it->first, instancesTree->getOrCreateStateSet() );
            resetTexturesCallback->addTextureDirty( it->first );
            resetTexturesCallback->addTextureDirtyParams( it->first );

            it->second.addIndirectTargetData( true, "indirectTarget", it->first, instancesTree->getOrCreateStateSet() );
            resetTexturesCallback->addTextureDirtyParams( OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER+it->first );
        }

        osg::Uniform* indirectCommandSize = new osg::Uniform( osg::Uniform::INT, "indirectCommandSize" );
        indirectCommandSize->set( (int)(sizeof(DrawArraysIndirectCommand) / sizeof(unsigned int))  );
        instancesTree->getOrCreateStateSet()->addUniform( indirectCommandSize );

        instancesTree->getOrCreateStateSet()->setUpdateCallback( resetTexturesCallback.get() );
    }

    // in the end - we create OSG objects that draw instances using indirect targets and commands.
    std::map<unsigned int, IndirectTarget>::iterator it,eit;
    for(it=gpuData.targets.begin(), eit=gpuData.targets.end(); it!=eit; ++it)
    {
        osg::ref_ptr<osg::Geode> drawGeode = new osg::Geode;
        it->second.geometryAggregator->getAggregatedGeometry()->setDrawCallback( new InvokeMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT) );
        drawGeode->addDrawable( it->second.geometryAggregator->getAggregatedGeometry() );
        drawGeode->setCullingActive(false);

        it->second.addIndirectTargetData( false, "indirectTarget", it->first, drawGeode->getOrCreateStateSet() );
        drawGeode->getOrCreateStateSet()->setAttributeAndModes( gpuData.instanceTypesUBB.get() );
        it->second.addDrawProgram( "instanceTypesData", drawGeode->getOrCreateStateSet() );
        drawGeode->getOrCreateStateSet()->setAttributeAndModes( new osg::CullFace( osg::CullFace::BACK ) );

        root->addChild(drawGeode.get());
    }
}

// Dynamic instances are stored in a single osg::Geometry. This geometry holds only a "pointer"
// to texture buffer that contains all info about particular instance. When we animate instances
// we only change information in this texture buffer
osg::Geometry* buildGPUCullGeometry( const std::vector<DynamicInstance>& instances  )
{
    osg::Vec3Array* vertexArray = new osg::Vec3Array;
    osg::Vec4iArray* attrib10   = new osg::Vec4iArray;

    osg::BoundingBox bbox;
    std::vector<DynamicInstance>::const_iterator it,eit;
    for(it=instances.begin(), eit=instances.end(); it!=eit; ++it)
    {
        vertexArray->push_back( it->getPosition() );
        attrib10->push_back( it->idParams );

        bbox.expandBy( it->getPosition() );
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray(vertexArray);
    geom->setVertexAttribArray(10, attrib10, osg::Array::BIND_PER_VERTEX);

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, instances.size() ) );

    geom->setInitialBound( bbox );

    return geom.release();
}

// To animate dynamic objects we use an update callback. It performs some simple
// instance wandering ( object goes to random destination point and when it reaches
// destination, it picks another random point and so on ).
// Object parts are animated ( wheels and propellers )
struct AnimateObjectsCallback : public osg::DrawableUpdateCallback
{
    AnimateObjectsCallback( osg::BufferTemplate< std::vector<DynamicInstance> >* instances, osg::Image* instancesImage, const osg::BoundingBox& bbox, unsigned int quantityPerType )
        : _instances(instances), _instancesImage(instancesImage), _bbox(bbox), _quantityPerType(quantityPerType), _lastTime(0.0)
    {
        _destination = new osg::Vec2Array;
        _destination->reserve(3*_quantityPerType);
        unsigned int i;
        for(i=0; i<3*_quantityPerType; ++i)
            _destination->push_back( osg::Vec2(random( _bbox.xMin(), _bbox.xMax()), random( _bbox.yMin(), _bbox.yMax()) ) );
        i=0;
        for(; i<_quantityPerType; ++i) // speed of blimps
            _speed.push_back( random( 5.0, 10.0) );
        for(; i<2*_quantityPerType; ++i) // speed of cars
            _speed.push_back( random( 1.0, 5.0) );
        for(; i<3*_quantityPerType; ++i) // speed of airplanes
            _speed.push_back( random( 10.0, 16.0 ) );
    }

    virtual void update(osg::NodeVisitor* nv, osg::Drawable* drawable)
    {
        if( nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR )
            return;
        const osg::FrameStamp* frameStamp = nv->getFrameStamp();
        if(frameStamp==NULL)
            return;
        double currentTime = frameStamp->getSimulationTime();
        double deltaTime = 0.016;
        if(_lastTime!=0.0)
            deltaTime = currentTime - _lastTime;
        _lastTime = currentTime;

        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(drawable);
        if(geom==NULL)
            return;
        osg::Vec3Array* vertexArray = dynamic_cast<osg::Vec3Array*>( geom->getVertexArray() );
        if( vertexArray == NULL )
            return;


        osg::BoundingBox nbbox;
        unsigned int i=0;
        for(;i<_quantityPerType;++i) // update blimps
        {
            nbbox.expandBy( updateObjectPosition( vertexArray, i, deltaTime ) );
            // now we update propeler positions
            setRotationUsingRotSpeed( i, 5, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0)) * osg::Matrix::translate(0.0,2.0,-6.0), currentTime, 0.5 );
            setRotationUsingRotSpeed( i, 6, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0)) * osg::Matrix::translate(0.0,-2.0,-6.0), currentTime, -0.5 );
        }

        for(;i<2*_quantityPerType;++i) //update cars
        {
            nbbox.expandBy( updateObjectPosition( vertexArray, i, deltaTime ) );
            // calculate car wheel rotation speed measured in rot/sec
            double wheelRotSpeed = -1.0 * _speed[i] / ( 2.0*osg::PI );
            setRotationUsingRotSpeed( i, 1, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(2.0,1.8,1.0), currentTime, wheelRotSpeed );
            setRotationUsingRotSpeed( i, 2, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(-2.0,1.8,1.0), currentTime, wheelRotSpeed );
            setRotationUsingRotSpeed( i, 3, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(2.0,-1.8,1.0), currentTime, wheelRotSpeed );
            setRotationUsingRotSpeed( i, 4, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::translate(-2.0,-1.8,1.0), currentTime, wheelRotSpeed );
        }

        for(;i<3*_quantityPerType;++i) // update airplanes
        {
            nbbox.expandBy( updateObjectPosition( vertexArray, i, deltaTime ) );
            setRotationUsingRotSpeed( i, 5, osg::Matrix::rotate( osg::DegreesToRadians(90.0), osg::Vec3(0.0,1.0,0.0)) * osg::Matrix::translate(3.8,0.0,0.0), currentTime, 1.5 );
        }
        geom->setInitialBound( nbbox );
        _instancesImage->dirty();
    }

    osg::Vec3 updateObjectPosition( osg::Vec3Array* vertexArray, unsigned int index, float deltaTime )
    {
        osg::Vec3 oldPosition = _instances->getData().at(index).getPosition();

        osg::Vec2 op2(oldPosition.x(), oldPosition.y() );
        while( ( (*_destination)[index]- op2).length()  < 10.0 )
            (*_destination)[index] = osg::Vec2(random( _bbox.xMin(), _bbox.xMax()), random( _bbox.yMin(), _bbox.yMax()) );
        osg::Vec2 direction = (*_destination)[index] - op2;
        direction.normalize();
        osg::Vec2 np2 = direction * deltaTime * _speed[index] ;
        osg::Vec3 newPosition = oldPosition + osg::Vec3( np2.x(), np2.y(), 0.0 );

        osg::Quat heading;
        heading.makeRotate( osg::Vec3(1.0,0.0,0.0), osg::Vec3(direction.x(), direction.y(), 0.0) );
        _instances->getData().at(index).position.setTrans( newPosition );
        _instances->getData().at(index).position.setRotate( heading );
        (*vertexArray)[index] = newPosition;
        return newPosition;
    }

    void setRotationUsingRotSpeed( unsigned int index, unsigned int boneIndex, const osg::Matrix& zeroMatrix, double currentTime, double rotSpeed )
    {
        // setRotationUsingRotSpeed() is a very unoptimally written ( because it uses osg::Matrix::inverse() ),
        // and that is done on purpose : in real life scenario functions making updates may take long time
        // to calculate new object positions due to sophisticated physics models, geometry intersections etc.
        osg::Matrix mRot = osg::Matrix::rotate( fmod( 2.0 * osg::PI * rotSpeed * currentTime,2.0*osg::PI) , osg::Vec3(0.0,0.0,1.0) );
        _instances->getData().at(index).bones[boneIndex] = osg::Matrix::inverse(zeroMatrix) * mRot * zeroMatrix;
    }


    osg::ref_ptr< osg::BufferTemplate< std::vector<DynamicInstance> > > _instances;
    osg::ref_ptr<osg::Image> _instancesImage;
    osg::BoundingBox _bbox;
    unsigned int _quantityPerType;
    osg::ref_ptr<osg::Vec2Array> _destination;
    std::vector<float> _speed;
    double _lastTime;
};

// createDynamicRendering() is similar to earlier createStaticRendering() method. The differences are as follows :
// - instance input data is not stored in osg::Geometry vertex attributes but in dedicated texture buffer ( named "dynamicInstancesData" ). Vertex attributes
//   in osg::Geometry store only a "pointer" to that buffer. This pointer is later sent to indirect target by the cull shader. And then - used by draw
//   shader to get instance data from "dynamicInstancesData" buffer during rendering.
// - draw shader shows how to animate objects using "bones". Each vertex in rendered geometry is associated with single "bone". Such association is
//   sufficient to animate mechanical objects. It is also possible to animate organic objects ( large crowds of people ) but it is far beyond
//   the scope of this example.
void createDynamicRendering( osg::Group* root, GPUCullData& gpuData, osg::BufferTemplate< std::vector<DynamicInstance> >* instances, const osg::Vec2& minArea, const osg::Vec2& maxArea, float lodModifier, float densityModifier, float triangleModifier, bool exportInstanceObjects )
{
    // Creating objects using ShapeToGeometry - its vertex count my vary according to triangleModifier variable.
    // Every object may be stored to file if you want to inspect it ( to show how many vertices it has for example ).
    // To do it - use "--export-objects" parameter in commandline.
    // Every object LOD has different color to show the place where LODs switch.
    osg::ref_ptr<osg::Node> blimpLod0 = createBlimp( 0.75f * triangleModifier, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec4(0.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*blimpLod0.get(),"blimpLod0.osgt");
    osg::ref_ptr<osg::Node> blimpLod1 = createBlimp( 0.45f * triangleModifier, osg::Vec4(0.0,0.0,1.0,1.0), osg::Vec4(1.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*blimpLod1.get(),"blimpLod1.osgt");
    osg::ref_ptr<osg::Node> blimpLod2 = createBlimp( 0.15f * triangleModifier, osg::Vec4(1.0,0.0,0.0,1.0), osg::Vec4(0.0,0.0,1.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*blimpLod2.get(),"blimpLod2.osgt");

    osg::ref_ptr<osg::Node> carLod0 = createCar( 0.75f * triangleModifier, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec4(0.3,0.3,0.3,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*carLod0.get(),"carLod0.osgt");
    osg::ref_ptr<osg::Node> carLod1 = createCar( 0.45f * triangleModifier, osg::Vec4(0.0,0.0,1.0,1.0), osg::Vec4(1.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*carLod1.get(),"carLod1.osgt");
    osg::ref_ptr<osg::Node> carLod2 = createCar( 0.15f * triangleModifier, osg::Vec4(1.0,0.0,0.0,1.0), osg::Vec4(0.0,0.0,1.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*carLod2.get(),"carLod2.osgt");

    osg::ref_ptr<osg::Node> airplaneLod0 = createAirplane( 0.75f * triangleModifier, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec4(0.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*airplaneLod0.get(),"airplaneLod0.osgt");
    osg::ref_ptr<osg::Node> airplaneLod1 = createAirplane( 0.45f * triangleModifier, osg::Vec4(0.0,0.0,1.0,1.0), osg::Vec4(1.0,1.0,0.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*airplaneLod1.get(),"airplaneLod1.osgt");
    osg::ref_ptr<osg::Node> airplaneLod2 = createAirplane( 0.15f * triangleModifier, osg::Vec4(1.0,0.0,0.0,1.0), osg::Vec4(0.0,0.0,1.0,1.0) );
    if( exportInstanceObjects ) osgDB::writeNodeFile(*airplaneLod2.get(),"airplaneLod2.osgt");

    // ConvertTrianglesOperatorClassic struct is informed which node names correspond to which bone indices
    ConvertTrianglesOperatorClassic* target0Converter = new ConvertTrianglesOperatorClassic();
    target0Converter->registerBoneByName("wheel0",1);
    target0Converter->registerBoneByName("wheel1",2);
    target0Converter->registerBoneByName("wheel2",3);
    target0Converter->registerBoneByName("wheel3",4);
    target0Converter->registerBoneByName("prop0",5);
    target0Converter->registerBoneByName("prop1",6);
    // dynamic rendering uses only one indirect target in this example
    gpuData.registerIndirectTarget( 0, new AggregateGeometryVisitor( target0Converter ), createProgram( "dynamic_draw_0", SHADER_DYNAMIC_DRAW_0_VERTEX, SHADER_DYNAMIC_DRAW_0_FRAGMENT ) );

    float objectDensity = 100.0f * densityModifier;
    // all instance types are registered and will render using the same indirect target
    gpuData.registerType( 0, 0, blimpLod0.get(),    osg::Vec4(0.0f,0.0f,150.0f,160.0f) * lodModifier,       objectDensity );
    gpuData.registerType( 0, 0, blimpLod1.get(),    osg::Vec4(150.0f,160.0f,800.0f,810.0f) * lodModifier,   objectDensity );
    gpuData.registerType( 0, 0, blimpLod2.get(),    osg::Vec4(800.0f,810.0f,6500.0f,6510.0f) * lodModifier, objectDensity );
    gpuData.registerType( 1, 0, carLod0.get(),      osg::Vec4(0.0f,0.0f,50.0f,60.0f) * lodModifier,         objectDensity );
    gpuData.registerType( 1, 0, carLod1.get(),      osg::Vec4(50.0f,60.0f,300.0f,310.0f) * lodModifier,     objectDensity );
    gpuData.registerType( 1, 0, carLod2.get(),      osg::Vec4(300.0f,310.0f,1000.0f,1010.0f) * lodModifier, objectDensity );
    gpuData.registerType( 2, 0, airplaneLod0.get(), osg::Vec4(0.0f,0.0f,80.0f,90.0f) * lodModifier,         objectDensity );
    gpuData.registerType( 2, 0, airplaneLod1.get(), osg::Vec4(80.0f,90.0f,400.0f,410.0f) * lodModifier,     objectDensity );
    gpuData.registerType( 2, 0, airplaneLod2.get(), osg::Vec4(400.0f,410.0f,1200.0f,1210.0f) * lodModifier, objectDensity );
    // dynamic targets store only a "pointer" to instanceTarget buffer
    gpuData.endRegister(1,GL_RGBA,GL_INT, GL_RGBA32I_EXT);

    // add dynamic instances to instances container
    float objectZ[3];
    objectZ[0] = 50.0f;
    objectZ[1] = 0.0f;
    objectZ[2] = 20.0f;

    osg::BoundingBox bbox( minArea.x(), minArea.y(), 0.0, maxArea.x(), maxArea.y(), 0.0 );
    float fullArea = ( bbox.xMax() - bbox.xMin() ) * ( bbox.yMax() - bbox.yMin() );
    unsigned int objectQuantity = (unsigned int) floor ( objectDensity * fullArea / 1000000.0f );
    unsigned int currentID = 0;

    for( unsigned int i=0; i<3; ++i)
    {
        for(unsigned int j=0; j<objectQuantity; ++j )
        {
            osg::Vec3 position( random(minArea.x(),maxArea.x()), random(minArea.y(),maxArea.y()), objectZ[i] );
            float rotation      = random(0.0, 2*osg::PI);
            float brightness    = random(0.25,1.0);
            instances->getData().push_back(DynamicInstance(i,currentID, osg::Matrixf::rotate(rotation, osg::Vec3(0.0,0.0,1.0) ) * osg::Matrixf::translate(position) , osg::Vec4( brightness, 0.0,0.0,0.0 ) ) );
            currentID++;
        }
    }

    // all data about instances is stored in texture buffer ( compare it with static rendering )
    osg::Image* instancesImage = new osg::Image;
    instancesImage->setImage( instances->getTotalDataSize() / sizeof(osg::Vec4f), 1, 1, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT, (unsigned char*)instances->getDataPointer(), osg::Image::NO_DELETE );

    osg::VertexBufferObject *instancesBuffer =new osg::VertexBufferObject;
    instancesBuffer->setUsage(GL_STATIC_DRAW);
    instancesImage->setBufferObject(instancesBuffer);

    osg::TextureBuffer* instancesTextureBuffer = new osg::TextureBuffer(instancesImage);
    instancesTextureBuffer->setUnRefImageDataAfterApply(false);

    osg::Uniform* dynamicInstancesDataUniform = new osg::Uniform( "dynamicInstancesData", 8 );
    osg::Uniform* dynamicInstancesDataSize = new osg::Uniform( osg::Uniform::INT, "dynamicInstancesDataSize" );
    dynamicInstancesDataSize->set( (int)(sizeof(DynamicInstance) / sizeof(osg::Vec4f)) );

    // all instance "pointers" are stored in a single geometry rendered with cull shader
    osg::ref_ptr<osg::Geometry> instanceGeometry = buildGPUCullGeometry( instances->getData()  );
    osg::ref_ptr<osg::Geode> instanceGeode = new osg::Geode;
    instanceGeode->addDrawable(instanceGeometry.get());
    if( exportInstanceObjects )
        osgDB::writeNodeFile(*instanceGeode.get(),"dynamicInstanceGeode.osgt");

    // update callback that animates dynamic objects
    instanceGeometry->setUpdateCallback( new AnimateObjectsCallback( instances, instancesImage, bbox, objectQuantity ) );
    instanceGeometry->setDrawCallback( new InvokeMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT) );
    root->addChild( instanceGeode.get() );
    // instance geode is connected to cull shader with all necessary data ( all indirect commands, all
    // indirect targets, necessary OpenGL modes etc. )
    {

        instanceGeode->getOrCreateStateSet()->setAttributeAndModes( gpuData.instanceTypesUBB.get() );
        osg::ref_ptr<ResetTexturesCallback> resetTexturesCallback = new ResetTexturesCallback;
        osg::ref_ptr<osg::Program> cullProgram = createProgram( "dynamic_cull", SHADER_DYNAMIC_CULL_VERTEX, SHADER_DYNAMIC_CULL_FRAGMENT );
        cullProgram->addBindUniformBlock("instanceTypesData", 1);
        instanceGeode->getOrCreateStateSet()->setAttributeAndModes( cullProgram.get(), osg::StateAttribute::ON );

        instanceGeode->getOrCreateStateSet()->setMode( GL_RASTERIZER_DISCARD, osg::StateAttribute::ON );

        instanceGeode->getOrCreateStateSet()->setTextureAttribute( 8, instancesTextureBuffer );
        instanceGeode->getOrCreateStateSet()->addUniform( dynamicInstancesDataUniform );
        instanceGeode->getOrCreateStateSet()->addUniform( dynamicInstancesDataSize );

        std::map<unsigned int, IndirectTarget>::iterator it,eit;
        for(it=gpuData.targets.begin(), eit=gpuData.targets.end(); it!=eit; ++it)
        {
            it->second.addIndirectCommandData( "indirectCommand", it->first, instanceGeode->getOrCreateStateSet() );
            resetTexturesCallback->addTextureDirty( it->first );
            resetTexturesCallback->addTextureDirtyParams( it->first );

            it->second.addIndirectTargetData( true, "indirectTarget", it->first, instanceGeode->getOrCreateStateSet() );
            resetTexturesCallback->addTextureDirtyParams( OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER+it->first );
        }

        osg::Uniform* indirectCommandSize = new osg::Uniform( osg::Uniform::INT, "indirectCommandSize" );
        indirectCommandSize->set( (int)(sizeof(DrawArraysIndirectCommand) / sizeof(unsigned int))  );
        instanceGeode->getOrCreateStateSet()->addUniform( indirectCommandSize );

        instanceGeode->getOrCreateStateSet()->setUpdateCallback( resetTexturesCallback.get() );
    }

    // in the end - we create OSG objects that draw instances using indirect targets and commands.
    std::map<unsigned int, IndirectTarget>::iterator it,eit;
    for(it=gpuData.targets.begin(), eit=gpuData.targets.end(); it!=eit; ++it)
    {
        osg::ref_ptr<osg::Geode> drawGeode = new osg::Geode;
        it->second.geometryAggregator->getAggregatedGeometry()->setDrawCallback( new InvokeMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT) );
        drawGeode->addDrawable( it->second.geometryAggregator->getAggregatedGeometry() );
        drawGeode->setCullingActive(false);

        drawGeode->getOrCreateStateSet()->setTextureAttribute( 8, instancesTextureBuffer );
        drawGeode->getOrCreateStateSet()->addUniform( dynamicInstancesDataUniform );
        drawGeode->getOrCreateStateSet()->addUniform( dynamicInstancesDataSize );

        it->second.addIndirectTargetData( false, "indirectTarget", it->first, drawGeode->getOrCreateStateSet() );
        drawGeode->getOrCreateStateSet()->setAttributeAndModes( gpuData.instanceTypesUBB.get() );
        it->second.addDrawProgram( "instanceTypesData", drawGeode->getOrCreateStateSet() );
        drawGeode->getOrCreateStateSet()->setAttributeAndModes( new osg::CullFace( osg::CullFace::BACK ) );

        root->addChild(drawGeode.get());
    }
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates two-phase geometry instancing using GPU to cull and lod objects");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] ");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--skip-static","Skip rendering of static objects");
    arguments.getApplicationUsage()->addCommandLineOption("--skip-dynamic","Skip rendering of dynamic objects");
    arguments.getApplicationUsage()->addCommandLineOption("--export-objects","Export instance objects to files");
    arguments.getApplicationUsage()->addCommandLineOption("--use-multi-draw","Use glMultiDrawArraysIndirect in draw shader. Requires OpenGL version 4.3.");
    arguments.getApplicationUsage()->addCommandLineOption("--instances-per-cell","How many static instances per cell = 4096");
    arguments.getApplicationUsage()->addCommandLineOption("--static-area-size","Size of the area for static rendering = 2000");
    arguments.getApplicationUsage()->addCommandLineOption("--dynamic-area-size","Size of the area for dynamic rendering = 1000");
    arguments.getApplicationUsage()->addCommandLineOption("--lod-modifier","LOD range [%] = 100");
    arguments.getApplicationUsage()->addCommandLineOption("--density-modifier","Instance density [%] = 100");
    arguments.getApplicationUsage()->addCommandLineOption("--triangle-modifier","Instance triangle quantity [%] = 100");

    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // application configuration - default values
    bool showStaticRendering        = true;
    bool showDynamicRendering       = true;
    bool exportInstanceObjects      = false;
    bool useMultiDrawArraysIndirect = false;
    unsigned int instancesPerCell   = 4096;
    float staticAreaSize            = 2000.0f;
    float dynamicAreaSize           = 1000.0f;
    float lodModifier               = 100.0f;
    float densityModifier           = 100.0f;
    float triangleModifier          = 100.0f;

    if ( arguments.read("--skip-static") )
        showStaticRendering = false;

    if ( arguments.read("--skip-dynamic") )
        showDynamicRendering = false;

    if ( arguments.read("--export-objects") )
        exportInstanceObjects = true;

    if ( arguments.read("--use-multi-draw") )
        useMultiDrawArraysIndirect = true;

    arguments.read("--instances-per-cell",instancesPerCell);
    arguments.read("--static-area-size",staticAreaSize);
    arguments.read("--dynamic-area-size",dynamicAreaSize);
    arguments.read("--lod-modifier",lodModifier);
    arguments.read("--density-modifier",densityModifier);
    arguments.read("--triangle-modifier",triangleModifier);

    lodModifier /= 100.0f;
    densityModifier /= 100.0f;
    triangleModifier /= 100.0f;

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);
    // To enable proper LOD fading we use multisampling
    osg::DisplaySettings::instance()->setNumMultiSamples( 4 );

    // Add basic event handlers and manipulators
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

    keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
    keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
    keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
    keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
    viewer.setCameraManipulator( keyswitchManipulator.get() );

    // create root node
    osg::ref_ptr<osg::Group> root = new osg::Group;

    // add lightsource to root node
    osg::ref_ptr<osg::LightSource> lSource = new osg::LightSource;
    lSource->getLight()->setPosition(osg::Vec4(1.0,-1.0,1.0,0.0));
    lSource->getLight()->setDiffuse(osg::Vec4(0.9,0.9,0.9,1.0));
    lSource->getLight()->setAmbient(osg::Vec4(0.1,0.1,0.1,1.0));
    root->addChild( lSource.get() );

    // Add ground
    osg::ref_ptr<osg::Geometry> groundGeometry = createTexturedQuadGeometry( osg::Vec3(-0.5*staticAreaSize,-0.5*staticAreaSize,0.0), osg::Vec3(0.0,staticAreaSize,0.0), osg::Vec3(staticAreaSize,0.0,0.0) );
    osg::ref_ptr<osg::Geode> groundGeode = new osg::Geode;
    groundGeode->addDrawable( groundGeometry.get() );
    root->addChild( groundGeode.get() );

    // Uniform for trees waving in the wind
    osg::Vec2 windDirection( random(-1.0,1.0), random(-1.0,1.0) );
    windDirection.normalize();
    osg::ref_ptr<osg::Uniform> windDirectionUniform = new osg::Uniform("windDirection", windDirection);
    root->getOrCreateStateSet()->addUniform( windDirectionUniform.get() );

    // create static objects and setup its rendering
    GPUCullData staticGpuData;
    staticGpuData.setUseMultiDrawArraysIndirect( useMultiDrawArraysIndirect );
    if(showStaticRendering)
    {
        createStaticRendering( root.get(), staticGpuData, osg::Vec2(-0.5*staticAreaSize,-0.5*staticAreaSize), osg::Vec2(0.5*staticAreaSize,0.5*staticAreaSize), instancesPerCell, lodModifier, densityModifier, triangleModifier, exportInstanceObjects );
    }

    // create dynamic objects and setup its rendering
    GPUCullData dynamicGpuData;
    dynamicGpuData.setUseMultiDrawArraysIndirect( useMultiDrawArraysIndirect );
    osg::ref_ptr< osg::BufferTemplate< std::vector< DynamicInstance> > > dynamicInstances;
    if(showDynamicRendering)
    {
        dynamicInstances = new osg::BufferTemplate< std::vector<DynamicInstance> >();
        createDynamicRendering( root.get(), dynamicGpuData, dynamicInstances.get(), osg::Vec2(-0.5*dynamicAreaSize,-0.5*dynamicAreaSize), osg::Vec2(0.5*dynamicAreaSize,0.5*dynamicAreaSize), lodModifier, densityModifier, triangleModifier, exportInstanceObjects );
    }

    viewer.setSceneData( root.get() );

    viewer.realize();

    // shaders use osg_ variables so we must do the following
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
        (*itr)->getState()->setUseModelViewAndProjectionUniforms(true);

    return viewer.run();
}
