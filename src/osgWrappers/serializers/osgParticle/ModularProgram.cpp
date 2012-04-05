#include <osgParticle/ModularProgram>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkOperators( const osgParticle::ModularProgram& prog )
{
    return prog.numOperators()>0;
}

static bool readOperators( osgDB::InputStream& is, osgParticle::ModularProgram& prog )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgParticle::Operator* op = dynamic_cast<osgParticle::Operator*>( is.readObject() );
        if ( op ) prog.addOperator( op );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeOperators( osgDB::OutputStream& os, const osgParticle::ModularProgram& prog )
{
    unsigned int size = prog.numOperators();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << prog.getOperator(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleModularProgram,
                         new osgParticle::ModularProgram,
                         osgParticle::ModularProgram,
                         "osg::Object osg::Node osgParticle::ParticleProcessor osgParticle::Program osgParticle::ModularProgram" )
{
    ADD_USER_SERIALIZER( Operators );  // _operators
}
