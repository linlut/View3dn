#ifndef _CIA3D_SYSTEM_SETDIRECTORY_H
#define _CIA3D_SYSTEM_SETDIRECTORY_H

#include <string>

namespace cia3d
{

namespace system
{

// A small utility class to temporarly set the current directory to the 
// same as a specified file
class SetDirectory
{
public:
    std::string previousDir;
    std::string directory;

    SetDirectory(const char* filename);
    SetDirectory(const std::string& filename);

    ~SetDirectory();

    /// Get the current directory
    static std::string GetCurrentDir();

    /// Get the parent directory of a given file, i.e. if given "a/b/c", return "a/b".
    static std::string GetParentDir(const char* filename);

    /// Get the full path of the current process. The given filename should be the value of argv[0].
    static std::string GetProcessFullPath(const char* filename);

    /// Get the file relative to a directory, i.e. if given "../e" and "a/b/c", return "a/b/e".
    static std::string GetRelativeFromDir(const char* filename, const char* basename);

    /// Get the file relative to another file path, i.e. if given "../e" and "a/b/c", return "a/e".
    static std::string GetRelativeFromFile(const char* filename, const char* basename);

    /// Get the file relative to current process path, i.e. if given "../e" and "a/b/c", return "a/e".
    static std::string GetRelativeFromProcess(const char* filename, const char* basename=NULL);
};

} // namespace system
} // namespace sofa

#endif
