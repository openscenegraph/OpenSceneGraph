// The majority of the application is dedicated to building the
// current contribitors list by parsing the ChangeLog, it just takes
// one line in the main itself to report the version number.

#include <set>
#include <vector>
#include <iostream>

#include <OpenThreads/Version>

#include <osg/Notify>
#include <osg/Version>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Matrix>
#include <osg/Plane>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>

#include <osgDB/fstream>

using namespace std;


#if defined(_MSC_VER)
    #pragma setlocale("C")
#endif

typedef pair<string, string>        NamePair;
typedef map<NamePair, unsigned int> NameMap;
typedef vector<string>              Words;

NamePair EmptyNamePair;
NamePair NameRobertOsfield("Robert", "Osfield");
NamePair NameDonBurns     ("Don",    "Burns");


const char* validNames[] =
{
    "de",
    "den",
    "van",
    "von"
};

const char* invalidNames[] =
{
    "Added",
    "AnimationPath",
    "AnimationPathCallback",
    "AnimationPathManipulator",
    "ArgumentParser",
    "AttrData",
    "AutoTransform",
    "AutoTransform.",
    "Azimuth",
    "BlenColour(Vec4",
    "Camera",
    "CameraBarrierCallback",
    "CameraNode",
    "Canvas",
    "CluserCullingCallback",
    "ClusterCullingCallback",
    "CoordinateSystem",
    "CoordinateSystemNode",
    "CoordinateSystemNode&",
    "CopyOp",
    "Copyright",
    "Core",
    "Creator",
    "CullCallbacks",
    "CullingSettngs",
    "CullVisitor",
    "CullVistor",
    "Cygwin",
    "DatabasePager",
    "DataSet",
    "DataVariance",
    "DeleteHandler",
    "DisplaySettings",
    "Drawable",
    "DrawElementsUByte",
    "Endian",
    "Escape",
    "EventQueue",
    "Face",
    "Files",
    "Fixed",
    "Fixes",
    "FluidProgram",
    "FontImplementation",
    "FrameStats",
    "Framstamp",
    "FreeBSD",
    "Geode",
    "GeoemtryTechnique",
    "GeometryTechnique",
    "GeoSet",
    "GraphicsContext",
    "GraphicsContextImplementation",
    "GraphicsThread",
    "GraphicsWindowX11",
    "GraphicsHandleX11",
    "Group",
    "Image",
    "ImageOptions",
    "Images",
    "Images/SolarSystem",
    "ImageStream",
    "Improved",
    "IndexBlock",
    "IndexFaceSets",
    "IntersectVisitor",
    "Inventor",
    "KdTree",
    "KeyboardMouse",
    "KeyboardMouseCallback",
    "LightModel",
    "LightPointNode",
    "LineSegment",
    "LineStipple",
    "Logos",
    "LongIDRecord",
    "InputIterator",
    "Make",
    "Makefile",
    "Material",
    "Matrix",
    "Matrix*",
    "MatrixManipulator",
    "MatrixTransform",
    "MemoryManager",
    "MergeGeometryVisitor",
    "MeshRecord",
    "Mode",
    "Mr",
    "Multigen",
    "New",
    "NewCullVisitor",
    "NodeVisitor",
    "Object",
    "OpenDX",
    "OpenSceneGraph",
    "OpenSceneGraph-2",
    "OpenSceneGraph-osgWidget-dev",
    "OpenThreads",
    "OperationThread",
    "Optimizer",
    "OrientationConverter",
    "Osfields",
    "Ouput",
    "Output",
    "OveralyNode",
    "PagedLOD",
    "PageLOD",
    "Paging",
    "ParentList",
    "Performer",
    "PickHandler",
    "PixelDataBufferObject",
    "PolygonStipple",
    "PolytopeVisitor",
    "PrimitiveIndexFunctor",
    "Prodcuer",
    "Producer",
    "ProducerEventCallback",
    "Program",
    "Proxy",
    "ProxyNode",
    "Quake3",
    "Quicktime",
    "ReaderWriters",
    "ReaderWriterTXP",
    "ReentrantMutex",
    "Referenced",
    "ReferenceFrame",
    "Removed",
    "RemoveLoadedProxyNodes",
    "RenderStage",
    "RenderTargetFallback",
    "RenderToTextureStage",
    "SceneGraphBuilder",
    "SceneView",
    "Sequence",
    "Shape",
    "SimpleViewer",
    "SimpleViewer",
    "SmokeTrailEffect",
    "Source",
    "SoftImage",
    "SpotExponent",
    "State",
    "StateAttribute",
    "Stats",
    "StateSet",
    "StatsVisitor",
    "Studio",
    "Support",
    "Switch",
    "TechniqueEventHandler",
    "TerraPage",
    "TessellationHints",
    "TestManipulator",
    "TestSupportCallback",
    "Text",
    "TexEnv",
    "TexMat(Matrix",
    "Texture",
    "Texture2DArray",
    "Texture3D",
    "TextureCubeMap",
    "TextureObjectManager",
    "TextureRectangle",
    "TextureRectangle(Image*",
    "TextureType",
    "Texuture",
    "TriStripVisitor",
    "TrPageViewer",
    "Uniform",
    "UseFarLineSegments",
    "UserData",
    "Valve",
    "Vec*",
    "Vec2d",
    "Vec3d",
    "Vec4ub",
    "Version",
    "VertexData",
    "View",
    "Viewer",
    "ViewPoint",
    "Viewport",
    "Viewport",
    "Visual",
    "VisualStudio",
    "WindowData",
    "Windows",
    "X",
    "Xcode",
    "GraphicsWindowWin32",
    "ImageLayer",
    "Xml",
    "Y",
    "UpdateSkeleton",
    "IncementalCompileOperation",
    "Billboard",
    "UpdateSkeleton",
    "Vec3",
    "Plugin",
    "Get",
    "LightSource",
    "SlideEventHandler",
    "GraphicsContext*",
    "Apple",
    "GeometryNew",
    "FrameBufferObject",
    "Lua",
    "VolumeTile",
    "PushStackValueVisitor",
    "RayIntersector",
    "OpenSceneGraph-Data",
    "Node",
    "AlphaFunc"
};


struct TypoCorrection
{
    const char* original;
    const char* corrected;
};

TypoCorrection typoCorrections[] =
{
    {"Cheaev",         "Chebaev"},
    {"Alaxandre",      "Alexandre"},
    {"Aderian",        "Adrian"},
    {"Adndre",         "Andre"},
    {"Adrain",         "Adrian"},
    {"Amueller",       "Aumueller"},
    {"Andew",          "Andrew"},
    {"AndrÃ©",         "André"},
    {"Antione",        "Antoine"},
    {"Antonoine",      "Antoine"},
    {"Aurelein",       "Aurelien"},
    {"Atr",            "Art"},
    {"Baverage",       "Beverage"},
    {"Bistroviae",     "Bistrovic"},
    {"Callue",         "Callu"},
    {"Christaiansen",  "Christiansen"},
    {"Cobin",          "Corbin"},
    {"Comporesi",      "Camporesi"},
    {"Connel",         "Connell"},
    {"Cullu",          "Callu"},
    {"Daneil",         "Daniel"},
    {"Daust",          "Daoust"},
    {"Daved",          "David"},
    {"Drederic",       "Frederic"},
    {"Eileman",        "Eilemann"},
    {"Elgi",           "Egli"},
    {"Fabian",         "Fabien"},
    {"Fautre",         "Fautré"},
    {"Frashid",        "Farshid"},
    {"Farshild",       "Farshid"},
    {"Fred",           "Frederic"},
    {"Fredrick",       "Frederic"},
    {"Fredric",        "Frederic"},
    {"Froechlich",     "Fröhlich"},
    {"Froehilch",      "Fröhlich"},
    {"Froehlich",      "Fröhlich"},
    {"Froelich",       "Fröhlich"},
    {"Froenlich",      "Fröhlich"},
    {"FrÃ¶hlich",      "Fröhlich"},
    {"Fruciel",        "Frauciel"},
    {"GarcÃ­a",        "Garcea"},
    {"Garrat",         "Garrett"},
    {"Garret",         "Garrett"},
    {"Geof",           "Geoff"},
    {"Giatan",         "Gaitan"},
    {"Gronenger",      "Gronager"},
    {"Gronger",        "Gronager"},
    {"Haritchablaet",  "Haritchabalet"},
    {"Hebelin",        "Herbelin"},
    {"Heirtlein",      "Hertlein"},
    {"Heirtlein",      "Hertlein"},
    {"Hertleinm",      "Hertlein"},
    {"Happalahti",     "Haapalahti"},
    {"Hertlien",       "Hertlein"},
    {"Hatwig",         "Hartwig"},
    {"Hauknes",        "Haukness"},
    {"Hunber",        "Huber"},
    {"Hi",             "He"},
    {"Hooper",         "Hopper"},
    {"Inverson",       "Iverson"},
    {"Iversion",       "Iverson"},
    {"Jean-Sebastein", "Jean-Sébastien"},
    {"Jean-Sebastian", "Jean-Sébastien"},
    {"Jean-Sebastirn", "Jean-Sébastien"},
    {"Jea-Sebastien",  "Jean-Sébastien"},
    {"Jean-Sebasien",  "Jean-Sébastien"},
    {"Jean-Sebastien", "Jean-Sébastien"},
    {"Jean-Sebestien", "Jean-Sébastien"},
    {"Johhansen",      "Johansen"},
    {"Johnansen",      "Johansen"},
    {"Johnasen",       "Johansen"},
    {"Jolly",          "Jolley"},
    {"Jose",           "José"},
    {"JosÃ©",          "José"},
    {"Joson",          "Jason"},
    {"J",              "José"},
    {"Keuhne",         "Kuehne"},
    {"Kheune",         "Kuehne"},
    {"Krulthof",       "Kruithof"},
    {"Lagrade",        "Lagarde"},
    {"Largade",        "Lagarde"},
    {"Largarde",       "Lagarde"},
    {"Larshkari",      "Lashkari"},
    {"Lashakari",      "Lashkari"},
    {"Lashari",        "Lashkari"},
    {"Lasharki",       "Lashkari"},
    {"Laskari",        "Lashkari"},
    {"Leandowski",     "Lewandowski"},
    {"Lawandowski",    "Lewandowski"},
    {"Luacas",         "Luaces"},
    {"Luacas",         "Luaces"},
    {"Liech",          "Leich"},
    {"Lownman",        "Lowman"},
    {"Lugi",           "Luigi"},
    {"Lweandowski",    "Lewandowski"},
    {"Lavingotte",     "Lavignotte"},
    {"Maceij",         "Maciej"},
    {"Machler",        "Mächler"},
    {"Macro",          "Marco"},
    {"Maechler",       "Mächler"},
    {"Mahai",          "Mihai"},
    {"Magnes",          "Magnus"},
    {"Mammond",        "Marmond"},
    {"March",          "Marco"},
    {"Martsz",         "Martz"},
    {"Marz",           "Martz"},
    {"Matz",           "Martz"},
    {"Melchoir",       "Melchior"},
    {"Mellis",         "Melis"},
    {"Messerschimdt",  "Messerschmidt"},
    {"Micheal",        "Michael"},
    {"Mihair",         "Mihai"},
    {"Molishtan",      "Moloshtan"},
    {"Molishtan",      "Moloshtan"},
    {"Moloshton",      "Moloshtan"},
    {"MornÃ©",         "Morné"},
    {"Morne",          "Morné"},
    {"Moule",          "Moiule"},
    {"Narache",        "Marache"},
    {"Nicklov",        "Nikolov"},
    {"Nickolov",       "Nikolov"},
    {"Nilson",         "Nilsson"},
    {"Olad",           "Olaf"},
    {"Olar",           "Olaf"},
    {"Oritz",          "Ortiz"},
    {"Osfeld",         "Osfield"},
    {"Osfied",         "Osfield"},
    {"Paulk",          "Paul"},
    {"Pail",           "Paul"},
    {"Perciva",        "Peciva"},
    {"Pecvia",         "Peciva"},
    {"Priyadashi",     "Priyadarshi"},
    {"Rajce",          "Trajce"},
    {"Raymon",         "Raymond"},
    {"Randal",         "Randall"},
    {"Robet",          "Robert"},
    {"Rodger",         "Roger"},
    {"Rolad",          "Roland"},
    {"Riddel",         "Riddell"},
    {"Rucard",         "Richard"},
    {"Sekender",       "Sukender"},
    {"Sewel",          "Sewell"},
    {"Simmonson",      "Simonsson"},
    {"Simmonsson",     "Simonsson"},
    {"Sjolie",         "Sjölie"},
    {"SjÃ¶lie",        "Sjölie"},
    {"Skinnder",       "Skinner"},
    {"Seberion",       "Seberino"},
    {"Sokolosky",      "Sokolowsky"},
    {"Sokolowky",      "Sokolowsky"},
    {"Sokolowki",      "Sokolowsky"},
    {"Sokolowski",     "Sokolowsky"},
    {"Sokolsky",       "Sokolowsky"},
    {"Sokolwsky",      "Sokolowsky"},
    {"Sonda",          "Sondra"},
    {"Stansilav",      "Stanislav"},
    {"Stefan",         "Stephan"},
    {"Stell",          "Steel"},
    {"Sylvan",         "Sylvain"},
    {"Takeahei",       "Takahei"},
    {"Takehei",        "Takahei"},
    {"Tery",           "Terry"},
    {"Tarantilils",    "Tarantilis"},
    {"Trastenjak",     "Trstenjak"},
    {"Urlich",         "Ulrich"},
    {"Vines",          "Vine"},
    {"Waldrom",        "Waldron"},
    {"Wedner",         "Weidner"},
    {"Weidemann",      "Wiedemann"},
    {"Wieblen",        "Weiblen"},
    {"Woesnner",       "Woessner"},
    {"Wojchiech",      "Wojciech"},
    {"Wojcoech",       "Wojciech"},
    {"Wojiech",        "Wojciech"},
    {"Xennon",         "Hanson"},
    {"Yefrei",         "Yefei"},
    {"Yfei",           "Yefei"},
    {"Gurhrie",        "Guthrie"},
    {"Byran",          "Bryan"},
    {"Fielder",        "Fiedler"},
    {"Mathia",         "Mathias"},
    {"Halgarth",       "Hogarth"},
    {"Katherina",      "Katharina"},
    {"Biyfarguine",    "Boufarguine"},
    {"Dickenson",       "Dickinson"},
    {"Jahannes","Johannes"},
    {"Eskland","Ekstrand"},
    {"Baeuerele","Baeuerle"},
    {"Bauerle","Baeuerle"},
    {"Baeurele","Baeuerle"},
    {"Nillson","Nilsson"},
    {"Bjorn","Björn"},
    {"BjÃ¶rn","Björn"},
    {"Stepan","Stephan"},
    {"Kristoger","Kristofer"},
    {"Blessing","Blissing"},
    {"Dannahuer","Dannhauer"},
    {"Chebeav", "Chebaev"},
    {"Messershmidt","Messerschmidt"},
    {"Auelien","Aurelien"},
    {"AurÃ©lien","Aurélien"},
    {"McDonnel","Mc Donnell"},
    {"McDonnell","Mc Donnell"},
    {"DelallÃ©e","Delallée"},
    {"GjÃ¸l","Gjøl"},
    {"RavÅ¡elj","Rav¨elj"},
    {"Ravsel",  "Rav¨elj"},
    {"Ravselj", "Rav¨elj"},
    {"Janik", "Jannik"},
    {"ViganÃ²", "Viganò"},
    {"Vigano", "Viganò"},
    {"Frashud", "Farshid"}
};


struct NameCorrection
{
    const char* originalFirst;
    const char* originalSecond;
    const char* correctedFirst;
    const char* correctedSecond;
};

NameCorrection nameCorrections[] =
{
    {"FrancoisTigeot","",
     "Francois", "Tigeot"},
    {"Juan","Manuel",
     "Juan", "Manuel Alvarez"},
    {"Jaap","Gas",
     "Jaap", "Glas"},
    {"Philip","Lamp",
     "Philip", "Lamb"},
    {"Dimi","Christop",
     "Dimi", "Christopoulos"},
    {"Jorge","Ciges",
     "Jorge", "Izquierdo Ciges"},
    {"Jorge","Izquierdo",
     "Jorge", "Izquierdo Ciges"},
    {"Rafa","Gata",
     "Rafa", "Gaitan"},
    {"Sukender","I",
     "Sukender", ""},
    {"Sukender","Here",
     "Sukender", ""},
    {"Sukender","Fix",
     "Sukender", ""},
    {"Ewe","Woessner",
     "Uwe", "Woessner"},
    {"Martin","von",
     "Martin", "von Gargern"},
    {"Alexandre","Irion",
     "Alexander", "Irion"},
    {"WojciechLewandowski","",
     "Wojciech", "Lewandowski"},
    {"Marin",          "Platings",
     "Michael",        "Platings"},
    {"Tomas",          "Holgarth",
     "Thomas",         "Hogarth"},
    {"Marin",          "Lavery",
     "Martin",         "Lavery"},
    {"Michael",        "Bach",
     "Michael",        "Bach Jensen"},
    {"Nguyen",         "Van",
     "Nguyen",         "Van Truong"},
    {"Thom",           "Carlo",
     "Thom",           "DeCarlo"},
    {"Stephan",        "Lamoliatte",
     "Stephane",       "Lamoliatte"},
    {"Ronald",         "van",
     "Ronald",         "van Maarseveen"},
    {"Lee",            "Bulter",
     "Lee",            "Butler"},
    {"Tery",           "Welsh",
     "Terry",          "Welsh"},
    {"Cesar",          "L",
     "César",          "L. B. Silveira"},
    {"Marc",           "Sciabica",
     "Mark",           "Sciabica"},
    {"Tom",            "Moore",
     "Tim",            "Moore"},
    {"Time",            "Moore",
     "Tim",            "Moore"},
    {"Jean",           "Sebastien",
     "Jean-Sébastien", "Guay"},
    {"Michale",        "Platings",
     "Michael",        "Platings"},
    {"Mick",           "",
     "Maik",           "Keller"},
    {"Gary",           "Quin",
     "Gary",           "Quinn"},
    {"BjornHein",      "",
     "Björn",          "Hein"},
    {"Bjorn",          "Hein",
     "Björn",          "Hein"},
    {"Erik",           "van",
     "Erik",           "den Dekker"},
    {"Erik",           "den",
     "Erik",           "den Dekker"},
    {"Jeoen",          "den",
     "Jeroen",         "den Dekker"},
    {"John",           "Vidar",
     "John",           "Vidar Larring"},
    {"John",           "Vida",
     "John",           "Vidar Larring"},
    {"Sebastien",      "Messerschmidt",
     "Sebastian",      "Messerschmidt"},
    {"Mattias",        "Fröhlich",
     "Mathias",        "Fröhlich"},
    {"Mathias",        "Helsing",
     "Mattias",        "Helsing"},
    {"Jose",           "Delport",
     "J.P.",           "Delport"},
    {"José",           "Delport",
     "J.P.",           "Delport"},
    {"Franz",          "Melchior",
     "Melchior",       "Franz"},
    {"Glen",           "Waldon",
     "Glenn",          "Waldron"},
    {"Glen",           "Waldron",
     "Glenn",          "Waldron"},
    {"Ralf",           "Karn",
     "Ralf",           "Kern"},
    {"Donny",          "Cipperly",
     "Donald",         "Cipperly"},
    {"Gino",           "van",
     "Gino",           "van den Bergen"},
    {"Radu",           "Mihai",
     "Mihai",          "Radu"},
    {"Art",            "Trevs",
     "Art",            "Tevs"},
    {"Tim",            "More",
     "Tim",            "Moore"},
    {"Andre",          "Garneau",
     "André",          "Garneau"},
    {"Eric",           "Hammil",
     "Chris",          "Hanson"},
    {"Paul",           "de",
     "Paul",           "de Repentigny"},
    {"Raymond",        "de",
     "Raymond",        "de Vries"},
    {"Nick",           "",
     "Trajce",         "Nikolov"},
    {"Daniel",         "",
     "Daniel",         "Sjölie"},
    {"Julia",          "Ortiz",
     "Julian",         "Ortiz"},
    {"Rune",           "Schmidt",
     "Rune",           "Schmidt Jensen"},
    {"Romano",         "José",
     "Romano",         "José Magacho da Silva"},
    {"Rommano",        "Silva",
     "Romano",         "José Magacho da Silva"},
    {"Romano",         "Magacho",
     "Romano",         "José Magacho da Silva"},
    {"Leandro",        "Motta",
     "Leandro",        "Motta Barros"},
    {"Leandro",        "Motto",
     "Leandro",        "Motta Barros"},
    {"A",              "Botorabi",
     "Ali",            "Botorabi"},
    {"Waltice",        "",
     "Walter",         "J. Altice"},
    {"Drew",           "",
     "Drew",           "Whitehouse"},
    {"Douglas",        "A",
     "Douglas",        "A. Pouk"},
    {"Colin",          "MacDonald",
     "Colin",          "McDonald"},
    {"Nikolov",        "Trajce",
     "Trajce",         "Nikolov"},
    {"Frauciel",       "Luc",
     "Luc",            "Frauciel"},
    {"Alberto",        "Lucas",
     "Alberto",        "Luaces"},
    {"Alberto",        "Luacus",
     "Alberto",        "Luaces"},
    {"Tyge",           "",
     "Tyge",           "Løvset"},
    {"Ricard",         "Schmidt",
     "Richard",        "Schmidt"},
    {"Matthias",       "Helsing",
     "Mattias",        "Helsing"},
    {"Clement",        "Boesch",
     "Clément",        "B½sch"},
    {"Lauren",         "Voerman",
     "Laurens",        "Voerman"},
    {"Pjotr",          "Sventachov",
     "Pjotr",          "Svetachov"},
     {"Bradley",       "Baker",
      "Bradley",       "Baker Searles"},
     {"PawelKsiezopolski", "",
      "Pawel",          "Ksiezopolski"},
     {"Albert", "Luaces",
      "Alberto","Luaces"},
     {"KOS", "",
      "Konstantin","Matveyev"},
     {"WeSee", "",
       "Alois", "Wismer"},
     {"We", "See",
       "Alois", "Wismer"}
};


bool validName(const string& first)
{
    // Check for valid names
    for (unsigned int i = 0; i < sizeof(validNames) / sizeof(char*); ++i)
    {
        if (first == validNames[i])
        {
            return true;
        }
    }

    if (first.empty()) return false;

    if (first[0] < 'A' || first[0] > 'Z') return false;

    if (first.size() >= 2 && (first[1] < 'a' || first[1] > 'z') && (first[1] != '.') && (first[1] != ',')) return false;

    // Check for invalid names
    for (unsigned int i = 0; i < sizeof(invalidNames) / sizeof(char*); ++i)
    {
        if (first == invalidNames[i])
        {
            return false;
        }
    }

    // Default to a valid name
    return true;
}


string typoCorrection(const string& name)
{
    // Loop over all known typo corrections
    for (unsigned int i = 0; i < sizeof(typoCorrections) / sizeof(TypoCorrection); ++i)
    {
        // If a typo is found
        if (name == typoCorrections[i].original)
        {
            // Return the correction
            return typoCorrections[i].corrected;
        }
    }

    return name;
}


void nameCorrection(NamePair& name)
{
    // Loop over all known name corrections
    for (unsigned int i = 0; i < sizeof(nameCorrections) / sizeof(NameCorrection); ++i)
    {
        // If a matching name is found
        if (name.first  == nameCorrections[i].originalFirst &&
            name.second == nameCorrections[i].originalSecond)
        {
            // Return the correction
            name.first  = nameCorrections[i].correctedFirst;
            name.second = nameCorrections[i].correctedSecond;

            // Early out, since we don't want corrections of corrections
            return;
        }
    }
}


void lastValidCharacter(const string& name, unsigned int& pos, char c)
{
    for (unsigned int i = 0; i < pos; ++i)
    {
        if (name[i] == c)
        {
            pos = i;
            return;
        }
    }
}


void lastValidCharacter(const string& name, unsigned int& last)
{
    lastValidCharacter(name, last, '.');
    lastValidCharacter(name, last, ',');
    lastValidCharacter(name, last, '\'');
    lastValidCharacter(name, last, '/');
    lastValidCharacter(name, last, '\\');
    lastValidCharacter(name, last, ':');
    lastValidCharacter(name, last, ';');
    lastValidCharacter(name, last, ')');
}


NamePair createName(const string& first, const string& second)
{
    if (first.empty()) return EmptyNamePair;

//  cout << "first = " << first << " second = " << second << endl;

    unsigned int last = first.size();
    lastValidCharacter(first, last);

    if (last == 0) return EmptyNamePair;

    string name;

    name.append(first.begin(), first.begin() + last);

    if (!validName(name)) return EmptyNamePair;

    name = typoCorrection(name);

    if (second.empty() ||
        !validName(second))
    {
        // filter any single or two letter words as unlike to be names.
        if (name.size() <= 2) return EmptyNamePair;

        return NamePair(name, "");
    }

    last = second.size();

    lastValidCharacter(second, last);

    if (last > 0)
    {
        string surname(second.begin(), second.begin() + last);

        if (validName(surname))
        {
            surname = typoCorrection(surname);
            return NamePair(name, surname);
        }
    }

    // filter any single or two letter words as unlike to be names.
    if (name.size() <= 2) return EmptyNamePair;

    return NamePair(name, "");
}


bool submissionsSequence(const Words& words, unsigned int& i)
{
    if (i + 1 >= words.size()) return false;

    if (words[i] == "From"   ||
        words[i] == "from"   ||
        words[i] == "From:"  ||
        words[i] == "from:"  ||
        words[i] == "Merged" ||
        words[i] == "Integrated") return true;

    if (i + 2 >= words.size()) return false;

    if (words[i] == "submitted" && words[i + 1] == "by")
    {
        i += 1;
        return true;
    }

    if (words[i] == "Folded" && words[i + 1] == "in")
    {
        i += 1;
        return true;
    }

    if (words[i] == "Rolled" && words[i + 1] == "in")
    {
        i += 1;
        return true;
    }

    if (words[i] == "Checked" && words[i + 1] == "in")
    {
        i += 1;
        return true;
    }

    if (i + 3 >= words.size()) return false;

    if (words[i] == "sent" && words[i + 1] == "in" && words[i + 2] == "by")
    {
        i += 2;
        return true;
    }

    return false;
}


void readContributors(NameMap& names, const string& file)
{
    osgDB::ifstream fin(file.c_str());

    Words words;
    while(fin)
    {
        string keyword;
        fin >> keyword;
        words.push_back(keyword);
    }

    string blank_string;

    for(unsigned int i = 0; i < words.size(); ++i)
    {
        if (submissionsSequence(words, i))
        {
            if (i + 2 < words.size() && validName(words[i + 1]))
            {
                NamePair name = createName(words[i + 1], words[i + 2]);
                nameCorrection(name);
                if (!name.first.empty()) ++names[name];
                i += 2;
            }
            else if (i + 1 < words.size() && validName(words[i + 1]))
            {
                NamePair name = createName(words[i + 1], blank_string);
                nameCorrection(name);
                if (!name.first.empty()) ++names[name];
                i += 1;
            }
        }
        else
        {
            if (words[i] == "robert")
            {
                ++names[NameRobertOsfield];
            }
            else if (words[i] == "don")
            {
                ++names[NameDonBurns];
            }
        }
    }

    // reassign fisrt name entries to their full names entries
    if (names.size() > 1)
    {
        for (NameMap::iterator itr = names.begin(); itr != names.end(); )
        {
            if (itr->first.second.empty())
            {
                NameMap::iterator next_itr = itr;
                ++next_itr;

                if (next_itr != names.end() && itr->first.first == next_itr->first.first)
                {
                    next_itr->second += itr->second;
                    names.erase(itr);
                    itr = next_itr;
                }
                else
                {
                    ++itr;
                }
            }
            else
            {
                ++itr;
            }
        }
    }

    // remove the double entries from Robert's contributions
    if (names.size() > 1)
    {
        for (NameMap::iterator itr = names.begin(); itr != names.end(); ++itr)
        {
            if (itr->first != NameRobertOsfield && itr->first != NameDonBurns)
            {
                names[NameRobertOsfield] -= itr->second;
            }
        }
    }
}


void buildContributors(NameMap& names)
{
    // top five contributors
    ++names[NamePair("Robert",     "Osfield")];
    ++names[NamePair("Don",        "Burns")];
    ++names[NamePair("Marco",      "Jez")];
    ++names[NamePair("Mike",       "Weiblen")];
    ++names[NamePair("Geoff",      "Michel")];
    ++names[NamePair("Ben",        "van Basten")];

    // contributors that don't appear in the ChangeLog due to their contributions
    // being before CVS started for the OSG, or before the name logging began.
    ++names[NamePair("Karsten",    "Weiss")];
    ++names[NamePair("Graeme",     "Harkness")];
    ++names[NamePair("Axel",       "Volley")];
    ++names[NamePair("Nikolaus",   "Hanekamp")];
    ++names[NamePair("Kristopher", "Bixler")];
    ++names[NamePair("Tanguy",     "Fautré")];
    ++names[NamePair("J.E.",       "Hoffmann")];
}


void printContributors(const std::string& changeLog, bool printNumEntries)
{
    NameMap names;
    buildContributors(names);
    if (!changeLog.empty())
    {
        readContributors(names, changeLog);
    }

    typedef multimap<unsigned int, NamePair> SortedNameMap;

    SortedNameMap sortedNames;
    for (NameMap::iterator itr = names.begin(); itr != names.end(); ++itr)
    {
        sortedNames.insert(SortedNameMap::value_type(itr->second, itr->first));
    }

    cout << names.size() << " Contributors:" << endl << endl;

    if (printNumEntries)
    {
        cout << "Entries Firstname Surname" << endl;
        cout << "-------------------------" << endl;
        for (SortedNameMap::reverse_iterator sitr = sortedNames.rbegin(); sitr != sortedNames.rend(); ++sitr)
        {
            cout << sitr->first << "\t" << sitr->second.first;
            if (!sitr->second.second.empty()) cout << " " << sitr->second.second;
            cout << endl;
        }
    }
    else
    {
        cout << "Firstname Surname" << endl;
        cout << "-----------------" << endl;
        for (SortedNameMap::reverse_iterator sitr = sortedNames.rbegin(); sitr != sortedNames.rend(); ++sitr)
        {
            cout << sitr->second.first;
            if (!sitr->second.second.empty()) cout << " " << sitr->second.second;
            cout << endl;
        }
    }
}
