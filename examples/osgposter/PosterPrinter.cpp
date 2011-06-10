#include <osg/ClusterCullingCallback>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <string.h>
#include <iostream>
#include <sstream>
#include "PosterPrinter.h"

/* PagedLoadingCallback: Callback for loading paged nodes while doing intersecting test */
struct PagedLoadingCallback : public osgUtil::IntersectionVisitor::ReadCallback
{
    virtual osg::Node* readNodeFile( const std::string& filename )
    {
        return osgDB::readNodeFile( filename );
    }
};
static osg::ref_ptr<PagedLoadingCallback> g_pagedLoadingCallback = new PagedLoadingCallback;

/* LodCullingCallback: Callback for culling LODs and selecting the highest level */
class LodCullingCallback : public osg::NodeCallback
{
public:
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osg::LOD* lod = static_cast<osg::LOD*>(node);
        if ( lod && lod->getNumChildren()>0 )
            lod->getChild(lod->getNumChildren()-1)->accept(*nv);
    }
};
static osg::ref_ptr<LodCullingCallback> g_lodCullingCallback = new LodCullingCallback;

/* PagedCullingCallback: Callback for culling paged nodes and selecting the highest level */
class PagedCullingCallback : public osg::NodeCallback
{
public:
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osg::PagedLOD* pagedLOD = static_cast<osg::PagedLOD*>(node);
        if ( pagedLOD && pagedLOD->getNumChildren()>0 )
        {
            unsigned int numChildren = pagedLOD->getNumChildren();
            bool updateTimeStamp = nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR;
            if ( nv->getFrameStamp() && updateTimeStamp )
            {
                double timeStamp = nv->getFrameStamp()?nv->getFrameStamp()->getReferenceTime():0.0;
                unsigned int frameNumber = nv->getFrameStamp()?nv->getFrameStamp()->getFrameNumber():0;
                
                pagedLOD->setFrameNumberOfLastTraversal( frameNumber );
                pagedLOD->setTimeStamp( numChildren-1, timeStamp );
                pagedLOD->setFrameNumber( numChildren-1, frameNumber );
                pagedLOD->getChild(numChildren-1)->accept(*nv);
            }
            
            // Request for new child
            if ( !pagedLOD->getDisableExternalChildrenPaging() &&
                 nv->getDatabaseRequestHandler() &&
                 numChildren<pagedLOD->getNumRanges() )
            {
                if ( pagedLOD->getDatabasePath().empty() )
                {
                    nv->getDatabaseRequestHandler()->requestNodeFile(
                        pagedLOD->getFileName(numChildren), nv->getNodePath(),
                        1.0, nv->getFrameStamp(),
                        pagedLOD->getDatabaseRequest(numChildren), pagedLOD->getDatabaseOptions() );
                }
                else
                {
                    nv->getDatabaseRequestHandler()->requestNodeFile(
                        pagedLOD->getDatabasePath()+pagedLOD->getFileName(numChildren), nv->getNodePath(),
                        1.0, nv->getFrameStamp(),
                        pagedLOD->getDatabaseRequest(numChildren), pagedLOD->getDatabaseOptions() );
                }
            }
        }
        //node->traverse(*nv);
    }
};
static osg::ref_ptr<PagedCullingCallback> g_pagedCullingCallback = new PagedCullingCallback;

/* PosterVisitor: A visitor for adding culling callbacks to newly allocated paged nodes */
PosterVisitor::PosterVisitor()
:   osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _appliedCount(0), _needToApplyCount(0),
    _addingCallbacks(true)
{
}

void PosterVisitor::apply( osg::LOD& node )
{
    /*if ( !hasCullCallback(node.getCullCallback(), g_lodCullingCallback.get()) )
    {
        if ( !node.getName().empty() )
        {
            PagedNodeNameSet::iterator itr = _pagedNodeNames.find( node.getName() );
            if ( itr!=_pagedNodeNames.end() )
            {
                insertCullCallback( node, g_lodCullingCallback.get() );
                _appliedCount++;
            }
        }
    }
    else if ( !_addingCallbacks )
    {
        node.removeCullCallback( g_lodCullingCallback.get() );
        _appliedCount--;
    }*/
    traverse( node );
}

void PosterVisitor::apply( osg::PagedLOD& node )
{
    if ( !hasCullCallback(node.getCullCallback(), g_pagedCullingCallback.get()) )
    {
        for ( unsigned int i=0; i<node.getNumFileNames(); ++i )
        {
            if ( node.getFileName(i).empty() ) continue;
            
            PagedNodeNameSet::iterator itr = _pagedNodeNames.find( node.getFileName(i) );
            if ( itr!=_pagedNodeNames.end() )
            {
                node.addCullCallback( g_pagedCullingCallback.get() );
                _appliedCount++;
            }
            break;
        }
    }
    else if ( !_addingCallbacks )
    {
        node.removeCullCallback( g_pagedCullingCallback.get() );
        if ( _appliedCount>0 ) _appliedCount--;
    }
    traverse( node );
}

/* PosterIntersector: A simple polytope intersector for updating pagedLODs in each image-tile */
PosterIntersector::PosterIntersector( const osg::Polytope& polytope )
:   _intersectionVisitor(0), _parent(0), _polytope(polytope)
{}

PosterIntersector::PosterIntersector( double xMin, double yMin, double xMax, double yMax )
:   Intersector(osgUtil::Intersector::PROJECTION),
    _intersectionVisitor(0), _parent(0)
{
    _polytope.add( osg::Plane( 1.0, 0.0, 0.0,-xMin) );
    _polytope.add( osg::Plane(-1.0, 0.0, 0.0, xMax) );
    _polytope.add( osg::Plane( 0.0, 1.0, 0.0,-yMin) );
    _polytope.add( osg::Plane( 0.0,-1.0, 0.0, yMax) );
}

osgUtil::Intersector* PosterIntersector::clone( osgUtil::IntersectionVisitor& iv )
{
    osg::Matrix matrix;
    if ( iv.getProjectionMatrix() ) matrix.preMult( *iv.getProjectionMatrix() );
    if ( iv.getViewMatrix() ) matrix.preMult( *iv.getViewMatrix() );
    if ( iv.getModelMatrix() ) matrix.preMult( *iv.getModelMatrix() );
    
    osg::Polytope transformedPolytope;
    transformedPolytope.setAndTransformProvidingInverse( _polytope, matrix );
    
    osg::ref_ptr<PosterIntersector> pi = new PosterIntersector( transformedPolytope );
    pi->_intersectionVisitor = &iv;
    pi->_parent = this;
    return pi.release();
}

bool PosterIntersector::enter( const osg::Node& node )
{
    if ( !node.isCullingActive() ) return true;
    if ( _polytope.contains(node.getBound()) )
    {
        if ( node.getCullCallback() )
        {
            const osg::ClusterCullingCallback* cccb =
                dynamic_cast<const osg::ClusterCullingCallback*>( node.getCullCallback() );
            if ( cccb && cccb->cull(_intersectionVisitor, 0, NULL) ) return false;
        }
        return true;
    }
    return false;
}

void PosterIntersector::reset()
{
    _intersectionVisitor = NULL;
    Intersector::reset();
}

void PosterIntersector::intersect( osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable )
{
    if ( !_polytope.contains(drawable->getBound()) ) return;
    if ( iv.getDoDummyTraversal() ) return;
    
    // Find and collect all paged LODs in the node path
    osg::NodePath& nodePath = iv.getNodePath();
    for ( osg::NodePath::iterator itr=nodePath.begin(); itr!=nodePath.end(); ++itr )
    {
        osg::PagedLOD* pagedLOD = dynamic_cast<osg::PagedLOD*>(*itr);
        if ( pagedLOD )
        {
            // FIXME: The first non-empty getFileName() is used as the identity of this paged node.
            // This should work with VPB-generated terrains but maybe unusable with others.
            for ( unsigned int i=0; i<pagedLOD->getNumFileNames(); ++i )
            {
                if ( pagedLOD->getFileName(i).empty() ) continue;
                if ( _parent->_visitor.valid() )
                    _parent->_visitor->insertName( pagedLOD->getFileName(i) );
                break;
            }
            continue;
        }
        
        /*osg::LOD* lod = dynamic_cast<osg::LOD*>(*itr);
        if ( lod )
        {
            if ( !lod->getName().empty() && _parent->_visitor.valid() )
                _parent->_visitor->insertName( lod->getName() );
        }*/
    }
}

/* PosterPrinter: The implementation class of high-res rendering */
PosterPrinter::PosterPrinter():
    _outputTiles(false), _outputTileExt("bmp"),
    _isRunning(false), _isFinishing(false), _lastBindingFrame(0),
    _currentRow(0), _currentColumn(0),
    _camera(0), _finalPoster(0)
{
    _intersector = new PosterIntersector(-1.0, -1.0, 1.0, 1.0);
    _visitor = new PosterVisitor;
    _intersector->setPosterVisitor( _visitor.get() );
}

void PosterPrinter::init( const osg::Camera* camera )
{
    if ( _camera.valid() )
        init( camera->getViewMatrix(), camera->getProjectionMatrix() );
}

void PosterPrinter::init( const osg::Matrixd& view, const osg::Matrixd& proj )
{
    if ( _isRunning ) return;
    _images.clear();
    _visitor->clearNames();
    _tileRows = (int)(_posterSize.y() / _tileSize.y());
    _tileColumns = (int)(_posterSize.x() / _tileSize.x());
    _currentRow = 0;
    _currentColumn = 0;
    _currentViewMatrix = view;
    _currentProjectionMatrix = proj;
    _lastBindingFrame = 0;
    _isRunning = true;
    _isFinishing = false;
}

void PosterPrinter::frame( const osg::FrameStamp* fs, osg::Node* node )
{
    // Add cull callbacks to all existing paged nodes,
    // and advance frame when all callbacks are dispatched.
    if ( addCullCallbacks(fs, node) )
        return;
    
    if ( _isFinishing )
    {
        if ( (fs->getFrameNumber()-_lastBindingFrame)>2 )
        {
            // Record images and the final poster
            recordImages();
            if ( _finalPoster.valid() )
            {
                std::cout << "Writing final result to file..." << std::endl;
                osgDB::writeImageFile( *_finalPoster, _outputPosterName );
            }
            
            // Release all cull callbacks to free unused paged nodes
            removeCullCallbacks( node );
            _visitor->clearNames();
            
            _isFinishing = false;
            std::cout << "Recording images finished." << std::endl;
        }
    }
    
    if ( _isRunning )
    {
        // Every "copy-to-image" process seems to be finished in 2 frames.
        // So record them and dispatch camera to next tiles.
        if ( (fs->getFrameNumber()-_lastBindingFrame)>2 )
        {
            // Record images and unref them to free memory
            recordImages();
            
            // Release all cull callbacks to free unused paged nodes
            removeCullCallbacks( node );
            _visitor->clearNames();
            
            if ( _camera.valid() )
            {
                std::cout << "Binding sub-camera " << _currentRow << "_" << _currentColumn
                          << " to image..." << std::endl;
                bindCameraToImage( _camera.get(), _currentRow, _currentColumn );
                if ( _currentColumn<_tileColumns-1 )
                {
                    _currentColumn++;
                }
                else
                {
                    if ( _currentRow<_tileRows-1 )
                    {
                        _currentRow++;
                        _currentColumn = 0;
                    }
                    else
                    {
                        _isRunning = false;
                        _isFinishing = true;
                    }
                }
            }
            _lastBindingFrame = fs->getFrameNumber();
        }
    }
}

bool PosterPrinter::addCullCallbacks( const osg::FrameStamp* fs, osg::Node* node )
{
    if ( !_visitor->inQueue() || done() )
        return false;
    
    _visitor->setAddingCallbacks( true );
    _camera->accept( *_visitor );
    _lastBindingFrame = fs->getFrameNumber();
    
    std::cout << "Dispatching callbacks to paged nodes... "
              << _visitor->inQueue() << std::endl;
    return true;
}

void PosterPrinter::removeCullCallbacks( osg::Node* node )
{
    _visitor->setAddingCallbacks( false );
    _camera->accept( *_visitor );
}

void PosterPrinter::bindCameraToImage( osg::Camera* camera, int row, int col )
{
    std::stringstream stream;
    stream << "image_" << row << "_" << col;
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->setName( stream.str() );
    image->allocateImage( (int)_tileSize.x(), (int)_tileSize.y(), 1, GL_RGBA, GL_UNSIGNED_BYTE );
    _images[TilePosition(row,col)] = image.get();
    
    // Calculate projection matrix offset of each tile
    osg::Matrix offsetMatrix =
        osg::Matrix::scale(_tileColumns, _tileRows, 1.0) *
        osg::Matrix::translate(_tileColumns-1-2*col, _tileRows-1-2*row, 0.0);
    camera->setViewMatrix( _currentViewMatrix );
    camera->setProjectionMatrix( _currentProjectionMatrix * offsetMatrix );
    
    // Check intersections between the image-tile box and the model
    osgUtil::IntersectionVisitor iv( _intersector.get() );
    iv.setReadCallback( g_pagedLoadingCallback.get() );
    _intersector->reset();
    camera->accept( iv );
    if ( _intersector->containsIntersections() )
    {
        // Apply a cull calback to every paged node obtained, to force the highest level displaying.
        // This will be done by the PosterVisitor, who already records all the paged nodes.
    }
    
    // Reattach cameras and new allocated images
    camera->setRenderingCache( NULL );  // FIXME: Uses for reattaching camera with image, maybe inefficient?
    camera->detach( osg::Camera::COLOR_BUFFER );
    camera->attach( osg::Camera::COLOR_BUFFER, image.get(), 0, 0 );
}

void PosterPrinter::recordImages()
{
    for ( TileImages::iterator itr=_images.begin(); itr!=_images.end(); ++itr )
    {
        osg::Image* image = (itr->second).get();
        if ( _finalPoster.valid() )
        {
            // FIXME: A stupid way to combine tile images to final result. Any better ideas?
            unsigned int row = itr->first.first, col = itr->first.second;
            for ( int t=0; t<image->t(); ++t )
            {
                unsigned char* source = image->data( 0, t );
                unsigned char* target = _finalPoster->data( col*(int)_tileSize.x(), t + row*(int)_tileSize.y() );
                memcpy( target, source, image->s() * 4 * sizeof(unsigned char) );
            }
        }
        
        if ( _outputTiles )
            osgDB::writeImageFile( *image, image->getName()+"."+_outputTileExt );
    }
    _images.clear();
}
