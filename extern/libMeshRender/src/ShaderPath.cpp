#include <string>
#include <map>
#include "AllShaders.h"
#include "ShaderPath.h"

#include "shaders/flatshading_vp.glsl"
#include "shaders/flatshading_fp.glsl"
#include "shaders/gouraudshading_vp.glsl"
#include "shaders/gouraudshading_fp.glsl"
#include "shaders/hiddenline_gp.glsl"
#include "shaders/phongshading_vp.glsl"
#include "shaders/phongshading_fp.glsl"
#include "shaders/shading.glsl"

#ifndef nullptr
#define nullptr 0
#endif

static std::string genKeyFromShaderPath(const std::string &filepath)
{
    std::string key;
    int pos = filepath.rfind('/');
    if (std::string::npos == pos)
        pos = filepath.rfind('\\');
    if (std::string::npos == pos) 
        key = filepath;
    else
        key = filepath.substr(pos + 1);
    
    for (unsigned int i=0; i<key.size(); i++){
        if (key[i] == '.' || key[i] == '-')
            key[i] = '_';
    }
    return key;
}

namespace MeshRender{

ShaderPath sdkPath;

ShaderPath::ShaderPath(const std::string &shader_home_env) :
    m_pathList()
{
	if (shader_home_env.empty()) return;

	const char *env = getenv(shader_home_env.c_str());
	if (nullptr != env){
		std::string envstr(env);
		const char lastChar = envstr[envstr.length() - 1];
		if (lastChar != '/' && lastChar != '\\')
			envstr.push_back('/');
		m_pathList.push_back(envstr);
	}
}

void ShaderPath::addPath(const std::string &path)
{
	m_pathList.push_back(path);
}

void ShaderPath::clearPaths()
{
	m_pathList.clear();
}

bool ShaderPath::getFilePath(const std::string &file, std::string &path)
{
    const std::string key = genKeyFromShaderPath(file);
    std::map<std::string, const char*>::iterator itr = shadersMap.find(key);
    if (itr != shadersMap.end()){
        path = file;
        return true;
    }

	struct stat fileInfo;
	if (stat(file.c_str(), &fileInfo) == 0){
		path = file;
		return true;
	}

	std::vector<std::string>::iterator it;
	for (it = m_pathList.begin(); it != m_pathList.end(); it++){
		const std::string pathString = *it + file;
		if (stat(pathString.c_str(), &fileInfo) == 0){
			path = pathString;
			return true;
		}
	}

    path = std::string("");
	return false;
}

std::string ShaderPath::loadShaderFromResource(const std::string &filepath)
{
    const std::string key = genKeyFromShaderPath(filepath);
    std::string text("");
    std::map<std::string, const char*>::iterator itr = shadersMap.find(key);
    if (itr != shadersMap.end()){
        text = std::string(itr->second);
    }

    return text;
}


    
} //end namespace
