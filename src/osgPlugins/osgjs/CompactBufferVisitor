/* -*-c++-*- OpenSceneGraph - Copyright (C) Sketchfab
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#ifndef COMPACT_BUFFER_VISITOR
#define COMPACT_BUFFER_VISITOR

#include <algorithm>


////// taken from gles/GeometryUniqueVisitor

#include <osg/NodeVisitor>
#include <osg/Geometry>
#include <osg/Geode>

#include <osgAnimation/RigGeometry>

#include <map>


class CompactBufferVisitor : public osg::NodeVisitor {
    public:
        CompactBufferVisitor():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {}

        void apply(osg::Geometry& geometry) {
            if(isProcessed(&geometry))
                return;

            compactPrimitiveSets(geometry);
            setProcessed(&geometry);

            osgAnimation::RigGeometry *rig = dynamic_cast<osgAnimation::RigGeometry*>(&geometry);
            if(rig && rig->getSourceGeometry()) {
                apply(*rig->getSourceGeometry());
            }
        }


    protected:
        bool isProcessed(const osg::Object* buffer)
        { return (_processed.find(buffer) != _processed.end()); }

        void setProcessed(const osg::Object* source, osg::Object* processed=0)
        { _processed.insert(std::pair<const osg::Object*, osg::Object*>(source, processed)); }

        void compactPrimitiveSets(osg::Geometry& geometry) {
            osg::Geometry::PrimitiveSetList& primitives = geometry.getPrimitiveSetList();

            for (unsigned int i = 0 ; i < primitives.size() ; i++) {
                osg::DrawElementsUInt* de = dynamic_cast<osg::DrawElementsUInt*>(primitives[i].get());
                if(isProcessed(de)) {
                    geometry.setPrimitiveSet(i, dynamic_cast<osg::DrawElements*>(getProcessedBuffer(de)));
                }
                else {
                    if(de && de->getNumIndices()) {
                        unsigned int maximum = maxIndex(de);

                        if(maximum < 256) {
                            osg::DrawElementsUByte* elements = new osg::DrawElementsUByte(de->getMode());
                            for (unsigned int j = 0 ; j < de->getNumIndices() ; ++ j) {
                                elements->push_back(static_cast<GLubyte>(de->index(j)));
                            }
                            geometry.setPrimitiveSet(i, elements);
                            setProcessed(de, elements);
                        }
                        else if (maximum < 65536) {
                            osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(de->getMode());
                            for (unsigned int j = 0 ; j < de->getNumIndices() ; ++ j) {
                                elements->push_back(static_cast<GLushort>(de->index(j)));
                            }
                            geometry.setPrimitiveSet(i, elements);
                            setProcessed(de, elements);
                        }
                    }
                }
            }
        }

        unsigned int maxIndex(osg::DrawElements* de) {
            unsigned int maximum = de->index(0);
            for(unsigned int i = 1 ; i < de->getNumIndices() ; ++ i){
                maximum = std::max(maximum, static_cast<unsigned int>(de->index(i)));
            }
            return maximum;
        }

        osg::Object* getProcessedBuffer(const osg::Object* buffer) {
            std::map<const osg::Object*, osg::Object*>::iterator it = _processed.find(buffer);
            if(it == _processed.end()) {
                return 0;
            }
            else {
                return it->second;
            }
        }

        std::map<const osg::Object*, osg::Object*> _processed;
};

#endif
