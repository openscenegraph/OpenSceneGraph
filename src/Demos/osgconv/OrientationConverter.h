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
	void setTranslation( const osg::Vec3 &trans);
	void setScale( const osg::Vec3 &trans);
	void convert( osg::Node &node );

    private :
    	OrientationConverter( const OrientationConverter& ) {}
	OrientationConverter& operator = (const OrientationConverter& ) { return *this; }

        class TransformFunctor : public osg::Drawable::AttributeFunctor
        {
            public:

                osg::Matrix _m;
                osg::Matrix _im;

                TransformFunctor():
                    osg::Drawable::AttributeFunctor(osg::Drawable::COORDS|osg::Drawable::NORMALS)
                {
                }
                
                void set(const osg::Matrix& m)
                {
                    _m = m;
                    _im.invert(_m);
                }

                virtual ~TransformFunctor() {}

                virtual bool apply(osg::Drawable::AttributeBitMask abm,osg::Vec3* begin,osg::Vec3* end)
                {
                    if (abm == osg::Drawable::COORDS)
                    {
                        for (osg::Vec3* itr=begin;itr<end;++itr)
                        {
                            (*itr) = (*itr)*_m;
                        }
                        return true;
                    }
                    else if (abm == osg::Drawable::NORMALS)
                    {
                        for (osg::Vec3* itr=begin;itr<end;++itr)
                        {
                            // note post mult by inverse for normals.
                            (*itr) = osg::Matrix::transform3x3(_im,(*itr));
                            (*itr).normalize();
                        }
                        return true;
                    }
                    return false;

                }

        };

	class ConvertVisitor : public osg::NodeVisitor
	{
	    public :
	    	ConvertVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}

		void setMatrix( const osg::Matrix& mat ) { _tf.set(mat); }

		virtual void apply( osg::Geode &geode );
                virtual void apply( osg::Billboard& billboard );
                virtual void apply( osg::LOD& lod );

	    private :
         	TransformFunctor _tf;
	};

	ConvertVisitor _cv;
	osg::Matrix R, T, S;
	bool _trans_set;

};
#endif
