/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#ifndef DATASET_H
#define DATASET_H 1

#include <osg/Node>
#include <osg/Matrixd>
#include <osg/BoundingBox>
#include <osg/Image>
#include <osg/Shape>

#include <osgTerrain/CoordinateSystem>

#include <gdal_priv.h>

class DataSet : public osg::Referenced
{
    public:

        class Source;
        

        struct SpatialProperties
        {
            SpatialProperties():
                _numValuesX(0),
                _numValuesY(0),
                _numValuesZ(0) {}
        
            SpatialProperties(const SpatialProperties& sp):
                _cs(sp._cs),
                _geoTransform(sp._geoTransform),
                _extents(sp._extents),
                _numValuesX(sp._numValuesX),
                _numValuesY(sp._numValuesY),
                _numValuesZ(sp._numValuesZ) {}

        
            SpatialProperties& operator = (const SpatialProperties& sp)
            {
                if (&sp==this) return *this;
                
                _cs = sp._cs;
                _geoTransform = sp._geoTransform;
                _extents = sp._extents;
                _numValuesX = sp._numValuesX;
                _numValuesY = sp._numValuesY;
                _numValuesZ = sp._numValuesZ;
                
                return *this;
            }

            osg::ref_ptr<osgTerrain::CoordinateSystem>  _cs;
            osg::Matrixd                                _geoTransform;
            osg::BoundingBox                            _extents;
            unsigned int                                _numValuesX;
            unsigned int                                _numValuesY;
            unsigned int                                _numValuesZ;
        };

        struct DestinationData : public osg::Referenced, SpatialProperties
        {
        
            DestinationData():
                _minDistance(0.0),
                _maxDistance(FLT_MAX) {}
        
            
            typedef std::vector< osg::ref_ptr<osg::Node> > ModelList;
            
            float                                       _minDistance;
            float                                       _maxDistance;

            osg::ref_ptr<osg::Image>                    _image;
            osg::ref_ptr<osg::HeightField>              _heightField;
            ModelList                                   _models;
        };

        struct SourceData : public osg::Referenced, public SpatialProperties
        {
        
            SourceData(Source* source=0):
                _source(source),
                _hasGCPs(false),
                _gdalDataSet(0) {}
                
                
            virtual ~SourceData()
            {
                if (_gdalDataSet) GDALClose(_gdalDataSet);
            }

            static SourceData* readData(Source* source);
            
            osg::BoundingBox getExtents(const osgTerrain::CoordinateSystem* cs) const;
            
            SpatialProperties computeSpatialProperties(osgTerrain::CoordinateSystem* cs) const;

            void read(DestinationData& destination);
            
            void readImage(DestinationData& destination);
            void readHeightField(DestinationData& destination);
            void readModels(DestinationData& destination);

            Source*                                     _source;
            
            bool                                        _hasGCPs;
            
            osg::ref_ptr<osg::Node>                     _model;
            GDALDataset*                                _gdalDataSet;
        };


        class Source : public osg::Referenced, public SpatialProperties
        {
        public:
        
            enum Type
            {
                IMAGE,
                HEIGHT_FIELD,
                MODEL
            };
            
            enum ParameterPolicy
            {
                PREFER_CONFIG_SETTINGS,
                PREFER_FILE_SETTINGS
            };

            Source():
                _type(IMAGE),
                _sortValue(0.0),
                _temporaryFile(false),
                _coordinateSystemPolicy(PREFER_FILE_SETTINGS),
                _geoTransformPolicy(PREFER_FILE_SETTINGS)
                {}
        
            Source(Type type, const std::string& filename):
                _type(type),
                _sortValue(0.0),
                _filename(filename),
                _temporaryFile(false),
                _coordinateSystemPolicy(PREFER_FILE_SETTINGS),
                _geoTransformPolicy(PREFER_FILE_SETTINGS)
                {}

            void setSortValue(double s) { _sortValue = s; }
            double getSortValue() const { return _sortValue; }

            void setSortValueFromSourceDataResolution();

            void setType(Type type) { _type = type; }
            Type getType() const { return _type; }

            void setFileName(const std::string& filename) { _filename = filename; }
            const std::string& getFileName() const { return _filename; }

            void setTemporaryFile(bool temporaryFile) { _temporaryFile = temporaryFile; }
            bool getTemporaryFile() const { return _temporaryFile; }

            void setCoordinateSystemPolicy(ParameterPolicy policy) { _coordinateSystemPolicy = policy; }
            void setCoordinateSystem(osgTerrain::CoordinateSystem* cs) { _cs = cs; }
            osgTerrain::CoordinateSystem* getCoordinateSystem() { return  _cs.get(); }

            
            void setGeoTransformPolicy(ParameterPolicy policy)  { _geoTransformPolicy = policy; }

            void setGeoTransform(osg::Matrixd& transform) { _geoTransform = transform; }
            osg::Matrixd& getGeoTransform() { return _geoTransform; }
            
            
            void setSourceData(SourceData* data) { _sourceData = data; if (_sourceData.valid()) _sourceData->_source = this; }
            SourceData* getSourceData() { return _sourceData.get(); }
            
            void loadSourceData();
            
            bool needReproject(const osgTerrain::CoordinateSystem* cs) const;
            
            Source* doReproject(osgTerrain::CoordinateSystem* cs, const std::string& filename) const;
            
            void buildOverviews();

        protected:
        

            Type                                        _type;

            double                                      _sortValue;
        
            std::string                                 _filename;
            bool                                        _temporaryFile;
            
            ParameterPolicy                             _coordinateSystemPolicy;
            ParameterPolicy                             _geoTransformPolicy;
            
            osg::ref_ptr<SourceData>                    _sourceData;
                
        };
        
        enum CompositeType
        {
            GROUP,
            LEVEL_OF_DETAIL
        };
        
        class CompositeSource : public osg::Referenced, public SpatialProperties
        {
        public:
            
            CompositeSource() {};
            
            typedef std::vector< osg::ref_ptr<Source> > SourceList;
            typedef std::vector< osg::ref_ptr< CompositeSource> > ChildList;
            
            void setSortValue(double s) { _sortValue = s; }
            double getSortValue() const { return _sortValue; }

            void setSortValueFromSourceDataResolution();

            void sort();            

            class iterator
            {
            public:
            

                iterator(CompositeSource* composite)
                {
                    if (composite) 
                    {
                        _positionStack.push_back(IteratorPosition(composite));
                        advance();
                    }
                }
                
                iterator(const iterator& rhs):
                    _positionStack(rhs._positionStack) {}

                iterator& operator = (const iterator& rhs)
                {
                    if (&rhs==this) return *this;
                    _positionStack = rhs._positionStack;
                }
                
                bool valid() const
                {
                    return !_positionStack.empty() && _positionStack.back().valid();
                }
                                    
                osg::ref_ptr<Source>& operator *()
                {
                    return valid()?_positionStack.back().currentSource():_nullSource;
                }
                
                osg::ref_ptr<Source>* operator ->()
                {
                    return valid()?&(_positionStack.back().currentSource()):&_nullSource;
                }
                
                iterator& operator++()
                {
                    advance(); 
                    return *this;
                }
                
                iterator operator++(int)
                {
                    iterator tmp=*this; 
                    advance(); 
                    return tmp; 
                }
                
                bool advance()
                {
                    if (_positionStack.empty()) return false;
                    
                    // simple advance to the next source
                    if (_positionStack.back().advance())
                    {
                        if (_positionStack.back().currentSource().valid()) return true;

                        if (_positionStack.back().currentChild())
                        {
                            std::cout<<"Pushing IteratorPosition"<<std::endl;
                            _positionStack.push_back(IteratorPosition(_positionStack.back().currentChild()));
                            return advance();
                        }
                    }
 
                    std::cout<<"Popping IteratorPosition"<<std::endl;
                    _positionStack.pop_back();
                    return advance();
                }

                
            protected:
            
                struct IteratorPosition
                {
                
                    IteratorPosition(CompositeSource* composite):
                        _composite(composite),
                        _index(-1) {}

                    IteratorPosition(const IteratorPosition& rhs):
                        _composite(rhs._composite),
                        _index(rhs._index) {}

                    IteratorPosition& operator = (const IteratorPosition& rhs)
                    {
                        _composite = rhs._composite;
                        _index = rhs._index;
                        return *this;
                    }

                
                    osg::ref_ptr<Source>& currentSource()
                    {
                        return  (_index>=0 && _index < (int)_composite->_sourceList.size())?_composite->_sourceList[_index]:_nullSource;
                    }

                    CompositeSource* currentChild()
                    {
                        return  (_index < (int)_composite->_sourceList.size())?0:
                                (_index-_composite->_sourceList.size() < _composite->_children.size())?(_composite->_children[_index-_composite->_sourceList.size()].get()):0;
                    }

                    bool valid() const
                    {
                        return _composite && 
                               _index >= 0 &&
                               _index < (int)(_composite->_sourceList.size()+_composite->_children.size());
                    }
                    
                    bool advance()
                    {
                        if (_index+1 < (int)(_composite->_sourceList.size()+_composite->_children.size()))
                        {
                            ++_index;
                            return valid();
                        }
                        return false;
                    }

                    CompositeSource*                _composite;
                    int                             _index;
                    osg::ref_ptr<Source>            _nullSource;
                };

                typedef std::vector<IteratorPosition> PositionStack;
                PositionStack _positionStack;
                osg::ref_ptr<Source>                _nullSource;
            };
            
            
            
            CompositeType   _type;
            SourceList      _sourceList;
            ChildList       _children;
            float           _sortValue;
        };
        
        
        
        class DestinationTile : public osg::Referenced, public SpatialProperties
        {
        public:
        
        
            enum Position
            {
                LEFT        = 0,
                LEFT_BELOW  = 1,
                BELOW       = 2,
                BELOW_RIGHT = 3,
                RIGHT       = 4,
                RIGHT_ABOVE = 5,
                ABOVE       = 6,
                ABOVE_LEFT  = 7,
                NUMBER_OF_POSITIONS = 8
            };
        
        
            DestinationTile();

            void setNeighbours(DestinationTile* left, DestinationTile* left_below, 
                               DestinationTile* below, DestinationTile* below_right,
                               DestinationTile* right, DestinationTile* right_above,
                               DestinationTile* above, DestinationTile* above_left);
                               
            void checkNeighbouringTiles();
            
            void setMaximumImagerySize(unsigned int maxNumColumns,unsigned int maxNumRows)
            {
                _imagery_maxNumColumns = maxNumColumns;
                _imagery_maxNumRows = maxNumRows;
            }

            void setMaximumTerrainSize(unsigned int maxNumColumns,unsigned int maxNumRows)
            {
                _terrain_maxNumColumns = maxNumColumns;
                _terrain_maxNumRows = maxNumRows;
            }
            
            void computeMaximumSourceResolution(CompositeSource* sourceGraph);

            void allocate();
            
            void readFrom(CompositeSource* sourceGraph);
            
            void equalizeCorner(Position position);
            void equalizeEdge(Position position);
            

            void equalizeBoundaries();

            osg::Node* createScene();
            
            
            std::string                                 _name;
            
            osg::ref_ptr<DestinationData>               _imagery;
            osg::ref_ptr<DestinationData>               _terrain;
            osg::ref_ptr<DestinationData>               _models;
            
            DestinationTile*                            _neighbour[8];
            bool                                        _equalized[8];
            
            
            unsigned int                                _imagery_maxNumColumns;
            unsigned int                                _imagery_maxNumRows;
            float                                       _imagery_maxSourceResolutionX;
            float                                       _imagery_maxSourceResolutionY;
            
            unsigned int                                _terrain_maxNumColumns;
            unsigned int                                _terrain_maxNumRows;
            float                                       _terrain_maxSourceResolutionX;
            float                                       _terrain_maxSourceResolutionY;

        };

        class CompositeDestination : public osg::Referenced, public SpatialProperties
        {
        public:
            
            void readFrom(CompositeSource* sourceGraph);

            void equalizeBoundaries();

            osg::Node* createScene();

            typedef std::vector< osg::ref_ptr<DestinationTile> > TileList;
            typedef std::vector< osg::ref_ptr<CompositeDestination> > ChildList;

            CompositeType   _type;
            TileList        _tiles;
            ChildList       _children;
        };

    public:


        DataSet();

        void addSource(Source* source);
        void addSource(CompositeSource* composite);

        void loadSources();
        
        void setDestinationCoordinateSystem(osgTerrain::CoordinateSystem* cs) { _coordinateSystem = cs; }
        void setDestinationExtents(const osg::BoundingBox& extents) { _extents = extents; }
        void setDestinationGeoTransform(const osg::Matrixd& geoTransform) { _geoTransform = geoTransform; }
        
        void setDestinationTileBaseName(const std::string& basename) { _tileBasename = basename; }
        void setDestinationTileExtension(const std::string& extension) { _tileExtension = _tileExtension; }
        
        
        void computeDestinationGraphFromSources();
        void updateSourcesForDestinationGraphNeeds();
        void populateDestinationGraphFromSources();
        
        void createDestination();
        
        void writeDestination(const std::string& filename);
        
        osg::Node* getDestinationRootNode() { return _rootNode.get(); }


    protected:

        virtual ~DataSet() {}
        
        void init();

        osg::ref_ptr<CompositeSource>               _sourceGraph;
        
        osg::ref_ptr<CompositeDestination>          _destinationGraph;

        osg::ref_ptr<osgTerrain::CoordinateSystem>  _coordinateSystem;
        osg::Matrixd                                _geoTransform;
        osg::BoundingBox                            _extents;
        std::string                                 _tileBasename;
        std::string                                 _tileExtension;
        osg::Vec4                                   _defaultColour;
        
        
        osg::ref_ptr<osg::Node>                     _rootNode;
        
};

#endif
