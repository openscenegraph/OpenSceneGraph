#ifndef OSGPOSTER_POSTERPRINTER
#define OSGPOSTER_POSTERPRINTER

#include <osg/Camera>
#include <osg/PagedLOD>
#include <osgUtil/IntersectionVisitor>

/** PosterVisitor: A visitor for adding culling callbacks to newly allocated paged nodes */
class PosterVisitor : public osg::NodeVisitor
{
public:
    typedef std::set<std::string> PagedNodeNameSet;
    
    PosterVisitor();
    META_NodeVisitor( osgPoster, PosterVisitor );
    
    void insertName( const std::string& name )
    { if ( _pagedNodeNames.insert(name).second ) _needToApplyCount++; }
    
    void eraseName( const std::string& name )
    { if ( _pagedNodeNames.erase(name)>0 ) _needToApplyCount--; }
    
    void clearNames() { _pagedNodeNames.clear(); _needToApplyCount = 0; _appliedCount = 0; }
    unsigned int getNumNames() const { return _pagedNodeNames.size(); }
    
    PagedNodeNameSet& getPagedNodeNames() { return _pagedNodeNames; }
    const PagedNodeNameSet& getPagedNodeNames() const { return _pagedNodeNames; }
    
    unsigned int getNeedToApplyCount() const { return _needToApplyCount; }
    unsigned int getAppliedCount() const { return _appliedCount; }
    unsigned int inQueue() const { return _needToApplyCount>_appliedCount ? _needToApplyCount-_appliedCount : 0; }
    
    void setAddingCallbacks( bool b ) { _addingCallbacks = b; }
    bool getAddingCallbacks() const { return _addingCallbacks; }
    
    virtual void apply( osg::LOD& node );
    virtual void apply( osg::PagedLOD& node );
    
protected:
    bool hasCullCallback( osg::NodeCallback* nc, osg::NodeCallback* target )
    {
        if ( nc==target ) return true;
        else if ( !nc ) return false;
        return hasCullCallback( nc->getNestedCallback(), target );
    }
    
    PagedNodeNameSet _pagedNodeNames;
    unsigned int _appliedCount;
    unsigned int _needToApplyCount;
    bool _addingCallbacks;
};

/** PosterIntersector: A simple polytope intersector for updating pagedLODs in each image-tile */
class PosterIntersector : public osgUtil::Intersector
{
public:
    typedef std::set<std::string> PagedNodeNameSet;
    
    PosterIntersector( const osg::Polytope& polytope );
    PosterIntersector( double xMin, double yMin, double xMax, double yMax );
    
    void setPosterVisitor( PosterVisitor* pcv ) { _visitor = pcv; }
    PosterVisitor* getPosterVisitor() { return _visitor.get(); }
    const PosterVisitor* getPosterVisitor() const { return _visitor.get(); }
    
    virtual Intersector* clone( osgUtil::IntersectionVisitor& iv );
    
    virtual bool containsIntersections()
    { return _visitor.valid()&&_visitor->getNumNames()>0; }
    
    virtual bool enter( const osg::Node& node );
    virtual void leave() {}
    virtual void reset();
    virtual void intersect( osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable );
    
protected:
    osgUtil::IntersectionVisitor* _intersectionVisitor;
    osg::ref_ptr<PosterVisitor> _visitor;
    PosterIntersector* _parent;
    osg::Polytope _polytope;
};

/** PosterPrinter: The implementation class of high-res rendering */
class PosterPrinter : public osg::Referenced
{
public:
    typedef std::pair<unsigned int, unsigned int> TilePosition;
    typedef std::map< TilePosition, osg::ref_ptr<osg::Image> > TileImages;
    
    PosterPrinter();
    
    /** Set to output each sub-image-tile to disk */
    void setOutputTiles( bool b ) { _outputTiles = b; }
    bool getOutputTiles() const { return _outputTiles; }
    
    /** Set the output sub-image-tile extension, e.g. bmp */
    void setOutputTileExtension( const std::string& ext ) { _outputTileExt = ext; }
    const std::string& getOutputTileExtension() const { return _outputTileExt; }
    
    /** Set the output poster name, e.g. output.bmp */
    void setOutputPosterName( const std::string& name ) { _outputPosterName = name; }
    const std::string& getOutputPosterName() const { return _outputPosterName; }
    
    /** Set the size of each sub-image-tile, e.g. 640x480 */
    void setTileSize( int w, int h ) { _tileSize.set(w, h); }
    const osg::Vec2& getTileSize() const { return _tileSize; }
    
    /** Set the final size of the high-res poster, e.g. 6400x4800 */
    void setPosterSize( int w, int h ) { _posterSize.set(w, h); }
    const osg::Vec2& getPosterSize() const { return _posterSize; }
    
    /** Set the capturing camera */
    void setCamera( osg::Camera* camera ) { _camera = camera; }
    const osg::Camera* getCamera() const { return _camera.get(); }
    
    /** Set the final poster image, should be already allocated */
    void setFinalPoster( osg::Image* image ) { _finalPoster = image; }
    const osg::Image* getFinalPoster() const { return _finalPoster.get(); }
    
    PosterVisitor* getPosterVisitor() { return _visitor.get(); }
    const PosterVisitor* getPosterVisitor() const { return _visitor.get(); }
    
    bool done() const { return !_isRunning && !_isFinishing; }
    
    void init( const osg::Camera* camera );
    void init( const osg::Matrixd& view, const osg::Matrixd& proj );
    void frame( const osg::FrameStamp* fs, osg::Node* node );
    
protected:
    virtual ~PosterPrinter() {}
    
    bool addCullCallbacks( const osg::FrameStamp* fs, osg::Node* node );
    void removeCullCallbacks( osg::Node* node );
    void bindCameraToImage( osg::Camera* camera, int row, int col );
    void recordImages();
    
    bool _outputTiles;
    std::string _outputTileExt;
    std::string _outputPosterName;
    osg::Vec2 _tileSize;
    osg::Vec2 _posterSize;
    
    bool _isRunning;
    bool _isFinishing;
    unsigned int _lastBindingFrame;
    int _tileRows;
    int _tileColumns;
    int _currentRow;
    int _currentColumn;
    osg::ref_ptr<PosterIntersector> _intersector;
    osg::ref_ptr<PosterVisitor> _visitor;
    
    osg::Matrixd _currentViewMatrix;
    osg::Matrixd _currentProjectionMatrix;
    osg::ref_ptr<osg::Camera> _camera;
    osg::ref_ptr<osg::Image> _finalPoster;
    TileImages _images;
};

#endif
