#include <osgParticle/Particle>
#include <osg/Drawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

BEGIN_USER_TABLE( Shape, osgParticle::Particle );
    ADD_USER_VALUE( POINT );
    ADD_USER_VALUE( QUAD );
    ADD_USER_VALUE( QUAD_TRIANGLESTRIP );
    ADD_USER_VALUE( HEXAGON );
    ADD_USER_VALUE( LINE );
    ADD_USER_VALUE( USER );
END_USER_TABLE()

USER_READ_FUNC( Shape, readShapeValue )
USER_WRITE_FUNC( Shape, writeShapeValue )

bool readParticle( osgDB::InputStream& is, osgParticle::Particle& p )
{
    is >> is.BEGIN_BRACKET;

    is >> is.PROPERTY("Shape");
    p.setShape( static_cast<osgParticle::Particle::Shape>(readShapeValue(is)) );

    double lifeTime; is >> is.PROPERTY("LifeTime") >> lifeTime;
    p.setLifeTime( lifeTime );

    float min, max; osg::Vec4d minV, maxV;
    is >> is.PROPERTY("SizeRange") >> min >> max; p.setSizeRange( osgParticle::rangef(min, max) );
    is >> is.PROPERTY("AlphaRange") >> min >> max; p.setAlphaRange( osgParticle::rangef(min, max) );
    is >> is.PROPERTY("ColorRange") >> minV >> maxV; p.setColorRange( osgParticle::rangev4(minV, maxV) );

    bool hasInterpolator = false;
    is >> is.PROPERTY("SizeInterpolator") >> hasInterpolator;
    if ( hasInterpolator )
    {
        is >> is.BEGIN_BRACKET;
        p.setSizeInterpolator( is.readObjectOfType<osgParticle::Interpolator>() );
        is >> is.END_BRACKET;
    }
    is >> is.PROPERTY("AlphaInterpolator") >> hasInterpolator;
    if ( hasInterpolator )
    {
        is >> is.BEGIN_BRACKET;
        p.setAlphaInterpolator( is.readObjectOfType<osgParticle::Interpolator>() );
        is >> is.END_BRACKET;
    }
    is >> is.PROPERTY("ColorInterpolator") >> hasInterpolator;
    if ( hasInterpolator )
    {
        is >> is.BEGIN_BRACKET;
        p.setColorInterpolator( is.readObjectOfType<osgParticle::Interpolator>() );
        is >> is.END_BRACKET;
    }

    float radius; is >> is.PROPERTY("Radius") >> radius;
    float mass; is >> is.PROPERTY("Mass") >> mass;
    osg::Vec3d pos; is >> is.PROPERTY("Position") >> pos;
    osg::Vec3d vel; is >> is.PROPERTY("Velocity") >> vel;
    osg::Vec3d angle; is >> is.PROPERTY("Angle") >> angle;
    osg::Vec3d angleV; is >> is.PROPERTY("AngularVelocity") >> angleV;
    int s, t, num; is >> is.PROPERTY("TextureTile") >> s >> t >> num;

    p.setRadius( radius );
    p.setMass( mass );
    p.setPosition( pos );
    p.setVelocity( vel );
    p.setAngle( angle );
    p.setAngularVelocity( angleV );
    p.setTextureTile( s, t, num );

    if (is.getFileVersion()<145)
    {
        bool hasObject = false; is >> is.PROPERTY("Drawable") >> hasObject;
        if ( hasObject )
        {
            is >> is.BEGIN_BRACKET;
            osg::ref_ptr<osg::Drawable> drawable = is.readObjectOfType<osg::Drawable>();
            OSG_NOTICE<<"Warning: read osgParticle::Particle with USER defined Drawable which is no longer supported."<<std::endl;
            is >> is.END_BRACKET;
        }

    }
    is >> is.END_BRACKET;
    return true;
}

bool writeParticle( osgDB::OutputStream& os, const osgParticle::Particle& p )
{
    os << os.BEGIN_BRACKET << std::endl;

    os << os.PROPERTY("Shape"); writeShapeValue( os, (int)p.getShape() ); os << std::endl;

    os << os.PROPERTY("LifeTime") << p.getLifeTime() << std::endl;
    os << os.PROPERTY("SizeRange") << p.getSizeRange().minimum << p.getSizeRange().maximum << std::endl;
    os << os.PROPERTY("AlphaRange") << p.getAlphaRange().minimum << p.getAlphaRange().maximum << std::endl;
    os << os.PROPERTY("ColorRange") << osg::Vec4d(p.getColorRange().minimum)
                                        << osg::Vec4d(p.getColorRange().maximum) << std::endl;

    os << os.PROPERTY("SizeInterpolator") << (p.getSizeInterpolator()!=NULL);
    if ( p.getSizeInterpolator()!=NULL )
        os << os.BEGIN_BRACKET << std::endl << p.getSizeInterpolator() << os.END_BRACKET << std::endl;
    os << os.PROPERTY("AlphaInterpolator") << (p.getAlphaInterpolator()!=NULL);
    if ( p.getAlphaInterpolator()!=NULL )
        os << os.BEGIN_BRACKET << std::endl << p.getAlphaInterpolator() << os.END_BRACKET << std::endl;
    os << os.PROPERTY("ColorInterpolator") << (p.getColorInterpolator()!=NULL);
    if ( p.getColorInterpolator()!=NULL )
        os << os.BEGIN_BRACKET << std::endl << p.getColorInterpolator() << os.END_BRACKET << std::endl;

    os << os.PROPERTY("Radius") << p.getRadius() << std::endl;
    os << os.PROPERTY("Mass") << p.getMass() << std::endl;
    os << os.PROPERTY("Position") << osg::Vec3d(p.getPosition()) << std::endl;
    os << os.PROPERTY("Velocity") << osg::Vec3d(p.getVelocity()) << std::endl;
    os << os.PROPERTY("Angle") << osg::Vec3d(p.getAngle()) << std::endl;
    os << os.PROPERTY("AngularVelocity") << osg::Vec3d(p.getAngularVelocity()) << std::endl;
    os << os.PROPERTY("TextureTile") << p.getTileS() << p.getTileT() << p.getNumTiles() << std::endl;

    os << os.END_BRACKET << std::endl;
    return true;
}
