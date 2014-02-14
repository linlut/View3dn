#if _MSC_VER
    #include <windows.h>
#endif
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/gl.h>
#include "ShaderPath.h"
#include "ShadersManagement.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifdef _MSC_VER
    #define snprintf _snprintf_s
#endif

using namespace std;
static std::map<string, string> shadersMacroMap;

//Text file loading for shaders sources
static std::string loadTextFile(const char *name)
{
    const std::string text = MeshRender::sdkPath.loadShaderFromResource(name);
    if (!text.empty()) return text;

	std::ifstream file;
	file.open(name);
	if(file.fail()){
		std::cerr<<"loadFile: unable to open file: "<<name;
		return std::string();
	}

	std::string buff;
	buff.reserve(128*1024);
	std::string line;
    while(std::getline(file, line)){
		buff += line + "\n";
    }
	return buff;
}

static bool checkProgramInfo(GLuint programID, GLuint stat, const char *filename = nullptr)
{
	GLint ok = 0;
    if (filename == nullptr) filename = "";
	glGetProgramiv(programID, stat, &ok);

	if ( ok != 0 ){
		int ilength;
        const int STRING_BUFFER_SIZE = 4096;
		char stringBuffer[STRING_BUFFER_SIZE];
		glGetProgramInfoLog(programID, STRING_BUFFER_SIZE, &ilength, stringBuffer);
		std::cerr << "Program error :\n";
        std::cerr << "File: " << filename << "\n";
        std::cerr << stringBuffer << "\n"; 
	}

    return (0 == ok);
}

static void defineMacro(std::string &shaderSource, const char *macro, const char *value)
{
	char buff[512];
	snprintf(buff, sizeof(buff), "#define %s", macro);
	int mstart = (int)shaderSource.find(buff);
	snprintf(buff, sizeof(buff), "#define %s %s\n", macro, value);
	if (mstart >= 0){
		int mlen = (int)shaderSource.find("\n", mstart) - mstart + 1 ;
		std::string prevval=shaderSource.substr(mstart, mlen);
		if( strcmp(prevval.c_str(), buff) ){
			shaderSource.replace(mstart, mlen, buff);
		}
	}
    else{
		shaderSource.insert(0, buff);
	}
}

//=======================================================
namespace MeshRender{

static string manageIncludes(const std::string& src, const std::string& sourceFileName);

void resetShadersGlobalMacros()
{
	shadersMacroMap.clear();
}

void setShadersGlobalMacro(const char *macro, int val)
{
	char buff[128];
	snprintf(buff, sizeof(buff), "%d", val);
    shadersMacroMap[std::string(macro)] = buff;
}

void setShadersGlobalMacro(const char *macro, float val)
{
	char buff[128];
	snprintf(buff, sizeof(buff), "%ff", val);
    shadersMacroMap[std::string(macro)] = buff;
}

void setShadersGlobalMacro(const char *macro, const char* val)
{
    shadersMacroMap[std::string(macro)] = val;
}

//GLSL shader program creation
GLuint createShaderProgram(const char *fileNameVS, const char *fileNameGS, const char *fileNameFS, GLuint programID)
{
	//Create a new GLSL program
	bool reload = (programID != 0);
	if(!reload){
		programID = glCreateProgram();
	}

	GLsizei count;
	GLuint shaders[3];
	glGetAttachedShaders(programID, 3, &count, shaders);
	GLuint vertexShaderID = 0;
	GLuint geometryShaderID = 0;
	GLuint fragmentShaderID = 0;

	for(GLsizei i=0; i<count; i++){
		GLint shadertype;
		glGetShaderiv(shaders[i], GL_SHADER_TYPE, &shadertype);
		if(shadertype == GL_VERTEX_SHADER){
			vertexShaderID = shaders[i];
		}else if(shadertype == GL_GEOMETRY_SHADER){
			geometryShaderID = shaders[i];
		}else if(shadertype == GL_FRAGMENT_SHADER){
			fragmentShaderID = shaders[i];
		}
	}

	if(fileNameVS){
		// Create vertex shader
		vertexShaderID=createShader(fileNameVS, GL_VERTEX_SHADER, vertexShaderID);
		if(!reload){
			// Attach vertex shader to program object
			glAttachShader(programID, vertexShaderID);
		}
	}

	if(fileNameGS){
		// Create geometry shader
		geometryShaderID=createShader(fileNameGS, GL_GEOMETRY_SHADER, geometryShaderID);
		if(!reload){
			// Attach vertex shader to program object
			glAttachShader(programID, geometryShaderID);
		}
	}
	
	if(fileNameFS){
		// Create fragment shader
		fragmentShaderID = createShader(fileNameFS, GL_FRAGMENT_SHADER, fragmentShaderID);
		if(!reload){
			// Attach fragment shader to program object
			glAttachShader(programID, fragmentShaderID);
		}
	}

	return programID;
}

void linkShaderProgram(GLuint programID)
{
	// Link all shaders togethers into the GLSL program
    glLinkProgram(programID);
    if (checkProgramInfo(programID, GL_LINK_STATUS, nullptr)){
	    // Validate program executability giving current OpenGL states
	    glValidateProgram(programID);
        if (checkProgramInfo(programID, GL_VALIDATE_STATUS, nullptr)){
	        std::cout << "Program " << programID << " linked\n";
        }
    }
}

//GLSL shader creation (of a certain type, vertex shader, fragment shader oe geometry shader)
GLuint createShaderFromString(const char *shaderString, const char *filepath, GLuint shaderType, GLuint shaderID)
{
    if(0 == shaderID){
        shaderID = glCreateShader(shaderType);
    }
	std::string fileName(filepath);
    if (fileName.empty()){
        fileName = "./__defaultGLSL__.glsl";
    }
	std::string shaderSource(shaderString);

    try{
        //include files are processed in a recursive way, no worry!!
        shaderSource = manageIncludes(shaderSource, fileName);

        //Define global macros
        std::map<string, string>::iterator itr;
	    for( itr = shadersMacroMap.begin(); itr != shadersMacroMap.end(); itr++){
            const char *macro = itr->first.c_str();
            const char *value = itr->second.c_str();
		    defineMacro(shaderSource, macro, value);
	    }

	    //Passing shader source code to GL
	    //Source used for "shaderID" shader, there is only "1" source code and 
	    //the string is NULL terminated (no sizes passed)
	    //std::cout << shaderSource;
	    const char *src = shaderSource.c_str();
	    glShaderSource(shaderID, 1, &src, NULL);
	    glCompileShader(shaderID);

        //check GL runtime error
        GLenum error;
        while ((error = glGetError() ) != GL_NO_ERROR){
            const char* format = "GL error at file %s, line %d: %s\n";
            fprintf (stderr, format, __FILE__, __LINE__, gluErrorString(error));
            shaderID = 0;
        }
    }
    catch(...){
        glDeleteShader(shaderID);
        shaderID = 0;
    }

	return shaderID;
}

//GLSL shader creation (of a certain type, vertex shader, fragment shader oe geometry shader)
GLuint createShader(const char *fileName, GLuint shaderType, GLuint shaderID)
{
	GLuint newShaderID = 0;
	std::string shaderSource = loadTextFile(fileName);
	if (!shaderSource.empty()){
		newShaderID = createShaderFromString(shaderSource.c_str(), fileName, shaderType, shaderID);
	}
	return newShaderID;
}

std::string manageIncludes(const std::string& _src, const std::string& _sourceFileName)
{
	std::string src(_src);
	const char include_cstr[] = "#include";
	size_t includepos = src.find(include_cstr, 0);

	std::string res;
	res.reserve(128 * 1024);
	while(includepos != std::string::npos){
		const bool comment = src.substr(includepos-2, 2) == std::string("//");
		if(!comment){			
			size_t fnamestartLoc = src.find("\"", includepos);
			size_t fnameendLoc = src.find("\"", fnamestartLoc+1);
			size_t fnamestartLib = src.find("<", includepos);
			size_t fnameendLib = src.find(">", fnamestartLib+1);
			size_t fnameEndOfLine = src.find("\n", includepos);
			size_t fnamestart;
			size_t fnameend;

			bool uselibpath=false;
			if( (fnamestartLoc == std::string::npos || fnamestartLib < fnamestartLoc) && fnamestartLib < fnameEndOfLine){
				fnamestart=fnamestartLib;
				fnameend=fnameendLib;
				uselibpath=true;
			}
            else if(fnamestartLoc != std::string::npos && fnamestartLoc < fnameEndOfLine){
				fnamestart=fnamestartLoc;
				fnameend=fnameendLoc;
				uselibpath=false;
			}
            else{
                std::cerr << "Invalid #include directive into \"" << _sourceFileName << "\"\n";
				return src;
			}

			std::string incfilename = src.substr(fnamestart+1, fnameend-fnamestart-1);
			std::string incsource;

			if(uselibpath){
				std::string usedPath;
				//TODO: Add paths types into the manager -> search only onto shaders paths.
				std::vector<std::string> pathsList;
				//ResourcesManager::getManager()->getPaths(pathsList);
                pathsList.push_back("./");

				for(std::vector<std::string>::iterator it= pathsList.begin(); it!= pathsList.end(); it++){
					std::string fullpathtmp=(*it) + incfilename;
					FILE *file = fopen(fullpathtmp.c_str(), "r");
					if (file != nullptr){
						usedPath = *it;
						fclose(file);
						break;
					}
                    else{
						usedPath="";
					}
				}
				
				if(usedPath != ""){
					incsource=loadTextFile( (usedPath + incfilename ).c_str() );
				}
                else{
                    std::cerr <<"Unable to find included file \"" << incfilename <<"\" in paths.\n";
					return src;
				}
			}
            else{
                const int pos = _sourceFileName.find_last_of("/", _sourceFileName.size());
                const string fname = _sourceFileName.substr(0, pos+1 ) + incfilename;
				incsource = loadTextFile(fname.c_str());
			}

			incsource = manageIncludes(incsource, _sourceFileName);
			std::string preIncludePart = src.substr(0, includepos);
			std::string postIncludePart = src.substr(fnameend+1, src.size()- fnameend);
			int numline=0;
			size_t newlinepos=0;
			do{
				newlinepos=preIncludePart.find("\n", newlinepos+1);
				numline++;
			}
            while(newlinepos!=std::string::npos);
			numline--;
			
			char outbuff[512];
			snprintf(outbuff, sizeof(outbuff), "\n#line 0\n");
			std::string linePragmaPre(outbuff);
			snprintf(outbuff, sizeof(outbuff), "\n#line %d\n", numline);
			std::string linePragmaPost(outbuff);
			res = preIncludePart+ linePragmaPre+incsource + linePragmaPost + postIncludePart;
			src = res;
		}

		includepos = src.find(include_cstr, includepos + 1);
	}

	return src;
}

}//namespace