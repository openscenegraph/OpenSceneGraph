#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include "osg/GL"
#include "osg/GeoSet"
#include "osg/Notify"

using namespace osg;

#define DEBUG
#define DO_SHADING 1

#define I_ON (1<<4)
#define C_ON (1<<3)
#define N_ON (1<<2)
#define T_ON (1<<1)
#define V_ON (1<<0)

void GeoSet::set_fast_path( void )
{
    if( _iaformat != IA_OFF )
    {
        _fast_path = I_ON;
    }
    else
    {
        if( ( _normal_binding != BIND_PERPRIM) &&
            ( _nindex.null() || _nindex ==_cindex) &&
            (  _color_binding != BIND_PERPRIM) &&
            ( _colindex.null() || _colindex ==_cindex ) &&
            ( _primtype != FLAT_LINE_STRIP ) &&
            ( _primtype != FLAT_TRIANGLE_STRIP ) &&
            ( _primtype != FLAT_TRIANGLE_FAN )
            )
            _fast_path = V_ON;
        else
        {
            _fast_path = 0;

            #ifdef DEBUG

            if( _normal_binding == BIND_PERPRIM )
                notify( DEBUG ) << "Geoset - Failed fast path because NORMALS are bound PER_PRIM\n";

            if( _nindex.valid() )
                notify( DEBUG ) << "Geoset - Failed fast path because NORMAL indeces are specified\n";

            if( _color_binding == BIND_PERPRIM )
                notify( DEBUG ) << "Geoset - Failed fast path because COLORS are bound PER_PRIM\n";

            if( _cindex.valid() )
                notify( DEBUG ) << "Geoset - Failed fast path because COLOR indeces are specified\n";

            if( _primtype == FLAT_LINE_STRIP )
                notify( DEBUG ) << "Geoset - Failed fast path because primitive is FLAT_LINE_STRIP\n";

            if ( _primtype == FLAT_TRIANGLE_STRIP )
                notify( DEBUG ) << "Geoset - Failed fast path because primitive is FLAT_TRIANGLE_STRIP\n";

            if ( _primtype == FLAT_TRIANGLE_FAN )
                notify( DEBUG ) << "Geoset - Failed fast path because primitive is FLAT_TRIANGLE_FAN\n";
            #endif
        }

        if( _fast_path )
        {
            if( _color_binding == BIND_PERVERTEX )
                _fast_path |= C_ON;
            if( _normal_binding == BIND_PERVERTEX )
                _fast_path |= N_ON;
            if( _texture_binding == BIND_PERVERTEX )
                _fast_path |= T_ON;
        }
    }

    #ifdef DEBUG
    notify(INFO) << "GeoSet - fast path = " << _fast_path << "\n";
    #endif
}


void GeoSet::draw_fast_path( void )
{
    IndexPointer ocindex = _cindex;

    switch( _fast_path )
    {
        case (I_ON) :
            _cindex = _iaindex;
            glInterleavedArrays( (GLenum)_ogliaformat, 0, _iarray );
            break;

        case (V_ON) :
            glDisableClientState( GL_COLOR_ARRAY );
            glDisableClientState( GL_NORMAL_ARRAY );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (T_ON|V_ON) :
            glDisableClientState( GL_COLOR_ARRAY );
            glDisableClientState( GL_NORMAL_ARRAY );
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, 0, (GLfloat *)_tcoords );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (N_ON|V_ON) :
            glDisableClientState( GL_COLOR_ARRAY );
            glEnableClientState( GL_NORMAL_ARRAY );
            glNormalPointer( GL_FLOAT, 0, (GLfloat *)_normals );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (N_ON|T_ON|V_ON) :
            glDisableClientState( GL_COLOR_ARRAY );
            glEnableClientState( GL_NORMAL_ARRAY );
            glNormalPointer( GL_FLOAT, 0, (GLfloat *)_normals );
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, 0, (GLfloat *)_tcoords );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (C_ON|V_ON) :
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer( 4, GL_FLOAT, 0, (GLfloat *)_colors );
            glDisableClientState( GL_NORMAL_ARRAY );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (C_ON|T_ON|V_ON) :
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer( 4, GL_FLOAT, 0, (GLfloat *)_colors );
            glDisableClientState( GL_NORMAL_ARRAY );
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, 0, (GLfloat *)_tcoords );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (C_ON|N_ON|V_ON) :
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer( 4, GL_FLOAT, 0, (GLfloat *)_colors );
            glEnableClientState( GL_NORMAL_ARRAY );
            glNormalPointer( GL_FLOAT, 0, (GLfloat *)_normals );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;

        case (C_ON|N_ON|T_ON|V_ON) :
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer( 4, GL_FLOAT, 0, (GLfloat *)_colors );
            glEnableClientState( GL_NORMAL_ARRAY );
            glNormalPointer( GL_FLOAT, 0, (GLfloat *)_normals );
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, 0, (GLfloat *)_tcoords );
            glEnableClientState( GL_VERTEX_ARRAY );
            glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );
            break;
    }

    if( _color_binding == BIND_OVERALL )
    {
        if( _colindex.valid() )
            glColor4fv( (GLfloat * )&_colors[_colindex[0]] );
        else
            glColor4fv( (GLfloat * )&_colors[0] );
    }

    if( _normal_binding == BIND_OVERALL )
    {
        if( _nindex.valid() )
            glNormal3fv( (GLfloat * )&_normals[_nindex[0]] );
        else
            glNormal3fv( (GLfloat * )&_normals[0] );
    }

    if( _needprimlen )           // LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP,
        // TRIANGLE_FAN, QUAD_STRIP, POLYGONS
    {
        int index = 0;
        if( _primLengths == (int *)0 )
        {
            notify(WARN) << "GeoSet->draw() : " "Primitive lengths required\n";
            return;
        }

        for( int i = 0; i < _numprims; i++ )
        {
            if( _cindex.valid() )
            {
                if (_cindex._is_ushort)
                    glDrawElements( (GLenum)_oglprimtype, _primLengths[i], GL_UNSIGNED_SHORT, &_cindex._ptr._ushort[index] );
                else
                    glDrawElements( (GLenum)_oglprimtype, _primLengths[i], GL_UNSIGNED_INT, &_cindex._ptr._uint[index] );
            }
            else
                glDrawArrays( (GLenum)_oglprimtype, index, _primLengths[i] );

            index += _primLengths[i];
        }
    }
    else                         // POINTS, LINES, TRIANGLES, QUADS
    {
        if( _cindex.valid())
        {
            if (_cindex._is_ushort)
                glDrawElements( (GLenum)_oglprimtype, _cindex._size, GL_UNSIGNED_SHORT, _cindex._ptr._ushort );
            else
                glDrawElements( (GLenum)_oglprimtype, _cindex._size, GL_UNSIGNED_INT, _cindex._ptr._uint );
        }
        else
            glDrawArrays( (GLenum)_oglprimtype, 0, _numcoords );
    }

    _cindex = ocindex;
}


void GeoSet::draw_alternate_path( void )
{
    if( (_color_binding == BIND_PERVERTEX) && (_colindex.null() || _colindex ==_cindex) && (_flat_shaded_skip == 0) )
    {
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, 0, (GLfloat *)_colors );
    }
    else
    {
        glDisableClientState( GL_COLOR_ARRAY );
        if( _color_binding == BIND_OVERALL )
        {
            if( _colindex.valid() )
                glColor4fv( (GLfloat *)&_colors[_colindex[0]] );
            else
                glColor4fv( (GLfloat *)&_colors[0] );
        }
    }

    if( (_normal_binding == BIND_PERVERTEX) && (_nindex.null() || _nindex ==_cindex) && (_flat_shaded_skip == 0) )
    {
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( GL_FLOAT, 0, (GLfloat *)_normals );
    }
    else
    {
        glDisableClientState( GL_NORMAL_ARRAY );
        if( _normal_binding == BIND_OVERALL )
        {
            if( _nindex.valid() )
                glNormal3fv( (GLfloat *)&_normals[_nindex[0]] );
            else
                glNormal3fv( (GLfloat *)&_normals[0] );
        }
    }

    if( (_texture_binding == BIND_PERVERTEX) && (_tindex.null()) )
    {
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( 2, GL_FLOAT, 0, _tcoords );
    }
    else
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, (GLfloat *)_coords );

    if( _needprimlen )           // LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP,
        // TRIANGLE_FAN, QUAD_STRIP, POLYGONS
        // FLAT_LINE_STRIP, FLAT_TRIANGLE_STRIP, FLAT_TRIANGLE_FAN
    {
        int i, j;
        int index = 0;
        int ai = 0;
        int ci = 0;
        int ni = 0;
        int ti = 0;

        if( _primLengths == (int *)0 )
        {
            notify(WARN) << "GeoSet->draw() : " "Primitive lengths required\n";
            return;
        }

        for( i = 0; i < _numprims; i++ )
        {
            if( _color_binding == BIND_PERPRIM )
            {
                if( _colindex.valid() )
                    glColor4fv( (GLfloat *)&_colors[_colindex[ci++]] );
                else
                    glColor4fv( (GLfloat *)&_colors[ci++] );
            }
            if( _normal_binding == BIND_PERPRIM )
            {
                if( _nindex.valid() )
                    glNormal3fv( (GLfloat *)&_normals[_nindex[ni++]] );
                else
                    glNormal3fv( (GLfloat *)&_normals[ni++] );
            }

            if( _flat_shaded_skip )
            {
                #ifdef DO_SHADING
                glShadeModel( GL_FLAT );
                #endif
                glBegin( (GLenum)_oglprimtype );
                for( j = 0; j < _primLengths[i]; j++ )
                {
                    if( j >= _flat_shaded_skip )
                    {
                        if( _color_binding == BIND_PERVERTEX )
                        {
                            if( (_colindex.valid()) )
                                glColor4fv( (GLfloat *)&_colors[_colindex[ci++]] );
                            else
                                glColor4fv( (GLfloat *)&_colors[ci++] );
                        }

                        if( _normal_binding == BIND_PERVERTEX )
                        {
                            if(_nindex.valid())
                                glNormal3fv( (GLfloat *)&_normals[_nindex[ni++]] );
                            else
                                glNormal3fv( (GLfloat *)&_normals[ni++] );
                        }
                    }

                    if( _texture_binding == BIND_PERVERTEX )
                    {
                        if( _tindex.valid() )
                            glTexCoord2fv( (GLfloat *)&_tcoords[_tindex[ti++]] );
                        else
                            glTexCoord2fv( (GLfloat *)&_tcoords[ti++] );
                    }

                    if( _cindex.valid() )
                        glArrayElement( _cindex[ai++] );
                    else
                        glArrayElement( ai++ );
                }
                glEnd();

                #ifdef DO_SHADING
                glShadeModel( GL_SMOOTH );
                #endif
            }

            else
            if( ((_color_binding == BIND_PERVERTEX ) && (_colindex.valid()) ) ||
                ((_normal_binding == BIND_PERVERTEX ) && (_nindex.valid()) ) ||
                ((_texture_binding == BIND_PERVERTEX ) && (_tindex.valid()) ) )
            {
                glBegin( (GLenum)_oglprimtype );
                for( j = 0; j < _primLengths[i]; j++ )
                {
                    if( (_color_binding == BIND_PERVERTEX) && (_colindex.valid()) )
                        glColor4fv( (GLfloat *)&_colors[_colindex[ci++]] );

                    if( (_normal_binding == BIND_PERVERTEX) && (_nindex.valid()) )
                        glNormal3fv( (GLfloat *)&_normals[_nindex[ni++]] );

                    if( (_texture_binding == BIND_PERVERTEX) && (_tindex.valid()) )
                        glTexCoord2fv( (GLfloat *)&_tcoords[_tindex[ti++]] );

                    if( _cindex.valid() )
                        glArrayElement( _cindex[ai++] );
                    else
                        glArrayElement( ai++ );
                }
                glEnd();
            }
            else
            {

                if( _cindex.valid() )
                {
                    if (_cindex._is_ushort)
                        glDrawElements( (GLenum)_oglprimtype, _primLengths[i], GL_UNSIGNED_SHORT, &_cindex._ptr._ushort[index] );
                    else
                        glDrawElements( (GLenum)_oglprimtype, _primLengths[i], GL_UNSIGNED_INT, &_cindex._ptr._uint[index] );
                }
                else
                    glDrawArrays( (GLenum)_oglprimtype, index, _primLengths[i] );
            }
            index += _primLengths[i];
        }
    }
    else                         // POINTS, LINES, TRIANGLES, QUADS
    {
        int i, j;
        if( _normal_binding == BIND_PERPRIM ||  _color_binding == BIND_PERPRIM ||
            ((_color_binding == BIND_PERVERTEX ) && (_colindex.valid()) ) ||
            ((_normal_binding == BIND_PERVERTEX ) && (_nindex.valid()) ) ||
            ((_texture_binding == BIND_PERVERTEX ) && (_tindex.valid()) ) )
        {

            glBegin( (GLenum)_oglprimtype );
            for( i = 0; i < _numprims; i++ )
            {
                if( _color_binding == BIND_PERPRIM )
                {
                    if( _colindex.valid() )
                        glColor4fv( (GLfloat *)&_colors[_colindex[i]] );
                    else
                        glColor4fv( (GLfloat *)&_colors[i] );
                }
                if( _normal_binding == BIND_PERPRIM  )
                {
                    if( _nindex.valid() )
                        glNormal3fv( (GLfloat *)&_normals[_nindex[i]] );
                    else
                        glNormal3fv( (GLfloat *)&_normals[i] );
                }

                for( j = 0; j < _primlength; j++ )
                {
                    if( (_color_binding == BIND_PERVERTEX) && (_colindex.valid() ) )
                        glColor4fv( (GLfloat *)&_colors[_colindex[i*_primlength+j]] );

                    if( (_normal_binding == BIND_PERVERTEX) && (_nindex.valid() ) )
                        glNormal3fv( (GLfloat *)&_normals[_nindex[i*_primlength+j]] );

                    if( (_texture_binding == BIND_PERVERTEX) && (_tindex.valid() ) )
                        glTexCoord2fv( (GLfloat *)&_tcoords[_tindex[i*_primlength+j]] );

                    if( _cindex.valid())
                        glArrayElement( _cindex[i*_primlength+j] );
                    else
                        glArrayElement( i*_primlength+j );
                }
            }
            glEnd();
        }
        else
        {
            if( _cindex.valid())
            {
                if (_cindex._is_ushort)
                    glDrawElements( (GLenum)_oglprimtype, _cindex._size, GL_UNSIGNED_SHORT, _cindex._ptr._ushort );
                else
                    glDrawElements( (GLenum)_oglprimtype, _cindex._size, GL_UNSIGNED_INT, _cindex._ptr._uint );
            }
            else
                glDrawArrays( (GLenum)_oglprimtype, 0, _numcoords );
        }
    }
}
