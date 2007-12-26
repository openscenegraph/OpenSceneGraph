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
            default: break;
        }
    }
}
        
void DrawElementTypeSimplifierVisitor::apply(osg::Geode& node)
{
    DrawElementTypeSimplifier dets;
    
    unsigned int numDrawables = node.getNumDrawables();
    for (unsigned int i = 0; i != numDrawables; ++i)
    {
        osg::Geometry * geom = dynamic_cast<osg::Geometry*>(node.getDrawable(i));
        if (geom) dets.simplify(*geom);
    }    
        
    osg::NodeVisitor::apply((osg::Node&)node); 
}

}
