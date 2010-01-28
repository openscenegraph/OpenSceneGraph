#include <osgParticle/VariableRateCounter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkRateRange( const osgParticle::VariableRateCounter& obj )
{ return true; }

static bool readRateRange( osgDB::InputStream& is, osgParticle::VariableRateCounter& obj )
{
    float min, max; is >> min >> max;
    obj.setRateRange( min, max ); return true;
}

static bool writeRateRange( osgDB::OutputStream& os, const osgParticle::VariableRateCounter& obj )
{
    const osgParticle::rangef& range = obj.getRateRange();
    os << range.minimum << range.maximum << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleVariableRateCounter,
                         /*new osgParticle::VariableRateCounter*/NULL,
                         osgParticle::VariableRateCounter,
                         "osg::Object osgParticle::Counter osgParticle::VariableRateCounter" )
{
    ADD_USER_SERIALIZER( RateRange );  // _rate_range
}
