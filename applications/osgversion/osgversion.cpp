#include <osg/Notify>
#include <osg/Version>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Matrix>
#include <osg/Plane>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>

#include <osgDB/fstream>

#include <OpenThreads/Version>

#include <set>
#include <vector>
#include <iostream>

// the majority of the application is dedicated to building the
// current contribitors list by parsing the ChangeLog, it just takes
// one line in the main itself to report the version number.

#if defined(_MSC_VER)
    #pragma setlocale("C")
#endif

typedef std::pair<std::string, std::string> NamePair;
typedef std::map<NamePair,unsigned int> NameMap;
typedef std::vector< std::string > Words;

NamePair EmptyNamePair;
NamePair NameRobertOsfield("Robert","Osfield");
NamePair NameDonBurns("Don","Burns");

bool validName(const std::string& first)
{
    if (first=="de") return true;
    if (first=="den") return true;
    if (first=="van") return true;


    if (first.empty()) return false;
    if (first[0]<'A' || first[0]>'Z') return false;
    
    if (first.size()>=2 && (first[1]<'a' || first[1]>'z') && (first[1]!='.')  && (first[1]!=',')) return false; 

    if (first=="Xcode") return false;
    if (first=="Added") return false;
    if (first=="Camera") return false;
    if (first=="CameraNode") return false;
    if (first=="CopyOp") return false;
    if (first=="Fixed") return false;
    if (first=="View") return false;
    if (first=="GraphicsContext") return false;
    if (first=="WindowData") return false;
    if (first=="ViewPoint") return false;
    if (first=="PickHandler") return false;
    if (first=="Program") return false;
    if (first=="Object") return false;
    if (first=="OpenSceneGraph") return false;
    if (first=="SpotExponent") return false;
    if (first=="Framstamp") return false;
    if (first=="Stats") return false;
    if (first=="Group") return false;
    if (first=="Texture") return false;
    if (first=="Texture2DArray") return false;
    if (first=="Creator") return false;
    if (first=="CullVisitor") return false;
    if (first=="Drawable") return false;
    if (first=="Geode") return false;
    if (first=="GeoSet") return false;
    if (first=="Image") return false;
    if (first=="Face") return false;
    if (first=="Images/SolarSystem") return false;
    if (first=="IntersectVisitor") return false;
    if (first=="LongIDRecord") return false;
    if (first=="Makefile") return false;
    if (first=="Matrix") return false;
    if (first=="MemoryManager") return false;
    if (first=="MeshRecord") return false;
    if (first=="Multigen") return false;
    if (first=="NewCullVisitor") return false;
    if (first=="Output") return false;
    if (first=="PageLOD") return false;
    if (first=="Improved") return false;
    if (first=="PagedLOD") return false;
    if (first=="Referenced") return false;
    if (first=="StateAttribute") return false;
    if (first=="Switch") return false;
    if (first=="TechniqueEventHandler") return false;
    if (first=="Uniform") return false;
    if (first=="Vec*") return false;
    if (first=="Viewer") return false;
    if (first=="VisualStudio") return false;
    if (first=="X") return false;
    if (first=="Y") return false;
    if (first=="Producer") return false;
    if (first=="New") return false;
    if (first=="Removed") return false;
    if (first=="Ouput") return false;
    if (first=="ReaderWriters") return false;
    if (first=="NodeVisitor") return false;
    if (first=="Fixes") return false;
    if (first=="FontImplementation") return false;
    if (first=="DisplaySettings") return false;
    if (first=="AnimationPath") return false;
    if (first=="AnimationPathCallback") return false;
    if (first=="AnimationPathManipulator") return false;
    if (first=="ArgumentParser") return false;
    if (first=="AttrData") return false;
    if (first=="Azimuth") return false;
    if (first=="CluserCullingCallback") return false;
    if (first=="ClusterCullingCallback") return false;
    if (first=="CoordinateSystem") return false;
    if (first=="CoordinateSystemNode") return false;
    if (first=="CoordinateSystemNode&") return false;
    if (first=="Copyright") return false;
    if (first=="Cygwin") return false;
    if (first=="CullCallbacks") return false;
    if (first=="CullingSettngs") return false;
    if (first=="DataVariance") return false;
    if (first=="DatabasePager") return false;
    if (first=="DrawElementsUByte") return false;
    if (first=="Escape") return false;
    if (first=="FluidProgram") return false;
    if (first=="FrameStats") return false;
    if (first=="FreeBSD") return false;
    if (first=="GraphicsContextImplementation") return false;
    if (first=="GraphicsThread") return false;
    if (first=="Images") return false;
    if (first=="IndexBlock") return false;
    if (first=="Inventor") return false;
    if (first=="Make") return false;
    if (first=="Material") return false;
    if (first=="MergeGeometryVisitor") return false;
    if (first=="Mode") return false;
    if (first=="Prodcuer") return false;
    if (first=="ProxyNode") return false;
    if (first=="ReentrantMutex") return false;
    if (first=="ReferenceFrame") return false;
    if (first=="RemoveLoadedProxyNodes") return false;
    if (first=="RenderTargetFallback") return false;
    if (first=="RenderToTextureStage") return false;
    if (first=="Sequence") return false;
    if (first=="Shape") return false;
    if (first=="TessellationHints") return false;
    if (first=="Support") return false;
    if (first=="State") return false;
    if (first=="SmokeTrailEffect") return false;
    if (first=="TexEnv") return false;
    if (first=="Texture3D") return false;
    if (first=="TextureCubeMap") return false;
    if (first=="TextureObjectManager") return false;
    if (first=="TextureRectangle(Image*") return false;
    if (first=="TextureType") return false;
    if (first=="Texuture") return false;
    if (first=="TriStripVisitor") return false;
    if (first=="UserData") return false;
    if (first=="Viewport") return false;
    if (first=="Visual") return false;
    if (first=="Studio") return false;
    if (first=="Vec2d") return false;
    if (first=="Vec3d") return false;
    if (first=="Windows") return false;
    if (first=="Version") return false;
    if (first=="Viewport") return false;
    if (first=="Core") return false;
    if (first=="DataSet") return false;
    if (first=="Endian") return false;
    if (first=="ImageOptions") return false;
    if (first=="ImageStream") return false;
    if (first=="KeyboardMouse") return false;
    if (first=="KeyboardMouseCallback") return false;
    if (first=="AutoTransform") return false;
    if (first=="AutoTransform.") return false;
    if (first=="LightModel") return false;
    if (first=="MatrixManipulator") return false;
    if (first=="MatrixTransform") return false;
    if (first=="OpenDX") return false;
    if (first=="ParentList") return false;
    if (first=="TerraPage") return false;
    if (first=="OveralyNode") return false;
    if (first=="OpenThreads") return false;
    if (first=="PolygonStipple") return false;
    if (first=="SceneView") return false;
    if (first=="PrimitiveIndexFunctor") return false;
    if (first=="PolytopeVisitor") return false;
    if (first=="Performer") return false;
    if (first=="Paging") return false;
    if (first=="CameraBarrierCallback") return false;
    if (first=="TestSupportCallback") return false;
    if (first=="Quake3") return false;
    if (first=="BlenColour(Vec4") return false;
    if (first=="UseFarLineSegments") return false;
    if (first=="TextureRectangle") return false;
    if (first=="DeleteHandler") return false;
    if (first=="EventQueue") return false;
    if (first=="TrPageViewer") return false;
    if (first=="TestManipulator") return false;
    if (first=="ProducerEventCallback") return false;
    if (first=="OrientationConverter") return false;
    if (first=="Logos") return false;
    if (first=="StatsVisitor") return false;
    if (first=="LineStipple") return false;
    if (first=="Files") return false;
    if (first=="Mr") return false;
    if (first=="Osfields") return false;
    if (first=="Optimizer") return false;
    if (first=="RenderStage") return false;
    if (first=="Matrix*") return false;
    if (first=="Vec4ub") return false;
    if (first=="Proxy") return false;
    if (first=="CullVistor") return false;
    if (first=="SimpleViewer") return false;
    if (first=="TexMat(Matrix") return false;
    if (first=="GraphicsWindowX11") return false;
    if (first=="OperationThread") return false;
    if (first=="SimpleViewer") return false;
    if (first=="IndexFaceSets") return false;
    if (first=="Quicktime") return false;
    if (first=="SceneGraphBuilder") return false;
    if (first=="LightPointNode") return false;
    if (first=="GeometryTechnique") return false;
    if (first=="GeoemtryTechnique") return false;
    if (first=="KdTree") return false;
    if (first=="LineSegment") return false;
    if (first=="Canvas") return false;
    if (first=="OpenSceneGraph-2") return false;
    if (first=="OpenSceneGraph-osgWidget-dev") return false;
    if (first=="Valve") return false;
    if (first=="Source") return false;
    if (first=="PixelDataBufferObject") return false;
    return true;
}

std::string typoCorrection(const std::string& name)
{
#if 0
    if (name=="") return "";
    if (name=="") return "";
    if (name=="") return "";
    if (name=="") return "";
#endif
    if (name=="Aderian")  return "Adrian";
    if (name=="Adndre") return "Andre";
    if (name=="Adrain") return "Adrian";
    if (name=="Andew") return "Andrew";
    if (name=="AndrÃ©") return "André";
    if (name=="Antione") return "Antoine";
    if (name=="Antonoine") return "Antoine";
    if (name=="Atr")  return "Art";
    if (name=="Baverage") return "Beverage";
    if (name=="Bistroviae") return "Bistrovic";
    if (name=="Callue")  return "Callu";
    if (name=="Christaiansen") return "Christiansen";
    if (name=="Cobin") return "Corbin";
    if (name=="Comporesi") return "Camporesi";
    if (name=="Connel") return "Connell";
    if (name=="Cullu") return "Callu";
    if (name=="Daneil") return "Daniel";
    if (name=="Daust") return "Daoust";
    if (name=="Daved") return "David";
    if (name=="Drederic") return "Frederic";
    if (name=="Eileman") return "Eilemann";
    if (name=="Elgi") return "Egli";
    if (name=="Frashid") return "Farshid";
    if (name=="Fred") return "Frederic";
    if (name=="Fredrick") return "Frederic";
    if (name=="Fredric") return "Frederic";
    if (name=="Froechlich") return "Fröhlich";
    if (name=="Froehilch") return "Fröhlich";
    if (name=="Froehlich") return "Fröhlich";
    if (name=="Froelich") return "Fröhlich";    
    if (name=="Froenlich") return "Fröhlich";
    if (name=="FrÃ¶hlich") return "Fröhlich";
    if (name=="Fruciel") return "Frauciel";
    if (name=="GarcÃ­a") return "Garcea";
    if (name=="Garrat") return "Garrett";
    if (name=="Garret") return "Garrett";
    if (name=="Geof") return "Geoff";
    if (name=="Giatan") return "Gaitan";
    if (name=="Gronenger") return "Gronager";
    if (name=="Gronger") return "Gronager";
    if (name=="Hebelin") return "Herbelin";
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Hertleinm") return "Hertlein";
    if (name=="Hertlien") return "Hertlein";
    if (name=="Hi") return "He";
    if (name=="Hooper") return "Hopper";
    if (name=="Inverson") return "Iverson";
    if (name=="Iversion") return "Iverson";
    if (name=="Jean-Sebastian") return "Jean-Sebastien";
    if (name=="Jean-Sebastirn") return "Jean-Sebastien";
    if (name=="Jea-Sebastien") return "Jean-Sebastien";
    if (name=="Johhansen") return "Johansen";
    if (name=="Johnansen") return "Johansen";
    if (name=="Johnasen") return "Johansen";
    if (name=="Jolly") return "Jolley";
    if (name=="Jose") return "José";
    if (name=="JosÃ©") return "José";
    if (name=="Joson") return "Jason";
    if (name=="J") return "José";
    if (name=="Keuhne") return "Kuehne";
    if (name=="Kheune") return "Kuehne";
    if (name=="Lagrade") return "Lagarde";
    if (name=="Larshkari") return "Lashkari";
    if (name=="Lashakari") return "Lashkari";
    if (name=="Lashari") return "Lashkari";
    if (name=="Lasharki") return "Lashkari";
    if (name=="Laskari") return "Lashkari";
    if (name=="Leandowski" || name=="Lawandowski")  return "Lewandowski";
    if (name=="Lucas") return "Luaces";
    if (name=="Lugi") return "Luigi";
    if (name=="Lweandowski")  return "Lewandowski";
    if (name=="Machler") return "Mächler";
    if (name=="Macro") return "Marco";
    if (name=="Maechler") return "Mächler";
    if (name=="Mahai")  return "Mihai";
    if (name=="Mammond") return "Marmond";
    if (name=="March") return "Marco";
    if (name=="Martsz") return "Martz";
    if (name=="Marz") return "Martz";
    if (name=="Matz")  return "Martz";
    if (name=="Melchoir") return "Melchior";
    if (name=="Mellis") return "Melis";
    if (name=="Messerschimdt") return "Messerschmidt";
    if (name=="Micheal") return "Michael";
    if (name=="Mihair")  return "Mihai";
    if (name=="Molishtan") return "Moloshtan";
    if (name=="Molishtan") return "Moloshtan";
    if (name=="Moloshton") return "Moloshtan";
    if (name=="MornÃ©") return "Morné";
    if (name=="Morne") return "Morné";
    if (name=="Moule") return "Moiule";
    if (name=="Narache") return "Marache";
    if (name=="Nicklov") return "Nikolov";
    if (name=="Nickolov") return "Nikolov";
    if (name=="Olad") return "Olaf";
    if (name=="Olar") return "Olaf";
    if (name=="Oritz") return "Ortiz";
    if (name=="Osfeld")  return "Osfield";
    if (name=="Osfied") return "Osfield";
    if (name=="Pail") return "Paul";
    if (name=="Rajce") return "Trajce";
    if (name=="Randal") return "Randall";
    if (name=="Robet") return "Robert";
    if (name=="Rodger") return "Roger";
    if (name=="Rolad") return "Roland";
    if (name=="Rucard")  return "Richard";
    if (name=="Sekender") return "Sukender";
    if (name=="Sewel") return "Sewell";
    if (name=="Simmonson") return "Simonsson";
    if (name=="Simmonsson") return "Simonsson";
    if (name=="Sjolie") return "Sjölie";
    if (name=="SjÃ¶lie") return "Sjölie";
    if (name=="Skinnder") return "Skinner";
    if (name=="Sokolosky") return "Sokolowsky";
    if (name=="Sokolowki") return "Sokolowsky";
    if (name=="Sokolowski") return "Sokolowsky";
    if (name=="Sokolsky") return "Sokolowsky";
    if (name=="Sokolwsky") return "Sokolowsky";
    if (name=="Sonda") return "Sondra";
    if (name=="Stansilav") return "Stanislav";
    if (name=="Stefan") return "Stephan";
    if (name=="Stell") return "Steel";
    if (name=="Sylvan") return "Sylvain";
    if (name=="Takeahei") return "Takahei";
    if (name=="Takehei") return "Takahei";
    if (name=="Tarantilils") return "Tarantilis";
    if (name=="Trastenjak") return "Trstenjak";
    if (name=="Urlich") return "Ulrich";
    if (name=="Vines") return "Vine";
    if (name=="Waldrom")  return "Waldron";
    if (name=="Wedner") return "Weidner";
    if (name=="Weidemann") return "Wiedemann";
    if (name=="Wieblen") return "Weiblen";
    if (name=="Woesnner") return "Woessner";
    if (name=="Wojiech" || name=="Wojchiech")  return "Wojciech";
    if (name=="Xennon") return "Hanson";
    if (name=="Yefrei") return "Yefei";
    if (name=="Yfei") return "Yefei";
    if (name=="Jean-Sebastein") return "Jean-Sebastien";
    if (name=="Haritchablaet") return "Haritchabalet";
    if (name=="Fautre") return "Fautré";
    if (name=="Maceij") return "Maciej";
    if (name=="Fabian") return "Fabien";
    return name;
}

void nameCorrection(NamePair& name)
{
    if (name.first=="Michale" && name.second=="Platings")
    {
        name.first = "Michael";
        name.second = "Platings";
    }
    
    if (name.first=="Mick" && name.second=="")
    {
        name.first = "Maik";
        name.second = "Keller";
    }
    
    if (name.first=="Gary" && name.second=="Quin")
    {
        name.first = "Gary";
        name.second = "Quinn";
    }
    
    if (name.first=="BjornHein" && name.second=="")
    {
        name.first = "Björn";
        name.second = "Hein";
    }
    
    if (name.first=="Mattias" && name.second=="Fröhlich")
    {
        name.first = "Mathias";
        name.second = "Fröhlich";
    }
    
    if (name.first=="Mathias"  && name.second=="Helsing")
    {
        name.first = "Mattias";
        name.second = "Helsing";
    }
    
    if (name.first=="Bjorn" && name.second=="Hein")
    {
        name.first = "Björn";
        name.second = "Hein";
    }

    if (name.first=="Erik" && name.second=="van")
    {
        name.first = "Erik";
        name.second = "den Dekker";
    }
    
    if (name.first=="Erik" && name.second=="den")
    {
        name.first = "Erik";
        name.second = "den Dekker";
    }
    
    if (name.first=="Jeoen" && name.second=="den")
    {
        name.first = "Jeoen";
        name.second = "den Dekker";
    }
    
    if (name.first=="John" && name.second=="Vidar")
    {
        name.first = "John";
        name.second = "Vidar Larring";
    }

    if (name.first=="John" && name.second=="Vida")
    {
        name.first = "John";
        name.second = "Vidar Larring";
    }

    if (name.first=="Sebastien" && name.second=="Messerschmidt")
    {
        name.first = "Sebastian";
        name.second = "Messerschmidt";
    }

    if ((name.first=="Jose" || name.first=="José") && name.second=="Delport")
    {
        name.first = "J.P.";
        name.second = "Delport";
    }
    if (name.first=="Franz" && name.second=="Melchior")
    {
        name.first = "Melchior";
        name.second = "Franz";
    }
    if (name.first=="Glen" && name.second=="Waldon")
    {
        name.first = "Glenn";
        name.second = "Waldron";
    }
    if (name.first=="Ralf" && name.second=="Karn")
    {
        name.first = "Ralf";
        name.second = "Kern";
    }
    if (name.first=="Donny" && name.second=="Cipperly")
    {
        name.first = "Donald";
        name.second = "Cipperly";
    }
    if (name.first=="Gino" && name.second=="van")
    {
        name.first = "Gino";
        name.second = "van den Bergen";
    }
    if (name.first=="Radu" && name.second=="Mihai")
    {
        name.first = "Mihai";
        name.second = "Radu";
    }
    if (name.first=="Art" && name.second=="Trevs")
    {
        name.first = "Art";
        name.second = "Tevs";
    }
    if (name.first=="Tim" && name.second=="More")
    {
        name.first = "Tim";
        name.second = "Moore";
    }
    if (name.first=="Andre" && name.second=="Garneau")
    {
        name.first = "André";
        name.second = "Garneau";
    }
    if (name.first=="Eric" && name.second=="Hammil")
    {
        name.first = "Chris";
        name.second = "Hanson";
    }
    if (name.first=="Paul" && name.second=="de")
    {
        name.first = "Paul";
        name.second = "de Repentigny";
    }
    if (name.first=="Raymond" && name.second=="de")
    {
        name.first = "Raymond";
        name.second = "de Vries";
    }
    if (name.first=="Nick" && name.second=="")
    {
        name.first = "Trajce";
        name.second = "Nikolov";
    }
    if (name.first=="Daniel" && name.second=="")
    {
        name.first = "Daniel";
        name.second = "Sjölie";
    }
    if (name.first=="Julia" && name.second=="Ortiz")
    {
        name.first = "Julian";
        name.second = "Ortiz";
    }
    if (name.first=="Rune" && name.second=="Schmidt")
    {
        name.first = "Rune";
        name.second = "Schmidt Jensen";
    }
    if (name.first=="Romano" && name.second=="José")
    {
        name.first = "Romano";
        name.second = "José Magacho da Silva";
    }
    if (name.first=="Rommano" && name.second=="Silva")
    {
        name.first = "Romano";
        name.second = "José Magacho da Silva";
    }
    if (name.first=="Leandro" && name.second=="Motta")
    {
        name.first = "Leandro";
        name.second = "Motta Barros";
    }
    if (name.first=="A" && name.second=="Botorabi")
    {
        name.first = "Ali";
        name.second = "Botorabi";
    }

    if (name.first=="Waltice" && name.second=="")
    {
        name.first = "Walter";
        name.second = "J. Altice";
    }

    if (name.first=="Drew" && name.second=="")
    {
        name.first = "Drew";
        name.second = "Whitehouse";
    }
    if (name.first=="Douglas" && name.second=="A")
    {
        name.first = "Douglas";
        name.second = "A. Pouk";
    }
    if (name.first=="Colin" && name.second=="MacDonald")
    {
        name.first = "Colin";
        name.second = "McDonald";
    }
    if (name.first=="Nikolov" && name.second=="Trajce")
    {
        name.first = "Trajce";
        name.second = "Nikolov";
    }
    if (name.first=="Frauciel" && name.second=="Luc")
    {
        name.first = "Luc";
        name.second = "Frauciel";
    }
}

void lastValidCharacter(const std::string& name, unsigned int& pos,char c)
{
    for(unsigned int i=0;i<pos;++i)
    {
        if (name[i]==c)
        {
            pos = i;
            return;
        }
    }
}

void lastValidCharacter(const std::string& name, unsigned int& last)
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



NamePair createName(const std::string& first, const std::string& second)
{
    if (first.empty()) return EmptyNamePair;
    
    // osg::notify(osg::NOTICE)<<"first = "<<first<<" second = "<<second<<std::endl;
    
    unsigned int last = first.size();
    lastValidCharacter(first, last);
    
    if (last==0) return EmptyNamePair;
    
    std::string name;
    
    name.append(first.begin(), first.begin()+last);

    if (!validName(name)) return EmptyNamePair;

    name = typoCorrection(name);
    
    if (second.empty() ||
        !validName(second)) 
    {
        // filter any single or two letter words as unlike to be names.
        if (name.size()<=2) return EmptyNamePair;

        return NamePair(name,"");
    }

    last = second.size();
    
    lastValidCharacter(second, last);
    
    if (last>0)
    {
        std::string surname(second.begin(), second.begin()+last);
        
        if (validName(surname))
        {
            surname = typoCorrection(surname);
            return NamePair(name, surname);
        }
    }
    
    // filter any single or two letter words as unlike to be names.
    if (name.size()<=2) return EmptyNamePair;
    
    return NamePair(name,"");
}

bool submissionsSequence(const Words& words, unsigned int& i)
{
    if (i+1>=words.size()) return false;
    
    if (words[i]=="From" || 
        words[i]=="from" || 
        words[i]=="From:" || 
        words[i]=="from:" || 
        words[i]=="Merged" || 
        words[i]=="Integrated") return true;
        
    if (i+2>=words.size()) return false;
    
    if (words[i]=="submitted" && words[i+1]=="by")
    {
        i+=1;
        return true;
    }
        
    if (words[i]=="Folded" && words[i+1]=="in")
    {
        i+=1;
        return true;
    }

    if (words[i]=="Rolled" && words[i+1]=="in")
    {
        i+=1;
        return true;
    }

    if (words[i]=="Checked" && words[i+1]=="in")
    {
        i+=1;
        return true;
    }

    if (i+3>=words.size()) return false;

    if (words[i]=="sent" && words[i+1]=="in" && words[i+2]=="by")
    {
        i+=2;
        return true;
    }

    return false;
}

void readContributors(NameMap& names, const std::string& file)
{
    osgDB::ifstream fin(file.c_str());
    
    Words words;
    while(fin)
    {
        std::string keyword;
        fin >> keyword;
        words.push_back(keyword);
    }
    
    std::string blank_string;
    
    for(unsigned int i=0; i< words.size(); ++i)
    {
        if (submissionsSequence(words,i)) 
        {
            if (i+2<words.size() && validName(words[i+1]))
            {
                NamePair name = createName(words[i+1], words[i+2]);
                nameCorrection(name);
                if (!name.first.empty()) ++names[name];
                i+=2;
            }
            else if (i+1<words.size() && validName(words[i+1]))
            {
                NamePair name = createName(words[i+1], blank_string);
                nameCorrection(name);
                if (!name.first.empty()) ++names[name];
                i+=1;
            }
        }
        else
        {
            if (words[i]=="robert") 
            {
                ++names[NameRobertOsfield];
            }
            else if (words[i]=="don")
            {
                ++names[NameDonBurns];
            }
        }
    }

    // reassign fisrt name entries to their full names entries
    if (names.size()>1)
    {
        for(NameMap::iterator itr = names.begin();
            itr != names.end();
            )
        {
            if (itr->first.second.empty()) 
            {
                NameMap::iterator next_itr = itr;
                ++next_itr;
                
                if (next_itr!=names.end() && itr->first.first==next_itr->first.first)
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
    if (names.size()>1)
    {        
        for(NameMap::iterator itr = names.begin();
            itr != names.end();
            ++itr)
        {
            if (itr->first != NameRobertOsfield && itr->first != NameDonBurns ) 
            {
                names[NameRobertOsfield] -= itr->second;
            }
        }
    }
    
    
}

void buildContributors(NameMap& names)
{
    // top five contributors
    ++names[NamePair("Robert","Osfield")];
    ++names[NamePair("Don","Burns")];
    ++names[NamePair("Marco","Jez")];
    ++names[NamePair("Mike","Weiblen")];
    ++names[NamePair("Geoff","Michel")];
    ++names[NamePair("Ben","van Basten")];
    
    // contributors that don't appear in the ChangeLog due to their contributions
    // being before CVS started for the OSG, or before the name logging began.
    ++names[NamePair("Karsten","Weiss")];
    ++names[NamePair("Graeme","Harkness")];
    ++names[NamePair("Axel","Volley")];
    ++names[NamePair("Nikolaus","Hanekamp")];
    ++names[NamePair("Kristopher","Bixler")];
    ++names[NamePair("Tanguy","Fautré")];
    ++names[NamePair("J.E.","Hoffmann")];
}

int main( int argc, char **argv)
{
    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--entries","Print out number of entries into the ChangeLog file for each contributor.");
    arguments.getApplicationUsage()->addCommandLineOption("--version-number","Print out version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--major-number","Print out major version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--minor-number","Print out minor version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--patch-number","Print out patch version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--so-number ","Print out shared object version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--openthreads-version-number","Print out version number for OpenThreads only");
    arguments.getApplicationUsage()->addCommandLineOption("--openthreads-soversion-number","Print out shared object version number for OpenThreads only");
    arguments.getApplicationUsage()->addCommandLineOption("Matrix::value_type","Print the value of Matrix::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("Plane::value_type","Print the value of Plane::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("BoundingSphere::value_type","Print the value of BoundingSphere::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("BoundingBox::value_type","Print the value of BoundingBox::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("-r <file> or --read <file>","Read the ChangeLog to generate an estimated contributors list.");

    if (arguments.read("--version-number"))
    {
        std::cout<<osgGetVersion()<<std::endl;
        return 0;
    }
    
    if (arguments.read("--major-number"))
    {
        std::cout<<OPENSCENEGRAPH_MAJOR_VERSION<<std::endl;
        return 0;
    }
    
    if (arguments.read("--minor-number"))
    {
        std::cout<<OPENSCENEGRAPH_MINOR_VERSION<<std::endl;
        return 0;
    }
    
    if (arguments.read("--patch-number"))
    {
        std::cout<<OPENSCENEGRAPH_PATCH_VERSION<<std::endl;
        return 0;
    }
    
    if (arguments.read("--soversion-number") || arguments.read("--so-number") )
    {
        std::cout<<osgGetSOVersion()<<std::endl;
        return 0;
    }
    

    if (arguments.read("--openthreads-version-number"))
    {
        std::cout<<OpenThreadsGetVersion()<<std::endl;
        return 0;
    }
    
    
    if (arguments.read("--openthreads-major-number"))
    {
        std::cout<<OPENTHREADS_MAJOR_VERSION<<std::endl;
        return 0;
    }
    
    if (arguments.read("--openthreads-minor-number"))
    {
        std::cout<<OPENTHREADS_MINOR_VERSION<<std::endl;
        return 0;
    }
    
    if (arguments.read("--openthreads-patch-number"))
    {
        std::cout<<OPENTHREADS_PATCH_VERSION<<std::endl;
        return 0;
    }

    if (arguments.read("--openthreads-soversion-number"))
    {
        std::cout<<OpenThreadsGetSOVersion()<<std::endl;
        return 0;
    }


    if (arguments.read("Matrix::value_type"))
    {
        std::cout<<((sizeof(osg::Matrix::value_type)==4)?"float":"double")<<std::endl;
        return 0;
    }

    if (arguments.read("Plane::value_type"))
    {
        std::cout<<((sizeof(osg::Plane::value_type)==4)?"float":"double")<<std::endl;
        return 0;
    }

    if (arguments.read("BoundingSphere::value_type"))
    {
        std::cout<<((sizeof(osg::BoundingSphere::value_type)==4)?"float":"double")<<std::endl;
        return 0;
    }
    if (arguments.read("BoundingBox::value_type"))
    {
        std::cout<<((sizeof(osg::BoundingBox::value_type)==4)?"float":"double")<<std::endl;
        return 0;
    }
    
    std::cout<<osgGetLibraryName()<< " "<< osgGetVersion()<<std::endl<<std::endl;

    bool printContributors = false;
    bool printNumEntries = false;
    while ( arguments.read("--entries"))
    {
        printContributors = true;
        printNumEntries = true;
    }

    std::string changeLog;
    while ( arguments.read("-r",changeLog) || arguments.read("--read",changeLog)) printContributors = true;

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        std::cout<<arguments.getApplicationUsage()->getCommandLineUsage()<<std::endl;
        arguments.getApplicationUsage()->write(std::cout,arguments.getApplicationUsage()->getCommandLineOptions());
        return 1;
    }

    if (printContributors)
    {
        NameMap names;
        buildContributors(names);
        if (!changeLog.empty())
        {
            readContributors(names, changeLog);
        }
        
        typedef std::multimap<unsigned int, NamePair> SortedNameMap;

        SortedNameMap sortedNames;        
        for(NameMap::iterator itr = names.begin();
            itr != names.end();
            ++itr)
        {
            sortedNames.insert(SortedNameMap::value_type(itr->second, itr->first));
        }

        std::cout<<names.size()<<" Contributors:"<<std::endl<<std::endl;
        
        if (printNumEntries)
        {
            std::cout<<"Entries Firstname Surname"<<std::endl;
            std::cout<<"-------------------------"<<std::endl;
            for(SortedNameMap::reverse_iterator sitr = sortedNames.rbegin();
                sitr != sortedNames.rend();
                ++sitr)
            {
                std::cout<<sitr->first<<"\t"<<sitr->second.first<<" "<<sitr->second.second<<std::endl;
            }
        }
        else
        {
            std::cout<<"Firstname Surname"<<std::endl;
            std::cout<<"-----------------"<<std::endl;
            for(SortedNameMap::reverse_iterator sitr = sortedNames.rbegin();
                sitr != sortedNames.rend();
                ++sitr)
            {
                std::cout<<sitr->second.first<<" "<<sitr->second.second<<std::endl;
            }
        }
    }

    return 0;
}
