#ifndef _ORIENTATION_CONVERTER_H
#define _ORIENTATION_CONVERTER_H

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Node>
#include <osg/Geode>

class OrientationConverter {
    public :
    	OrientationConverter(void);
	void setConversion( const osg::Vec3 &from, const osg::Vec3 &to);
	void convert( osg::Node &node );

    private :
    	OrientationConverter( const OrientationConverter& ) {}
	OrientationConverter& operator = (const OrientationConverter& ) { return *this; }

        osg::Matrix _mat;


	class ConvertVisitor : public osg::NodeVisitor
	{
	    public :
	    	ConvertVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN){}

		virtual void apply( osg::Node &node ) { traverse( node ); }
		virtual void apply( osg::Geode &geode );
	};

	ConvertVisitor _cv;

};
#endif
