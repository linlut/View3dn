#include <crest/system/setdirectory.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <direct.h>
#endif

#include <string.h>
#include <iostream>

namespace cia3d
{
namespace system
{

SetDirectory::SetDirectory(const char* filename)
{
    directory = GetParentDir(filename);
    if (!directory.empty()){
        std::cout << ">chdir("<<directory<<")"<<std::endl;
	previousDir = GetCurrentDir();
#ifndef WIN32
        chdir(directory.c_str());
#else
        _chdir(directory.c_str());
#endif
    }
}

SetDirectory::SetDirectory(const std::string& filename)
{
    directory = GetParentDir(filename.c_str());
    if (!directory.empty()){
        std::cout << ">chdir("<<directory<<")"<<std::endl;
		previousDir = GetCurrentDir();
#ifndef WIN32
        chdir(directory.c_str());
#else
        _chdir(directory.c_str());
#endif
    }
}

SetDirectory::~SetDirectory()
{
    if (!directory.empty() && !previousDir.empty())
    {
        std::cout << "<chdir("<<previousDir<<")"<<std::endl;
#ifndef WIN32
        chdir(previousDir.c_str());
#else
        _chdir(previousDir.c_str());
#endif
    }
}

/// Get the current directory
std::string SetDirectory::GetCurrentDir()
{
    char dir[1024];
    memset(dir,0,sizeof(dir));
#ifndef WIN32
    getcwd(dir, sizeof(dir));
#else
    _getcwd(dir, sizeof(dir));
#endif
    return dir;
}

std::string SetDirectory::GetParentDir(const char* filename)
{
    std::string s = filename;
    std::string::size_type pos = s.find_last_of("/\\");
    if (pos == std::string::npos)
        return ""; // no directory
    else
        return s.substr(0,pos);
}

std::string SetDirectory::GetRelativeFromDir(const char* filename, const char* basename)
{
    if (!filename || !filename[0]) return "";
    if (filename[0] == '/') return filename; // absolute path
    std::string base = basename;
    std::string s = filename;
    // remove any ".."
    while ((s.substr(0,3)=="../" || s.substr(0,3)=="..\\") && !base.empty())
    {
        s = s.substr(3);
        base = GetParentDir(base.c_str());
    }
    if (base.empty())
        return s;
    else if (base[base.length()-1] == '/')
	return base + s;
    else
        return base + "/" + s;
}

std::string SetDirectory::GetRelativeFromFile(const char* filename, const char* basename)
{
    std::string base = GetParentDir(basename);
    return GetRelativeFromDir(filename, base.c_str());
}

std::string SetDirectory::GetRelativeFromProcess(const char* filename, const char* basename)
{
    std::string base = GetProcessFullPath(basename);
    return GetRelativeFromFile(filename, base.c_str());
}

/// Get the full path of the current process. The given filename should be the value of argv[0].
std::string SetDirectory::GetProcessFullPath(const char* filename)
{
#ifdef WIN32
    if (!filename || !filename[0])
    {
        //return __argv[0];
        int n=0;
	LPWSTR wpath = *CommandLineToArgvW(GetCommandLineW(),&n);
	if (wpath)
	{
	    char path[1024];
	    memset(path,0,sizeof(path));
	    wcstombs(path, wpath, sizeof(path)-1);
	    //std::cout << "Current process: "<<path<<std::endl;
	    if (path[0]) return path;
	}
    }
    /// \TODO use GetCommandLineW and/or CommandLineToArgvW. This is however not strictly necessary, as argv[0] already contains the full path in most cases.
#else
    if (!filename || filename[0]!='/')
    {
	char path[1024];
	memset(path,0,sizeof(path));
	readlink("/proc/self/exe",path,sizeof(path)-1);
	//std::cout << "Current process: "<<path<<std::endl;
	if (path[0]) return path;
	
    }
#endif
    return filename;
}

} // namespace system

} // namespace sofa

