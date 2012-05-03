#include "osg/Sequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Sequence_readLocalData(Object& obj, Input& fr);
bool Sequence_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Sequence)
(
    new osg::Sequence,
    "Sequence",
    "Object Node Sequence Group",
    &Sequence_readLocalData,
    &Sequence_writeLocalData
);

static bool Sequence_matchLoopMode(const char* str,
                                   Sequence::LoopMode& mode)
{
    if (strcmp(str, "LOOP") == 0)
    {
        mode = Sequence::LOOP;
    }
    else if (strcmp(str, "SWING") == 0)
    {
        mode = Sequence::SWING;
    }
    else
    {
        return false;
    }

    return true;
}

static const char* Sequence_getLoopMode(Sequence::LoopMode mode)
{
    switch (mode) {
    case Sequence::SWING:
        return "SWING";
    case Sequence::LOOP:
    default:
        return "LOOP";
    }
}

// only storing 'START' and 'STOP' since 'PAUSE' doesn't make sense to me
static bool Sequence_matchSeqMode(const char* str,
                                  Sequence::SequenceMode& mode)
{
    if (strcmp(str, "START") == 0)
    {
        mode = Sequence::START;
    }
    else if (strcmp(str, "STOP") == 0)
    {
        mode = Sequence::STOP;
    }
    else
    {
        return false;
    }

    return true;
}

static const char* Sequence_getSeqMode(Sequence::SequenceMode mode)
{
    switch (mode) {
    case Sequence::START:
        return "START";
    default:
        return "STOP";
    }
}

bool Sequence_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Sequence& sw = static_cast<Sequence&>(obj);

    if (fr.matchSequence("defaultTime"))
    {
        if (fr[1].isFloat())
    {
        float t;
        fr[1].getFloat(t) ;
        sw.setDefaultTime(t) ;
            iteratorAdvanced = true;
            fr += 2;
        }
    }
    else if (fr.matchSequence("frameTime {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        int i = 0;
        while (!fr.eof() && fr[0].getNoNestedBrackets() > entry)
        {
            float t;
            if (fr[0].getFloat(t))
            {
                sw.setTime(i, t);
                ++fr;
                i++;
            }
        }

        iteratorAdvanced = true;
        ++fr;
    }
    else if (fr.matchSequence("lastFrameTime"))
    {
        if (fr[1].isFloat())
        {
            float t;
            fr[1].getFloat(t) ;
            sw.setLastFrameTime(t) ;
            iteratorAdvanced = true;
            fr += 2;
        }
    }
    else if (fr.matchSequence("interval"))
    {
        Sequence::LoopMode mode;
        int begin, end;
        if (Sequence_matchLoopMode(fr[1].getStr(), mode) &&
            fr[2].getInt(begin) && fr[3].getInt(end))
        {
            sw.setInterval(mode, begin, end);
            iteratorAdvanced = true;
            fr += 4;
        }
    }
    else if (fr.matchSequence("duration"))
    {
        if (fr[1].isFloat() && fr[2].isInt())
        {
            float speed;
            int nreps;
            fr[1].getFloat(speed);
            fr[2].getInt(nreps);
            sw.setDuration(speed, nreps);
            iteratorAdvanced = true;
            fr += 3;
        }
    }
    else if (fr.matchSequence("mode"))
    {
        Sequence::SequenceMode mode;
        if (Sequence_matchSeqMode(fr[1].getStr(), mode))
        {
            sw.setMode(mode);
            iteratorAdvanced = true;
            fr += 2;
        }
    }
    else if (fr.matchSequence("sync"))
    {
        if (fr[1].isInt())
        {
        int sync ;
        fr[1].getInt(sync) ;
        sw.setSync(sync!=0) ;
            iteratorAdvanced = true;
            fr += 2;
        }
    }
    else if (fr.matchSequence("clearOnStop"))
    {
        if (fr[1].isInt())
        {
            int clearOnStop ;
            fr[1].getInt(clearOnStop) ;
            sw.setClearOnStop(clearOnStop!=0) ;
            iteratorAdvanced = true;
            fr += 2;
        }
    }

    return iteratorAdvanced;
}

bool Sequence_writeLocalData(const Object& obj, Output& fw)
{
    const Sequence& sw = static_cast<const Sequence&>(obj);

    // default frame time
    fw.indent() << "defaultTime " << sw.getDefaultTime() << std::endl;

    // frame times
    fw.indent() << "frameTime {" << std::endl;
    fw.moveIn();
    for (unsigned int i = 0; i < sw.getNumChildren(); i++)
    {
        fw.indent() << sw.getTime(i) << std::endl;
    }

    fw.moveOut();
    fw.indent() << "}" << std::endl;

    // last frame time
    fw.indent() << "lastFrameTime " << sw.getLastFrameTime() << std::endl;

    // loop mode & interval
    Sequence::LoopMode mode;
    int begin, end;
    sw.getInterval(mode, begin, end);
    fw.indent() << "interval " << Sequence_getLoopMode(mode) << " " << begin << " " << end << std::endl;

    // duration
    float speed;
    int nreps;
    sw.getDuration(speed, nreps);
    fw.indent() << "duration " << speed << " " << nreps << std::endl;

    // sequence mode
    fw.indent() << "mode " << Sequence_getSeqMode(sw.getMode()) << std::endl;

    // sync
    fw.indent() << "sync " << (int) sw.getSync() << std::endl;

    // clearOnStop
    fw.indent() << "clearOnStop " << (int) sw.getClearOnStop() << std::endl;

    return true;
}
