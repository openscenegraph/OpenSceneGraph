#ifndef __TILEMAPPER_H_
#define __TILEMAPPER_H_

#include "trpage_sys.h"
#include "trpage_read.h"

#include <osg/Referenced>
#include <osg/PagedLOD>

#include <OpenThreads/Mutex>

namespace txp
{
    class TileMapper : public osg::Referenced
    {
    public:

        static TileMapper* instance();

        osg::PagedLOD* getPagedLOD(int x, int y, int lod);

        void insertPagedLOD(int x, int y, int lod, osg::PagedLOD* pagedLod);

        void removePagedLOD(int x, int y, int lod);

        void prunePagedLOD();

    protected:

        // Constructor
        TileMapper() {}

        // Destructor
        virtual ~TileMapper() {}

        struct TileTriple
        {
            TileTriple(int ax, int ay, int alod):
                x(ax),y(ay),lod(alod) {}
                
            int x,y,lod;
            
            bool operator < (const TileTriple& rhs) const
            {
                if (x<rhs.x) return true;
                if (x>rhs.x) return false;
                if (y<rhs.y) return true;
                if (y>rhs.y) return false;
                if (lod<rhs.lod) return true;
                if (lod>rhs.lod) return false;
                return false;
            }
        };
        
        typedef std::map< TileTriple, osg::ref_ptr<osg::PagedLOD> > TileMap;
        
        OpenThreads::Mutex  _mutex;
        TileMap             _tileMap;

    };

} // namespace

#endif // __TXPARCHIVE_H_
