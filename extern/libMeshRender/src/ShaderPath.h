#pragma once
#ifndef SHADER_PATH_H
#define SHADER_PATH_H

#include <sys/stat.h>
#include <vector>
#include <string>

namespace MeshRender{

class ShaderPath{
    
    public:
        explicit ShaderPath(const std::string &shader_home_env = std::string());

        void addPath(const std::string &path);

        void clearPaths();

        bool getFilePath(const std::string &file, std::string &path); 

        std::string loadShaderFromResource(const std::string &filepath);

    private:
        std::vector<std::string> m_pathList;

};
    
extern ShaderPath sdkPath;

} //end namespace

#endif 