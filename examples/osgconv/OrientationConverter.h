#ifndef _ORIENTATION_CONVERTER_H
#define _ORIENTATION_CONVERTER_H

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Node>
#include <osg/Geode>

class OrientationConverter {
    public :
    	OrientationConverter(void);
	void setRotation( const osg::Vec3 &from, 
	                    const osg::Vec3 &to  );
	void setRotation( float degrees, const osg::Vec3 &axis  );
	void setTranslation( const osg::Vec3 &trans);
	void setScale( const osg::Vec3 &trans);
        
        /** return the root of the updated subgraph as the subgraph
          * the node passed in my flatten during optimization.*/
	osg::Node* convert( osg::Node* node );

    private :
    	OrientationConverter( const OrientationConverter& ) {}
	OrientationConverter& operator = (const OrientationConverter& ) { return *this; }

	osg::Matrix R, T, S;
	bool _trans_set;

};
#endif
