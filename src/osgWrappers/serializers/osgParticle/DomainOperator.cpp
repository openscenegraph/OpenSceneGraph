#include <osgParticle/DomainOperator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkDomains( const osgParticle::DomainOperator& dp )
{
    return dp.getNumDomains()>0;
}

static bool readDomains( osgDB::InputStream& is, osgParticle::DomainOperator& dp )
{
    osgParticle::DomainOperator::Domain::Type type = osgParticle::DomainOperator::Domain::UNDEFINED_DOMAIN;
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string typeName;
        is >> is.PROPERTY("Domain") >> typeName >> is.BEGIN_BRACKET;
        if (typeName=="POINT") type = osgParticle::DomainOperator::Domain::POINT_DOMAIN;
        else if (typeName=="LINE") type = osgParticle::DomainOperator::Domain::LINE_DOMAIN;
        else if (typeName=="TRIANGLE") type = osgParticle::DomainOperator::Domain::TRI_DOMAIN;
        else if (typeName=="RECTANGLE") type = osgParticle::DomainOperator::Domain::RECT_DOMAIN;
        else if (typeName=="PLANE") type = osgParticle::DomainOperator::Domain::PLANE_DOMAIN;
        else if (typeName=="SPHERE") type = osgParticle::DomainOperator::Domain::SPHERE_DOMAIN;
        else if (typeName=="BOX") type = osgParticle::DomainOperator::Domain::BOX_DOMAIN;
        else if (typeName=="DISK") type = osgParticle::DomainOperator::Domain::DISK_DOMAIN;

        osgParticle::DomainOperator::Domain domain(type);
        is >> is.PROPERTY("Plane") >> domain.plane;
        is >> is.PROPERTY("Vertices1") >> domain.v1;
        is >> is.PROPERTY("Vertices2") >> domain.v2;
        is >> is.PROPERTY("Vertices3") >> domain.v3;
        is >> is.PROPERTY("Basis1") >> domain.s1;
        is >> is.PROPERTY("Basis2") >> domain.s2;
        is >> is.PROPERTY("Factors") >> domain.r1 >> domain.r2;
        dp.addDomain(domain);

        is >> is.END_BRACKET;
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeDomains( osgDB::OutputStream& os, const osgParticle::DomainOperator& dp )
{
    unsigned int size = dp.getNumDomains();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgParticle::DomainOperator::Domain& domain = dp.getDomain(i);

        os << os.PROPERTY("Domain");
        switch (domain.type)
        {
        case osgParticle::DomainOperator::Domain::POINT_DOMAIN:
            os << std::string("POINT") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::LINE_DOMAIN:
            os << std::string("LINE") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::TRI_DOMAIN:
            os << std::string("TRIANGLE") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::RECT_DOMAIN:
            os << std::string("RECTANGLE") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::PLANE_DOMAIN:
            os << std::string("PLANE") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::SPHERE_DOMAIN:
            os << std::string("SPHERE") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::BOX_DOMAIN:
            os << std::string("BOX") << os.BEGIN_BRACKET << std::endl; break;
        case osgParticle::DomainOperator::Domain::DISK_DOMAIN:
            os << std::string("DISK") << os.BEGIN_BRACKET << std::endl; break;
        default:
            os << std::string("UNDEFINED") << os.BEGIN_BRACKET << std::endl; break;
        }

        os << os.PROPERTY("Plane") << domain.plane << std::endl;
        os << os.PROPERTY("Vertices1") << domain.v1 << std::endl;
        os << os.PROPERTY("Vertices2") << domain.v2 << std::endl;
        os << os.PROPERTY("Vertices3") << domain.v3 << std::endl;
        os << os.PROPERTY("Basis1") << domain.s1 << std::endl;
        os << os.PROPERTY("Basis2") << domain.s2 << std::endl;
        os << os.PROPERTY("Factors") << domain.r1 << domain.r2 << std::endl;
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgParticleDomainOperator,
                         new osgParticle::DomainOperator,
                         osgParticle::DomainOperator,
                         "osg::Object osgParticle::Operator osgParticle::DomainOperator" )
{
    ADD_USER_SERIALIZER( Domains );  // _placers
}
