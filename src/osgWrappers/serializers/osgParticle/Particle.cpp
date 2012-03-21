#include <osgParticle/Particle>
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
    is >> osgDB::BEGIN_BRACKET;

    is >> osgDB::PROPERTY("Shape");
    p.setShape( static_cast<osgParticle::Particle::Shape>(readShapeValue(is)) );

    double lifeTime; is >> osgDB::PROPERTY("LifeTime") >> lifeTime;
    p.setLifeTime( lifeTime );

    float min, max; osg::Vec4d minV, maxV;
    is >> osgDB::PROPERTY("SizeRange") >> min >> max; p.setSizeRange( osgParticle::rangef(min, max) );
    is >> osgDB::PROPERTY("AlphaRange") >> min >> max; p.setAlphaRange( osgParticle::rangef(min, max) );
    is >> osgDB::PROPERTY("ColorRange") >> minV >> maxV; p.setColorRange( osgParticle::rangev4(minV, maxV) );

    bool hasInterpolator = false;
    is >> osgDB::PROPERTY("SizeInterpolator") >> hasInterpolator;
    if ( hasInterpolator )
    {
        is >> osgDB::BEGIN_BRACKET;
        p.setSizeInterpolator( static_cast<osgParticle::Interpolator*>(is.readObject()) );
        is >> osgDB::END_BRACKET;
    }
    is >> osgDB::PROPERTY("AlphaInterpolator") >> hasInterpolator;
    if ( hasInterpolator )
    {
        is >> osgDB::BEGIN_BRACKET;
        p.setAlphaInterpolator( static_cast<osgParticle::Interpolator*>(is.readObject()) );
        is >> osgDB::END_BRACKET;
    }
    is >> osgDB::PROPERTY("ColorInterpolator") >> hasInterpolator;
    if ( hasInterpolator )
    {
        is >> osgDB::BEGIN_BRACKET;
        p.setColorInterpolator( static_cast<osgParticle::Interpolator*>(is.readObject()) );
        is >> osgDB::END_BRACKET;
    }

    float radius; is >> osgDB::PROPERTY("Radius") >> radius;
    float mass; is >> osgDB::PROPERTY("Mass") >> mass;
    osg::Vec3d pos; is >> osgDB::PROPERTY("Position") >> pos;
    osg::Vec3d vel; is >> osgDB::PROPERTY("Velocity") >> vel;
    osg::Vec3d angle; is >> osgDB::PROPERTY("Angle") >> angle;
    osg::Vec3d angleV; is >> osgDB::PROPERTY("AngularVelocity") >> angleV;
    int s, t, num; is >> osgDB::PROPERTY("TextureTile") >> s >> t >> num;

    p.setRadius( radius );
    p.setMass( mass );
    p.setPosition( pos );
    p.setVelocity( vel );
    p.setAngle( angle );
    p.setAngularVelocity( angleV );
    p.setTextureTile( s, t, num );

    bool hasObject = false; is >> osgDB::PROPERTY("Drawable") >> hasObject;
    if ( hasObject )
    {
        is >> osgDB::BEGIN_BRACKET;
        p.setDrawable( dynamic_cast<osg::Drawable*>(is.readObject()) );
        is >> osgDB::END_BRACKET;
    }

    is >> osgDB::END_BRACKET;
    return true;
}

bool writeParticle( osgDB::OutputStream& os, const osgParticle::Particle& p )
{
    os << osgDB::BEGIN_BRACKET << std::endl;

    os << osgDB::PROPERTY("Shape"); writeShapeValue( os, (int)p.getShape() ); os << std::endl;

    os << osgDB::PROPERTY("LifeTime") << p.getLifeTime() << std::endl;
    os << osgDB::PROPERTY("SizeRange") << p.getSizeRange().minimum << p.getSizeRange().maximum << std::endl;
    os << osgDB::PROPERTY("AlphaRange") << p.getAlphaRange().minimum << p.getAlphaRange().maximum << std::endl;
    os << osgDB::PROPERTY("ColorRange") << osg::Vec4d(p.getColorRange().minimum)
                                        << osg::Vec4d(p.getColorRange().maximum) << std::endl;

    os << osgDB::PROPERTY("SizeInterpolator") << (p.getSizeInterpolator()!=NULL);
    if ( p.getSizeInterpolator()!=NULL )
        os << osgDB::BEGIN_BRACKET << std::endl << p.getSizeInterpolator() << osgDB::END_BRACKET << std::endl;
    os << osgDB::PROPERTY("AlphaInterpolator") << (p.getAlphaInterpolator()!=NULL);
    if ( p.getAlphaInterpolator()!=NULL )
        os << osgDB::BEGIN_BRACKET << std::endl << p.getAlphaInterpolator() << osgDB::END_BRACKET << std::endl;
    os << osgDB::PROPERTY("ColorInterpolator") << (p.getColorInterpolator()!=NULL);
    if ( p.getColorInterpolator()!=NULL )
        os << osgDB::BEGIN_BRACKET << std::endl << p.getColorInterpolator() << osgDB::END_BRACKET << std::endl;

    os << osgDB::PROPERTY("Radius") << p.getRadius() << std::endl;
    os << osgDB::PROPERTY("Mass") << p.getMass() << std::endl;
    os << osgDB::PROPERTY("Position") << osg::Vec3d(p.getPosition()) << std::endl;
    os << osgDB::PROPERTY("Velocity") << osg::Vec3d(p.getVelocity()) << std::endl;
    os << osgDB::PROPERTY("Angle") << osg::Vec3d(p.getAngle()) << std::endl;
    os << osgDB::PROPERTY("AngularVelocity") << osg::Vec3d(p.getAngularVelocity()) << std::endl;
    os << osgDB::PROPERTY("TextureTile") << p.getTileS() << p.getTileT() << p.getNumTiles() << std::endl;

    os << osgDB::PROPERTY("Drawable") << (p.getDrawable()!=NULL);
    if ( p.getDrawable()!=NULL )
    {
        os << osgDB::BEGIN_BRACKET << std::endl;
        os.writeObject( p.getDrawable() );
        os << osgDB::END_BRACKET;
    }
    os << std::endl;

    os << osgDB::END_BRACKET << std::endl;
    return true;
}
