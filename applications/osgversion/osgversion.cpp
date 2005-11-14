#include <osg/Version>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <set>
#include <vector>
#include <iostream>
#include <fstream>

typedef std::pair<std::string, std::string> NamePair;
typedef std::set<NamePair> NameSet;

NamePair EmptyNamePair;


bool validName(const std::string& first)
{
    if (first.empty()) return false;
    if (first[0]<'A' || first[0]>'Z') return false;
    
    if (first.size()>=2 && (first[1]<'a' || first[1]>'z')) return false; 

    if (first=="Added") return false;
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
    return true;
}

std::string typoCorrection(const std::string& name)
{
#if 0
    if (name=="") return "Tarantilils";
    if (name=="") return "";
    if (name=="") return "";
    if (name=="") return "";
#endif
    if (name=="Heirtlein") return "Hertlein";
    if (name=="Fredric") return "Frederic";
    if (name=="Geof") return "Geoff";
    if (name=="Sewel") return "Sewell";
    if (name=="Moule") return "Moiule";
    if (name=="Macro") return "Marco";
    if (name=="March") return "Marco";
    if (name=="Gronenger") return "Gronager";
    if (name=="Gronger") return "Gronager";
    if (name=="Wieblen") return "Weiblen";
    if (name=="Molishtan") return "Moloshtan";
    if (name=="Bistroviae") return "Bistrovic";
    if (name=="Christaiansen") return "Christiansen";
    if (name=="Daust") return "Daoust";
    if (name=="Daved") return "David";
    if (name=="Fred") return "Frederic";
    if (name=="Fredrick") return "Frederic";
    if (name=="Garrat") return "Garret";
    if (name=="Heirtlein") return "Heirtlein";
    if (name=="Hertlien") return "Hertlein";
    if (name=="Hi") return "He";
    if (name=="Inverson") return "Iverson";
    if (name=="Iversion") return "Iverson";
    if (name=="Jeoen") return "Joran";
    if (name=="Johnasen") return "Johansen";
    if (name=="Johhansen") return "Johansen";
    if (name=="Johnansen") return "Johansen";
    if (name=="Jolley") return "Jolly";
    if (name=="J") return "Jose";
    if (name=="Keuhne") return "Kuehne";
    if (name=="Kheune") return "Kuehne";
    if (name=="Lashari") return "Lashkari";
    if (name=="Laskari") return "Lashkari";
    if (name=="Mammond") return "Marmond";
    if (name=="Marz") return "Martz";
    if (name=="Molishtan") return "Molishtan";
    if (name=="Moloshton") return "Moloshtan";
    if (name=="Moloshton") return "Moloshtan";
    if (name=="Nicklov") return "Nikolov";
    if (name=="Olad") return "Olaf";
    if (name=="Osfied") return "Osfield";
    if (name=="Pail") return "Paul";
    if (name=="Sokolosky") return "Sokolowsky";
    if (name=="Sokolsky") return "Sokolowsky";
    if (name=="Sonda") return "Sondra";
    if (name=="Stansilav") return "Stanislav";
    if (name=="Stefan") return "Stephan";
    if (name=="Stell") return "Steel";
    if (name=="Xennon") return "Hanson";
    if (name=="Yfei") return "Yefei";
    return name;
}

void nameCorrection(NamePair& name)
{
    if (name.first=="Eric" && name.second=="Hammil")
    {
        name.first = "Chris";
        name.second = "Hanson";
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
    
    if (second.empty()) return NamePair(name,"");
    
    if (!validName(second)) return NamePair(name,"");

    last = second.size();
    
    lastValidCharacter(second, last);
    
    if (last>0)
    {
        std::string surname(second.begin(), second.begin()+last);
        surname = typoCorrection(surname);
    
        return NamePair(name, surname);
    }
    
    return NamePair(name,"");
}

void readContributors(NameSet& names, const std::string& file)
{
    std::cout<<"readContributions(names,"<<file<<")"<<std::endl;

    std::ifstream fin(file.c_str());
    
    typedef std::vector< std::string > Words;
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
        if (words[i]=="From" || 
            words[i]=="from" || 
            words[i]=="From:" || 
            words[i]=="from:") 
        {
            if (i+2<words.size() && validName(words[i+1]))
            {
                NamePair name = createName(words[i+1], words[i+2]);
                nameCorrection(name);
                if (!name.first.empty()) names.insert(name);
                i+=2;
            }
            else if (i+1<words.size() && validName(words[i+1]))
            {
                NamePair name = createName(words[i+1], blank_string);
                nameCorrection(name);
                if (!name.first.empty()) names.insert(name);
                i+=1;
            }
        }
    }

    if (names.size()>1)
    {
        for(NameSet::iterator itr = names.begin();
            itr != names.end();
            )
        {
            if (itr->second.empty()) 
            {
                NameSet::iterator next_itr = itr;
                ++next_itr;
                
                if (next_itr!=names.end() && itr->first==next_itr->first)
                {
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
}

void buildContributors(NameSet& names)
{
    names.insert(NamePair("Robert","Osfield"));
    names.insert(NamePair("Don","Burns"));
    names.insert(NamePair("Marco","Jez"));
}

int main( int argc, char **argv)
{
    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-c or --contributors","Print out the contributors list.");
    arguments.getApplicationUsage()->addCommandLineOption("-r <file> or --read <file>","Read the changelog to generate an estimated contributors list.");

    printf( "%s\n", osgGetVersion() );

    bool printContributors = false;
    while ( arguments.read("-c") || arguments.read("--contributors")) printContributors = true;

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
        NameSet names;
        buildContributors(names);
        if (!changeLog.empty())
        {
            readContributors(names, changeLog);
        }
        
        std::cout<<names.size()<<" Contributors:"<<std::endl;
        for(NameSet::iterator itr = names.begin();
            itr != names.end();
            ++itr)
        {
            std::cout<<"  "<<itr->first<<" "<<itr->second<<std::endl;
        }
    }

    return 0;
}
