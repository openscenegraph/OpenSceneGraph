#include <osg/ShapeDrawable>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;


// forward declare functions to use later.
bool TessellationHints_readLocalData(Object& obj, Input& fr);
bool TessellationHints_writeLocalData(const Object& obj, Output& fw);

RegisterDotOsgWrapperProxy g_TessellationHintsFuncProxy
(
    new osg::TessellationHints,
    "TessellationHints",
    "Object TessellationHints",
    &TessellationHints_readLocalData,
    &TessellationHints_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool TessellationHints_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TessellationHints& hints = static_cast<TessellationHints&>(obj);

    if (fr.matchSequence("detailRatio %f")) {
        float ratio = 1.0f;
        fr[1].getFloat(ratio);
        hints.setDetailRatio(ratio);
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("createFaces")) {
        hints.setCreateFrontFace(fr[1].matchWord("TRUE"));
        hints.setCreateBackFace(fr[2].matchWord("TRUE"));
        fr += 3;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("createNormals")) {
        hints.setCreateNormals(fr[1].matchWord("TRUE"));
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("createTextureCoords")) {
        hints.setCreateTextureCoords(fr[1].matchWord("TRUE"));
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("createParts")) {
        hints.setCreateTop(fr[1].matchWord("TRUE"));
        hints.setCreateBody(fr[2].matchWord("TRUE"));
        hints.setCreateBottom(fr[3].matchWord("TRUE"));
        fr += 4;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool TessellationHints_writeLocalData(const Object& obj, Output& fw)
{
    const TessellationHints& hints = static_cast<const TessellationHints&>(obj);

    fw.indent() << "detailRatio " << hints.getDetailRatio() << std::endl;

    fw.indent() << "createFaces " << (hints.getCreateFrontFace() ? "TRUE" : "FALSE") << " " << (hints.getCreateBackFace() ? "TRUE" : "FALSE") << std::endl;

    fw.indent() << "createNormals " << (hints.getCreateNormals() ? "TRUE" : "FALSE") << std::endl;
    fw.indent() << "createTextureCoords " << (hints.getCreateTextureCoords() ? "TRUE" : "FALSE") << std::endl;

    fw.indent() << "createParts " << (hints.getCreateTop() ? "TRUE" : "FALSE" ) << " " << (hints.getCreateBody() ? "TRUE" : "FALSE") << " " << (hints.getCreateBottom() ? "TRUE" : "FALSE") << std::endl;

    return true;
}
