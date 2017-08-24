#include <osg/PrimitiveSetIndirect>
#include <osgUtil/DrawElementTypeSimplifier>

#include <osg/Geode>

template <typename InType, typename OutType>
OutType * copy(InType& array)
{
    unsigned int size = array.size();
    OutType * newArray = new OutType(array.getMode(), size);
    OutType & na = *newArray;

    for (unsigned int i = 0; i < size; ++i) na[i] = array[i];

    return newArray;
}

template <typename InType>
unsigned int getMax(InType& array)
{
    unsigned int max = 0;
    unsigned int size = array.size();

    for (unsigned int i = 0; i < size; ++i)
    {
        if (array[i] > max) max = array[i];
    }
    return (max);
}

namespace osgUtil
{


void DrawElementTypeSimplifier::simplify(osg::Geometry & geometry) const
{
    osg::Geometry::PrimitiveSetList & psl = geometry.getPrimitiveSetList();
    osg::Geometry::PrimitiveSetList::iterator it, end = psl.end();

    unsigned int max = 0;

    for (it = psl.begin(); it!=end; ++it)
    {
        switch ((*it)->getType())
        {
            case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
            {
                osg::DrawElementsUShort & de = *static_cast<osg::DrawElementsUShort*>(it->get());

                max = getMax<osg::DrawElementsUShort>(de);
                if (max < 255) *it = copy<osg::DrawElementsUShort, osg::DrawElementsUByte>(de);

                break;
            }
            case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
            {
                osg::DrawElementsUInt & de = *static_cast<osg::DrawElementsUInt*>(it->get());

                max = getMax<osg::DrawElementsUInt>(de);
                if (max < 256) *it = copy<osg::DrawElementsUInt, osg::DrawElementsUByte>(de);
                else if (max < 65536) *it = copy<osg::DrawElementsUInt, osg::DrawElementsUShort>(de);

                break;
            }
            case osg::PrimitiveSet::DrawElementsUShortIndirectPrimitiveType:
            {
                osg::DrawElementsIndirectUShort & de = *static_cast<osg::DrawElementsIndirectUShort*>(it->get());

                max = getMax<osg::DrawElementsIndirectUShort>(de);
                if (max < 255) *it = copy<osg::DrawElementsIndirectUShort, osg::DrawElementsIndirectUByte>(de);

                break;
            }
            case osg::PrimitiveSet::DrawElementsUIntIndirectPrimitiveType:
            {
                osg::DrawElementsIndirectUInt & de = *static_cast<osg::DrawElementsIndirectUInt*>(it->get());

                max = getMax<osg::DrawElementsIndirectUInt>(de);
                if (max < 256) *it = copy<osg::DrawElementsIndirectUInt, osg::DrawElementsIndirectUByte>(de);
                else if (max < 65536) *it = copy<osg::DrawElementsIndirectUInt, osg::DrawElementsIndirectUShort>(de);

                break;
            }
            case osg::PrimitiveSet::MultiDrawElementsUShortIndirectPrimitiveType:
            {
                osg::MultiDrawElementsIndirectUShort & de = *static_cast<osg::MultiDrawElementsIndirectUShort*>(it->get());

                max = getMax<osg::MultiDrawElementsIndirectUShort>(de);
                if (max < 255) *it = copy<osg::MultiDrawElementsIndirectUShort, osg::MultiDrawElementsIndirectUByte>(de);

                break;
            }
            case osg::PrimitiveSet::MultiDrawElementsUIntIndirectPrimitiveType:
            {
                osg::MultiDrawElementsIndirectUInt & de = *static_cast<osg::MultiDrawElementsIndirectUInt*>(it->get());

                max = getMax<osg::MultiDrawElementsIndirectUInt>(de);
                if (max < 256) *it = copy<osg::MultiDrawElementsIndirectUInt, osg::MultiDrawElementsIndirectUByte>(de);
                else if (max < 65536) *it = copy<osg::MultiDrawElementsIndirectUInt, osg::MultiDrawElementsIndirectUShort>(de);

                break;
            }
            default: break;
        }
    }
}

void DrawElementTypeSimplifierVisitor::apply(osg::Geometry& geom)
{
    DrawElementTypeSimplifier dets;
    dets.simplify(geom);
}

}
