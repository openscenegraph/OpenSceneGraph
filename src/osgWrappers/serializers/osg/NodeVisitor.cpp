#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/NodeVisitor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( NodeVistor,
                         new osg::NodeVisitor,
                         osg::NodeVisitor,
                         "osg::Object osg::NodeVisitor" )
{
    BEGIN_ENUM_SERIALIZER( TraversalMode, TRAVERSE_NONE );
        ADD_ENUM_VALUE( TRAVERSE_NONE );
        ADD_ENUM_VALUE( TRAVERSE_PARENTS );
        ADD_ENUM_VALUE( TRAVERSE_ALL_CHILDREN );
        ADD_ENUM_VALUE( TRAVERSE_ACTIVE_CHILDREN );
    END_ENUM_SERIALIZER();

    BEGIN_ENUM_SERIALIZER( VisitorType, NODE_VISITOR );
        ADD_ENUM_VALUE( UPDATE_VISITOR );
        ADD_ENUM_VALUE( EVENT_VISITOR );
        ADD_ENUM_VALUE( COLLECT_OCCLUDER_VISITOR );
        ADD_ENUM_VALUE( CULL_VISITOR );
        ADD_ENUM_VALUE( INTERSECTION_VISITOR );
    END_ENUM_SERIALIZER();

    ADD_UINT_SERIALIZER(TraversalMask, 0xffffffff);
    ADD_UINT_SERIALIZER(TraversalNumber, 0);
}


#undef OBJECT_CAST
#define OBJECT_CAST static_cast
