//FILE: filerepository.cpp

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <crest/system/filerepository.h>
#include <crest/system/setdirectory.h>


namespace cia3d
{
namespace system
{

#ifdef WIN32
	FileRepository DataRepository("CIA3D_DATA_PATH", "../scenes;../share;../data");
#else
	FileRepository DataRepository("CIA3D_DATA_PATH", "../scenes:../share:../data");
#endif

FileRepository::FileRepository(const char* envVar, const char* relativePath)
{
    if (envVar != NULL && envVar[0]!='\0')
    {
        const char* envpath = getenv(envVar);
        if (envpath != NULL && envpath[0]!='\0')
            addFirstPath(envpath);
    }
    if (relativePath != NULL && relativePath[0]!='\0')
    {
        std::string path = relativePath;
        size_t p0 = 0;
        size_t p1;
        while ( p0 < path.size() )
        {
            p1 = path.find(entrySeparator(),p0);
            if (p1 == std::string::npos) p1 = path.size();
            if (p1>p0+1)
            {
                std::string p = path.substr(p0,p1-p0);
                addLastPath(SetDirectory::GetRelativeFromProcess(p.c_str()));
            }
            p0 = p1+1;
        }
    }
}

FileRepository::~FileRepository()
{
}

void FileRepository::addFirstPath(const std::string& path)
{
    std::vector<std::string> entries;
    size_t p0 = 0;
    size_t p1;
    while ( p0 < path.size() )
    {
        p1 = path.find(entrySeparator(),p0);
        if (p1 == std::string::npos) p1 = path.size();
        if (p1>p0+1)
        {
            entries.push_back(path.substr(p0,p1-p0));
        }
        p0 = p1+1;
    }
    vpath.insert(vpath.begin(), entries.begin(), entries.end());
}

void FileRepository::addLastPath(const std::string& path)
{
    std::vector<std::string> entries;
    size_t p0 = 0;
    size_t p1;
    while ( p0 < path.size() )
    {
        p1 = path.find(entrySeparator(),p0);
        if (p1 == std::string::npos) p1 = path.size();
        if (p1>p0+1)
        {
            entries.push_back(path.substr(p0,p1-p0));
        }
        p0 = p1+1;
    }
    vpath.insert(vpath.end(), entries.begin(), entries.end());
}

bool FileRepository::findFileIn(std::string& filename, const std::string& path)
{
    if (filename.empty()) return false; // no filename
    struct stat s;
    std::string newfname = SetDirectory::GetRelativeFromDir(filename.c_str(), path.c_str());
    //std::cout << "Looking for " << newfname <<std::endl;
    if (!stat(newfname.c_str(),&s))
    {
        // File found
        //std::cout << "File "<<filename<<" found in "<<path.substr(p0,p1-p0)<<std::endl;
        filename = newfname;
        return true;
    }
    return false;
}

bool FileRepository::findFile(std::string& filename, const std::string& basedir)
{
    if (filename.empty()) return false; // no filename
    if (findFileIn(filename, basedir)) return true;
    if (filename[0]=='/') return false; // absolute file path
    if (filename.substr(0,2)=="./" || filename.substr(0,3)=="../") return false; // absolute or local file path
    for (std::vector<std::string>::const_iterator it = vpath.begin(); it != vpath.end(); ++it)
        if (findFileIn(filename, *it)) return true;
    std::cerr << "File "<<filename<<" NOT FOUND"<<std::endl;
    return false;
}

bool FileRepository::findFileFromFile(std::string& filename, const std::string& basefile)
{
    return findFile(filename, SetDirectory::GetParentDir(basefile.c_str()));
}

void FileRepository::print()
{
    for (std::vector<std::string>::const_iterator it = vpath.begin(); it != vpath.end(); ++it)
        std::cout << *it << std::endl;
}


} // namespace system
} // namespace cia3d

