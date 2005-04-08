#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include "IO_LightPoint.h"

using namespace osgSim;

bool readLightPoint(LightPoint & lp, osgDB::Input &fr)
{
    if (fr.matchSequence("lightPoint {"))
    {
        fr += 2;
        int entry = fr[0].getNoNestedBrackets();
        bool itAdvanced = true;
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry && itAdvanced) {
            itAdvanced = false;
            if (fr[0].matchWord("isOn")) {
                const char * ptstr = fr[1].getStr();
                if (ptstr) {
                    if (std::string(ptstr) == "TRUE") {
                        lp._on = true;
                    } else if (std::string(ptstr) == "FALSE") {
                        lp._on = false;
                    } else {
                        osg::notify(osg::WARN) << "osg::Sim reader warning: invalid isOn: " << ptstr << std::endl;
                    }
                    fr += 2;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("position")) {
                float x, y, z;
                if (fr[1].getFloat(x) && fr[2].getFloat(y) && fr[3].getFloat(z)) {
                    lp._position.set(x, y, z);
                    fr += 4;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("color")) {
                float r, g, b, a;
                if (fr[1].getFloat(r) && fr[2].getFloat(g) && fr[3].getFloat(b) && fr[4].getFloat(a)) {
                    lp._color.set(r, g, b, a);
                    fr += 5;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("intensity")) {
                if (fr[1].getFloat(lp._intensity)) {
                    fr += 2;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("radius")) {
                if (fr[1].getFloat(lp._radius)) {
                    fr += 2;
                    itAdvanced = true;
                }
            }
            if (fr[0].matchWord("blendingMode")) {
                const char * ptstr = fr[1].getStr();
                if (ptstr) {
                    if (std::string(ptstr) == "ADDITIVE") {
                        lp._blendingMode = LightPoint::ADDITIVE;
                        fr += 2;
                        itAdvanced = true;
                    } else if (std::string(ptstr) == "BLENDED") {
                        lp._blendingMode = LightPoint::BLENDED;
                        fr += 2;
                        itAdvanced = true;
                    } else {
                        osg::notify(osg::WARN) << "osg::Sim reader warning: invalid blendingMode: " << ptstr << std::endl;
                    }
                }
            }
            Sector * sector = static_cast<Sector *>(fr.readObjectOfType(osgDB::type_wrapper<Sector>()));
            if (sector) {
                lp._sector = sector;
                itAdvanced = true;
            }
            BlinkSequence * seq = static_cast<BlinkSequence *>(fr.readObjectOfType(osgDB::type_wrapper<BlinkSequence>()));
            if (seq) {
                lp._blinkSequence = seq;
                itAdvanced = true;
            }
        }
        return true;
    }
    return false;
}

bool writeLightPoint(const LightPoint & lp, osgDB::Output &fw)
{
    fw.indent() << "lightPoint {" << std::endl;
    fw.moveIn();
    fw.indent() << "isOn " << ( lp._on ? "TRUE" : "FALSE") << std::endl;
    fw.indent() << "position " << lp._position << std::endl;
    fw.indent() << "color " << lp._color << std::endl;
    fw.indent() << "intensity " << lp._intensity << std::endl;
    fw.indent() << "radius " << lp._radius << std::endl;
    fw.indent() << "blendingMode ";
    switch (lp._blendingMode) {
        case LightPoint::ADDITIVE:
            fw << "ADDITIVE" << std::endl;
            break;
        case LightPoint::BLENDED:
        default :
            fw << "BLENDED" << std::endl;
            break;
    }
    if (lp._sector.valid()) {
        fw.writeObject(*lp._sector);
    }
    if (lp._blinkSequence.valid()) {
        fw.writeObject(*lp._blinkSequence);
    }

    fw.moveOut();
    fw.indent() << "}" << std::endl;
    return true;
}
