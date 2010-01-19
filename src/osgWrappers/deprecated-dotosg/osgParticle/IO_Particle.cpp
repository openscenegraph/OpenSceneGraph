
#include <osgParticle/Particle>
#include <osgParticle/Interpolator>
#include <osgParticle/range>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Notify>

#include <osgDB/Input>
#include <osgDB/Output>

bool  read_particle(osgDB::Input &fr, osgParticle::Particle &P)
{
    if (fr[0].matchString("{")) {
        ++fr;
        int entry = fr[0].getNoNestedBrackets();
        bool itAdvanced = true;
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry && itAdvanced) {
            itAdvanced = false;
            if (fr[0].matchWord("shape")) {
                const char *ptstr = fr[1].getStr();
                if (ptstr) {
                    if (std::string(ptstr) == "QUAD") {
                        P.setShape(osgParticle::Particle::QUAD);
                    } else if (std::string(ptstr) == "HEXAGON") {
                        P.setShape(osgParticle::Particle::HEXAGON);
                    } else if (std::string(ptstr) == "POINT") {
                        P.setShape(osgParticle::Particle::POINT);
                    } else if (std::string(ptstr) == "QUAD_TRIANGLESTRIP") {
                        P.setShape(osgParticle::Particle::QUAD_TRIANGLESTRIP);
                    } else if (std::string(ptstr) == "LINE") {
                        P.setShape(osgParticle::Particle::LINE);
                    } else {
                        osg::notify(osg::WARN) << "Particle reader warning: invalid shape: " << ptstr << std::endl;
                    }
                    fr += 2;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("lifeTime")) {
                float lt;
                if (fr[1].getFloat(lt)) {
                    P.setLifeTime(lt);
                    fr += 2;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("sizeRange")) {
                osgParticle::rangef r;
                if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
                    P.setSizeRange(r);
                    fr += 3;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("alphaRange")) {
                osgParticle::rangef r;
                if (fr[1].getFloat(r.minimum) && fr[2].getFloat(r.maximum)) {
                    P.setAlphaRange(r);
                    fr += 3;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("colorRange")) {
                osgParticle::rangev4 r;
                if (fr[1].getFloat(r.minimum.x()) && fr[2].getFloat(r.minimum.y()) && fr[3].getFloat(r.minimum.z()) && fr[4].getFloat(r.minimum.w()) &&
                    fr[5].getFloat(r.maximum.x()) && fr[6].getFloat(r.maximum.y()) && fr[7].getFloat(r.maximum.z()) && fr[8].getFloat(r.maximum.w())) {
                    P.setColorRange(r);
                    fr += 9;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("position")) {
                osg::Vec3 v;
                if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
                    P.setPosition(v);
                    fr += 4;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("velocity")) {
                osg::Vec3 v;
                if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
                    P.setVelocity(v);
                    fr += 4;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("angle")) {
                osg::Vec3 v;
                if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
                    P.setAngle(v);
                    fr += 4;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("angularVelocity")) {
                osg::Vec3 v;
                if (fr[1].getFloat(v.x()) && fr[2].getFloat(v.y()) && fr[3].getFloat(v.z())) {
                    P.setAngularVelocity(v);
                    fr += 4;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("radius")) {
                float f;
                if (fr[1].getFloat(f)) {
                    P.setRadius(f);
                    fr += 2;
                    itAdvanced = true;
                }
            }

            if (fr[0].matchWord("mass")) {
                float f;
                if (fr[1].getFloat(f)) {
                    P.setMass(f);
                    fr += 2;
                    itAdvanced = true;
                }
            }

            if (fr[0].matchWord("textureTile")) {
                int sTile,tTile,numTiles;
                if (fr[1].getInt(sTile) && fr[2].getInt(tTile) && fr[3].getInt(numTiles)) {
                    P.setTextureTile(sTile,tTile,numTiles);
                    fr += 4;
                    itAdvanced = true;
                }
            }
 

            // interpolators

            if (fr[0].matchWord("sizeInterpolator") && fr[1].matchString("{")) {
                fr += 2;
                itAdvanced = true;
                osgParticle::Interpolator *ip = dynamic_cast<osgParticle::Interpolator *>(fr.readObject());
                if (ip) {
                    P.setSizeInterpolator(ip);
                }
                ++fr;
            }
            if (fr[0].matchWord("alphaInterpolator") && fr[1].matchString("{")) {
                fr += 2;
                itAdvanced = true;
                osgParticle::Interpolator *ip = dynamic_cast<osgParticle::Interpolator *>(fr.readObject());
                if (ip) {
                    P.setAlphaInterpolator(ip);
                }
                ++fr;
            }
            if (fr[0].matchWord("colorInterpolator") && fr[1].matchString("{")) {
                fr += 2;
                itAdvanced = true;
                osgParticle::Interpolator *ip = dynamic_cast<osgParticle::Interpolator *>(fr.readObject());
                if (ip) {
                    P.setColorInterpolator(ip);
                }
                ++fr;
            }
        }
        return true;
    }
    return false;
}

void  write_particle(const osgParticle::Particle &P, osgDB::Output &fw)
{
    fw << "{" << std::endl;
    fw.moveIn();

    fw.indent() << "shape ";
    switch (P.getShape())
    {
    case osgParticle::Particle::POINT: fw << "POINT" << std::endl; break;
    case osgParticle::Particle::HEXAGON: fw << "HEXAGON" << std::endl; break;
    case osgParticle::Particle::QUAD_TRIANGLESTRIP: fw << "QUAD_TRIANGLESTRIP" << std::endl; break;
    case osgParticle::Particle::QUAD: fw << "QUAD" << std::endl; break;
    case osgParticle::Particle::LINE:
    default: fw << "LINE" << std::endl; break;
    }

    fw.indent() << "lifeTime " << P.getLifeTime() << std::endl;

    osgParticle::rangef rf = P.getSizeRange();
    fw.indent() << "sizeRange " << rf.minimum << " " << rf.maximum << std::endl;

    rf = P.getAlphaRange();
    fw.indent() << "alphaRange " << rf.minimum << " " << rf.maximum << std::endl;

    osgParticle::rangev4 rv4 = P.getColorRange();
    fw.indent() << "colorRange ";
    fw << rv4.minimum.x() << " " << rv4.minimum.y() << " " << rv4.minimum.z() << " " << rv4.minimum.w() << " ";
    fw << rv4.maximum.x() << " " << rv4.maximum.y() << " " << rv4.maximum.z() << " " << rv4.maximum.w() << std::endl;

    osg::Vec3 v = P.getPosition();
    fw.indent() << "position ";
    fw << v.x() << " " << v.y() << " " << v.z() << std::endl;

    v = P.getVelocity();
    fw.indent() << "velocity ";
    fw << v.x() << " " << v.y() << " " << v.z() << std::endl;

    v = P.getAngle();
    fw.indent() << "angle ";
    fw << v.x() << " " << v.y() << " " << v.z() << std::endl;

    v = P.getAngularVelocity();
    fw.indent() << "angularVelocity ";
    fw << v.x() << " " << v.y() << " " << v.z() << std::endl;

    fw.indent() << "radius " << P.getRadius() << std::endl;
    fw.indent() << "mass " << P.getMass() << std::endl;
    fw.indent() << "textureTile " << P.getTileS() << " " << P.getTileT() << " " << P.getNumTiles() << std::endl;

    // interpolators

    fw.indent() << "sizeInterpolator {" << std::endl;
    fw.moveIn();
    fw.writeObject(*P.getSizeInterpolator());
    fw.moveOut();
    fw.indent() << "}" << std::endl;

    fw.indent() << "alphaInterpolator {" << std::endl;
    fw.moveIn();
    fw.writeObject(*P.getAlphaInterpolator());
    fw.moveOut();
    fw.indent() << "}" << std::endl;

    fw.indent() << "colorInterpolator {" << std::endl;
    fw.moveIn();
    fw.writeObject(*P.getColorInterpolator());
    fw.moveOut();
    fw.indent() << "}" << std::endl;

    fw.moveOut();
    fw.indent() << "}" << std::endl;
}
