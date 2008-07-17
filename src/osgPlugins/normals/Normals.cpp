#include "Normals.h"

using namespace osg;

Normals::Normals( Node *node, float scale, Mode mode )
{
    setName(mode == VertexNormals ? "VertexNormals" : "SurfaceNormals");

    MakeNormalsVisitor mnv(scale,mode);
    node->accept( mnv );

    ref_ptr<Vec3Array> coords = mnv.getCoords();
    ref_ptr<Vec4Array> colors = new Vec4Array;
    if( mode == SurfaceNormals )
        colors->push_back( Vec4( 0, 1, 0, 1 ));
    else if( mode == VertexNormals )
        colors->push_back( Vec4( 1, 0, 0, 1 ));

    ref_ptr<Geometry> geom = new Geometry;
    geom->setVertexArray( coords.get() );
    geom->setColorArray( colors.get() );
    geom->setColorBinding( Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new DrawArrays( PrimitiveSet::LINES, 0, coords->size()));

    StateSet *sset = new StateSet;
    sset->setMode( GL_LIGHTING,   StateAttribute::OFF);
    geom->setStateSet( sset );
    addDrawable( geom.get() );
}



Normals::MakeNormalsVisitor::MakeNormalsVisitor( float normalScale, Mode mode): 
            NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _normal_scale(normalScale),
            _mode(mode)
{
    _local_coords = new Vec3Array;
    _mat = osg::Matrix::identity();
}


void Normals::MakeNormalsVisitor::apply(osg::MatrixTransform& tx)
{
    _matStack.push( _mat );
    _mat = _mat * tx.getMatrix();

    traverse( tx );

    _mat = _matStack.top();
    _matStack.pop();
}

void Normals::MakeNormalsVisitor::apply( Geode &geode )
{
    for( unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        Geometry *geom = dynamic_cast<Geometry *>(geode.getDrawable(i));
        if( geom )
        {
            Vec3Array *coords   = dynamic_cast<Vec3Array*>(geom->getVertexArray());
            if( coords == 0L )
                continue;

            Vec3Array *normals  = dynamic_cast<Vec3Array*>(geom->getNormalArray());
            if( normals == 0L )
                continue;

            Geometry::AttributeBinding binding = geom->getNormalBinding();
            if( binding == Geometry::BIND_OFF )
                continue;

            if( binding == Geometry::BIND_OVERALL )
            {
                Vec3 v(0,0,0);
                Vec3 n = normals->front();

                Vec3Array::iterator coord_index = coords->begin();
                while( coord_index != coords->end() )
                  v += *(coord_index++) * _mat;
                v /= (float)(coords->size()); 

                n *= _normal_scale;
                _local_coords->push_back( v );
                _local_coords->push_back( (v + n));
            }
            else // BIND_PER_PRIMITIVE_SET, BIND_PER_PRIMITIVE, BIND_PER_VERTEX
            {
                Geometry::PrimitiveSetList& primitiveSets = geom->getPrimitiveSetList();
                Geometry::PrimitiveSetList::iterator itr;

                Vec3Array::iterator coord_index   = coords->begin();
                Vec3Array::iterator normals_index = normals->begin();

                for(itr=primitiveSets.begin(); itr!=primitiveSets.end(); ++itr)
                {
#ifdef DEBUG
                    _printPrimitiveType( (*itr).get() );
#endif
                    if( binding == Geometry::BIND_PER_PRIMITIVE_SET )
                    {
                        Vec3 v(0,0,0);
                        Vec3 n = *(normals_index++); 
                        int ni = (*itr)->getNumIndices();
                        for( int i = 0; i < ni; i++ )
                            v += *(coord_index++) * _mat;
                        v /= (float)(ni);

                        n *= _normal_scale;
                        _local_coords->push_back( v );
                        _local_coords->push_back( (v + n));
                    }
                    else 
                    {
                        switch((*itr)->getMode())
                        {
                            case(PrimitiveSet::TRIANGLES):
                            {
                                for( unsigned int j = 0; j < (*itr)->getNumPrimitives(); j++ )
                                {
                                    _processPrimitive( 3, coord_index, normals_index, binding );
                                    coord_index += 3;
                                    if( binding == Geometry::BIND_PER_PRIMITIVE )
                                        normals_index++;
                                    else 
                                        normals_index+=3;
                                }
                                break;
                            }
                            case(PrimitiveSet::TRIANGLE_STRIP):
                            {
                                for( unsigned int j = 0; j < (*itr)->getNumIndices()-2; j++ )
                                {
                                    _processPrimitive( 3, coord_index, normals_index, binding );
                                    coord_index++;
                                    normals_index++;
                                }
                                coord_index += 2;
                                if( binding == Geometry::BIND_PER_VERTEX )
                                    normals_index += 2;
                                break;
                            }
                            case(PrimitiveSet::TRIANGLE_FAN):
                                break;

                            case(PrimitiveSet::QUADS):
                            {
                                for( unsigned int j = 0; j < (*itr)->getNumPrimitives(); j++ )
                                {
                                    _processPrimitive( 4, coord_index, normals_index, binding );
                                    coord_index += 4;
                                    if( binding == Geometry::BIND_PER_PRIMITIVE )
                                        normals_index++;
                                    else 
                                        normals_index+=4;
                                }
                                break;
                            }
                            case(PrimitiveSet::QUAD_STRIP):
                                break;

                            case(PrimitiveSet::POLYGON):
                            {
                                DrawArrayLengths* dal = dynamic_cast<DrawArrayLengths*>((*itr).get());
                                if (dal) {
                                    for (unsigned int j = 0; j < dal->size(); ++j) {
                                        unsigned int num_prim = (*dal)[j];
                                        //notify(WARN) << "j=" << j << " num_prim=" << num_prim << std::endl;
                                        _processPrimitive(num_prim, coord_index, normals_index, binding);
                                        coord_index += num_prim;
                                        if (binding == Geometry::BIND_PER_PRIMITIVE) {
                                            ++normals_index;
                                        } else {
                                            normals_index += num_prim;
                                        }
                                    }
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                }
            }
        }
    }
    traverse( geode );
}


void Normals::MakeNormalsVisitor::_processPrimitive(  unsigned int nv,
                        Vec3Array::iterator coords, 
                        Vec3Array::iterator normals,
                        Geometry::AttributeBinding binding )
{
    Vec3 v(0,0,0);
    Vec3 n(0,0,0);
    if( _mode == SurfaceNormals || binding == Geometry::BIND_PER_PRIMITIVE )
    {
        if( binding == Geometry::BIND_PER_PRIMITIVE )
        {
            n = *(normals++);
        }
        else if( binding == Geometry::BIND_PER_VERTEX )
        {
            for( unsigned int i = 0; i < nv; i++ )
                n += *(normals++);
            n /= (float)(nv); 
        }

        for( unsigned int i = 0; i < nv; i++ )
            v += *(coords++) * _mat;
        v /= (float)(nv);

        n *= _normal_scale;
        _local_coords->push_back( v );
        _local_coords->push_back( (v + n));
    }
    else if( _mode == VertexNormals )
    {
        for( unsigned int i = 0; i < nv; i++ )
        {
            v = *(coords++) * _mat;
            n = *(normals++);
            n *= _normal_scale;
            _local_coords->push_back( v );
            _local_coords->push_back( (v + n));
        }
    }
}

#ifdef DEBUG
void Normals::_printPrimitiveType( osg::PrimitiveSet *pset )
{
    std::cout << (
            pset->getMode() == PrimitiveSet::POINTS ? "POINTS" :
            pset->getMode() == PrimitiveSet::LINES ? "LINES" :
            pset->getMode() == PrimitiveSet::LINE_STRIP ? "LINE_STRIP" :
            pset->getMode() == PrimitiveSet::LINE_LOOP ? "LINE_LOOP" :
            pset->getMode() == PrimitiveSet::TRIANGLES ? "TRIANGLES" :
            pset->getMode() == PrimitiveSet::TRIANGLE_STRIP ? "TRIANGLE_STRIP" :
            pset->getMode() == PrimitiveSet::TRIANGLE_FAN ? "TRIANGLE_FAN" :
            pset->getMode() == PrimitiveSet::QUADS ? "QUADS" :
            pset->getMode() == PrimitiveSet::QUAD_STRIP ? "QUAD_STRIP" :
            pset->getMode() == PrimitiveSet::POLYGON ? "POLYGON" : "Dunno" )  << std::endl;
}
#endif

