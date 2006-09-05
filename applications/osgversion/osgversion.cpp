#include <osg/Version>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <set>
#include <vector>
#include <iostream>
#include <fstream>

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


    if (first.empty()) return false;
    if (first[0]<'A' || first[0]>'Z') return false;
    
    if (first.size()>=2 && (first[1]<'a' || first[1]>'z') && (first[1]!='.')  && (first[1]!=',')) return false; 

    if (first=="Added") return false;
    if (first=="Camera") return false;
    if (first=="CameraNode") return false;
    if (first=="CopyOp") return false;
    if (first=="Fixed") return false;
    if (first=="Creator") return false;
    if (first=="CullVisitor") return false;
    if (first=="Drawable") return false;
    if (first=="Geode") return false;
    if (first=="GeoSet") return false;
    if (first=="Image") return false;
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
    if (first=="TexMat(Matrix") return false;
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
    if (name=="Trastenjak") return "Trstenjak";
    if (name=="Baverage") return "Beverage";
    if (name=="Bistroviae") return "Bistrovic";
    if (name=="Christaiansen") return "Christiansen";
    if (name=="Cobin") return "Corbin";
    if (name=="Connel") return "Connell";
    if (name=="Daust") return "Daoust";
    if (name=="Daved") return "David";
    if (name=="Fred") return "Frederic";
    if (name=="Fredrick") return "Frederic";
    if (name=="Fredric") return "Frederic";
    if (name=="Garrat") return "Garret";
    if (name=="Geof") return "Geoff";
    if (name=="Gronenger") return "Gronager";
    if (name=="Gronger") return "Gronager";
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Hertlien") return "Hertlein";
    if (name=="Hi") return "He";
    if (name=="Hooper") return "Hopper";
    if (name=="Inverson") return "Iverson";
    if (name=="Iversion") return "Iverson";
    if (name=="Jeoen") return "Joran";
    if (name=="Johhansen") return "Johansen";
    if (name=="Johnansen") return "Johansen";
    if (name=="Johnasen") return "Johansen";
    if (name=="Jolly") return "Jolley";
    if (name=="Jose") return "José";
    if (name=="Joson") return "Jason";
    if (name=="J") return "José";
    if (name=="Keuhne") return "Kuehne";
    if (name=="Kheune") return "Kuehne";
    if (name=="Lashakari") return "Lashkari";
    if (name=="Lashari") return "Lashkari";
    if (name=="Lasharki") return "Lashkari";
    if (name=="Laskari") return "Lashkari";
    if (name=="Macro") return "Marco";
    if (name=="Mammond") return "Marmond";
    if (name=="March") return "Marco";
    if (name=="Marz") return "Martz";
    if (name=="Micheal") return "Michael";
    if (name=="Molishtan") return "Moloshtan";
    if (name=="Molishtan") return "Moloshtan";
    if (name=="Moloshton") return "Moloshtan";
    if (name=="Moule") return "Moiule";
    if (name=="Nicklov") return "Nikolov";
    if (name=="Olad") return "Olaf";
    if (name=="Oritz") return "Ortiz";
    if (name=="Osfied") return "Osfield";
    if (name=="Pail") return "Paul";
    if (name=="Randal") return "Randall";
    if (name=="Rodger") return "Roger";
    if (name=="Sewel") return "Sewell";
    if (name=="Sjolie") return "Sjölie";
    if (name=="Sokolosky") return "Sokolowsky";
    if (name=="Sokolowski") return "Sokolowsky";
    if (name=="Sokolsky") return "Sokolowsky";
    if (name=="Sonda") return "Sondra";
    if (name=="Stansilav") return "Stanislav";
    if (name=="Stefan") return "Stephan";
    if (name=="Stell") return "Steel";
    if (name=="Takeahei") return "Takahei";
    if (name=="Takehei") return "Takahei";
    if (name=="Tarantilils") return "Tarantilis";
    if (name=="Vines") return "Vine";
    if (name=="Wieblen") return "Weiblen";
    if (name=="Xennon") return "Hanson";
    if (name=="Yefrei") return "Yefei";
    if (name=="Yfei") return "Yefei";
    if (name=="Antonoine") return "Antoine";
    if (name=="Andew") return "Andrew";
    if (name=="Daneil") return "Daniel";
    return name;
}

void nameCorrection(NamePair& name)
{
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
        words[i]=="Applied" || 
        words[i]=="Added" || 
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
    std::ifstream fin(file.c_str());
    
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
    arguments.getApplicationUsage()->addCommandLineOption("-r <file> or --read <file>","Read the ChangeLog to generate an estimated contributors list.");

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
