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

#ifndef OSG_CONTEXTDATA
#define OSG_CONTEXTDATA 1

#include <osg/GraphicsContext>

namespace osg {

class OSG_EXPORT ContextData : public GraphicsObjectManager
{
    public:
        ContextData(unsigned int contextID);

        void incrementUsageCount() { ++_numContexts; }
        void decrementUsageCount() { --_numContexts; }

        void setNumContexts(unsigned int numContexts) { _numContexts = numContexts; }
        unsigned int getNumContexts() const { return _numContexts; }

        void setCompileContext(osg::GraphicsContext* gc) { _compileContext = gc; }
        osg::GraphicsContext* getCompileContext() { return _compileContext.get(); }

        /** Get a specific GL extensions object or GraphicsObjectManager, initialize if not already present.
          * Note, must only be called from a the graphics context thread associated with this osg::State. */
        template<typename T>
        T* get()
        {
            const std::type_info* id(&typeid(T));
            osg::ref_ptr<osg::Referenced>& ptr = _managerMap[id];
            if (!ptr)
            {
                ptr = new T(_contextID);
            }
            return static_cast<T*>(ptr.get());
        }

        /** Get a specific GL extensions object or GraphicsObjectManager if it already exists in the extension map.
          * Note, safe to call outwith a the graphics context thread associated with this osg::State.
          * Returns NULL if the desired extension object has not been created yet.*/
        template<typename T>
        const T* get() const
        {
            const std::type_info* id(&typeid(T));
            ManagerMap::const_iterator itr = _managerMap.find(id);
            if (itr==_managerMap.end()) return 0;
            else return itr->second.get();
        }

        /** Set a specific GL extensions object pr GraphicsObjectManager. */
        template<typename T>
        void set(T* ptr)
        {
            const std::type_info* id(&typeid(T));
            _managerMap[id] = ptr;
        }

        /** Signal that a new frame has started.*/
        virtual void newFrame(osg::FrameStamp*);

        virtual void resetStats();
        virtual void reportStats(std::ostream& out);
        virtual void recomputeStats(std::ostream& out) const;

        /** Flush all deleted OpenGL objects within the specified availableTime.
        * Note, must be called from a thread which has current the graphics context associated with contextID. */
        virtual void flushDeletedGLObjects(double currentTime, double& availableTime);

        /** Flush all deleted OpenGL objects.
        * Note, must be called from a thread which has current the graphics context associated with contextID. */
        virtual void flushAllDeletedGLObjects();

        /** Do a GL delete all OpenGL objects.
        * Note, must be called from a thread which has current the graphics context associated with contextID. */
        virtual void deleteAllGLObjects();

        /** Discard all OpenGL objects.
        * Note, unlike deleteAllGLjects discard does not
        * do any OpenGL calls so can be called from any thread, but as a consequence it
        * also doesn't remove the associated OpenGL resource so discard should only be
        * called when the associated graphics context is being/has been closed. */
        virtual void discardAllGLObjects();

    public:

        /** Create a contextID for a new graphics context, this contextID is used to set up the osg::State associate with context.
          * Automatically increments the usage count of the contextID to 1.*/
        static unsigned int createNewContextID();

        /** Get the current max ContextID.*/
        static unsigned int getMaxContextID();

        /** Increment the usage count associate with a contextID. The usage count specifies how many graphics contexts a specific contextID is shared between.*/
        static void incrementContextIDUsageCount(unsigned int contextID);

        /** Decrement the usage count associate with a contextID. Once the contextID goes to 0 the contextID is then free to be reused.*/
        static void decrementContextIDUsageCount(unsigned int contextID);

        typedef GraphicsContext::GraphicsContexts GraphicsContexts;

        /** Get all the registered graphics contexts.*/
        static GraphicsContexts getAllRegisteredGraphicsContexts();

        /** Get all the registered graphics contexts associated with a specific contextID.*/
        static GraphicsContexts getRegisteredGraphicsContexts(unsigned int contextID);

        /** Get the GraphicsContext for doing background compilation for GraphicsContexts associated with specified contextID.*/
        static void setCompileContext(unsigned int contextID, GraphicsContext* gc);

        /** Get existing or create a new GraphicsContext to do background compilation for GraphicsContexts associated with specified contextID.*/
        static  GraphicsContext* getOrCreateCompileContext(unsigned int contextID);

        /** Get the GraphicsContext for doing background compilation for GraphicsContexts associated with specified contextID.*/
        static GraphicsContext* getCompileContext(unsigned int contextID);

        /** Register a GraphicsContext.*/
        static void registerGraphicsContext(GraphicsContext* gc);

        /** Unregister a GraphicsContext.*/
        static void unregisterGraphicsContext(GraphicsContext* gc);

    protected:
        virtual ~ContextData();

        unsigned int _numContexts;
        osg::ref_ptr<osg::GraphicsContext> _compileContext;

        // ManagerMap contains GL Extentsions objects used by StateAttribue to call OpenGL extensions/advanced features
        typedef std::map<const std::type_info*, osg::ref_ptr<osg::Referenced> > ManagerMap;
        ManagerMap _managerMap;
};


/** Get the ContextData for a specific contextID.*/
extern OSG_EXPORT ContextData* getContextData(unsigned int contextID);

/** Get or create the ContextData for a specific contextID.*/
extern OSG_EXPORT ContextData* getOrCreateContextData(unsigned int contextID);

template<typename T>
inline T* get(unsigned int contextID)
{
    ContextData* gc = getOrCreateContextData(contextID);
    return gc->get<T>();
}

// specialize for ContextData to avoid ContextData being nested within itself.
template<> inline ContextData* get<ContextData>(unsigned int contextID) { return getOrCreateContextData(contextID); }

}

#endif
