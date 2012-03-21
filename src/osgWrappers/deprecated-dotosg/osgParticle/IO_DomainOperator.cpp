
#include <osgParticle/DomainOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>
#include <osg/io_utils>
#include <iostream>

bool  DomainOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  DomainOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(DomainOperator_Proxy)
(
    new osgParticle::DomainOperator,
    "DomainOperator",
    "Object Operator DomainOperator",
    DomainOperator_readLocalData,
    DomainOperator_writeLocalData
);

bool DomainOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::DomainOperator &dp = static_cast<osgParticle::DomainOperator &>(obj);
    bool itAdvanced = false;

    std::string typeName;
    while (fr.matchSequence("domain %s {"))
    {
        if (fr[1].getStr()) typeName = fr[1].getStr();
        fr += 3;

        osgParticle::DomainOperator::Domain::Type type = osgParticle::DomainOperator::Domain::UNDEFINED_DOMAIN;
        if (typeName == "point") type = osgParticle::DomainOperator::Domain::POINT_DOMAIN;
        else if (typeName == "line") type = osgParticle::DomainOperator::Domain::LINE_DOMAIN;
        else if (typeName == "triangle") type = osgParticle::DomainOperator::Domain::TRI_DOMAIN;
        else if (typeName == "rectangle") type = osgParticle::DomainOperator::Domain::RECT_DOMAIN;
        else if (typeName == "plane") type = osgParticle::DomainOperator::Domain::PLANE_DOMAIN;
        else if (typeName == "sphere") type = osgParticle::DomainOperator::Domain::SPHERE_DOMAIN;
        else if (typeName == "box") type = osgParticle::DomainOperator::Domain::BOX_DOMAIN;
        else if (typeName == "disk") type = osgParticle::DomainOperator::Domain::DISK_DOMAIN;

        osgParticle::DomainOperator::Domain domain(type);
        if (fr[0].matchWord("plane")) {
            if (fr[1].getFloat(domain.plane[0]) && fr[2].getFloat(domain.plane[1]) &&
                fr[3].getFloat(domain.plane[2]) && fr[4].getFloat(domain.plane[3]))
            {
                fr += 5;
            }
        }

        if (fr[0].matchWord("vertices1")) {
            if (fr[1].getFloat(domain.v1[0]) && fr[2].getFloat(domain.v1[1]) && fr[3].getFloat(domain.v1[2]))
            {
                fr += 4;
            }
        }

        if (fr[0].matchWord("vertices2")) {
            if (fr[1].getFloat(domain.v2[0]) && fr[2].getFloat(domain.v2[1]) && fr[3].getFloat(domain.v2[2]))
            {
                fr += 4;
            }
        }

        if (fr[0].matchWord("vertices3")) {
            if (fr[1].getFloat(domain.v3[0]) && fr[2].getFloat(domain.v3[1]) && fr[3].getFloat(domain.v3[2]))
            {
                fr += 4;
            }
        }

        if (fr[0].matchWord("basis1")) {
            if (fr[1].getFloat(domain.s1[0]) && fr[2].getFloat(domain.s1[1]) && fr[3].getFloat(domain.s1[2]))
            {
                fr += 4;
            }
        }

        if (fr[0].matchWord("basis2")) {
            if (fr[1].getFloat(domain.s2[0]) && fr[2].getFloat(domain.s2[1]) && fr[3].getFloat(domain.s2[2]))
            {
                fr += 4;
            }
        }

        if (fr[0].matchWord("factors")) {
            if (fr[1].getFloat(domain.r1) && fr[2].getFloat(domain.r2))
            {
                fr += 3;
            }
        }
        dp.addDomain(domain);

        ++fr;
        itAdvanced = true;
    }
    return itAdvanced;
}

bool DomainOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::DomainOperator &dp = static_cast<const osgParticle::DomainOperator &>(obj);

    for(unsigned int i=0;i<dp.getNumDomains();++i)
    {
        const osgParticle::DomainOperator::Domain& domain = dp.getDomain(i);

        switch (domain.type)
        {
        case osgParticle::DomainOperator::Domain::POINT_DOMAIN:
            fw.indent() << "domain point {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::LINE_DOMAIN:
            fw.indent() << "domain line {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::TRI_DOMAIN:
            fw.indent() << "domain triangle {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::RECT_DOMAIN:
            fw.indent() << "domain rectangle {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::PLANE_DOMAIN:
            fw.indent() << "domain plane {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::SPHERE_DOMAIN:
            fw.indent() << "domain sphere {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::BOX_DOMAIN:
            fw.indent() << "domain box {" << std::endl; break;
        case osgParticle::DomainOperator::Domain::DISK_DOMAIN:
            fw.indent() << "domain disk {" << std::endl; break;
        default:
            fw.indent() << "domain undefined {" << std::endl; break;
        }

        fw.moveIn();
        fw.indent() << "plane " << domain.plane << std::endl;
        fw.indent() << "vertices1 " << domain.v1 << std::endl;
        fw.indent() << "vertices2 " << domain.v2 << std::endl;
        fw.indent() << "vertices3 " << domain.v3 << std::endl;
        fw.indent() << "basis1 " << domain.s1 << std::endl;
        fw.indent() << "basis2 " << domain.s2 << std::endl;
        fw.indent() << "factors " << domain.r1 << " " << domain.r2 << std::endl;

        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }
    return true;
}
