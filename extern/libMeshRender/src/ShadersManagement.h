#pragma once
#ifndef SHADERSMANAGMENT_H
#define SHADERSMANAGMENT_H

#include <string.h>
#include <string>
#include <GL/glew.h>

namespace MeshRender{

GLuint createShader(const char *fileName, GLuint shaderType, GLuint shaderID=0);

GLuint createShaderFromString(const char *shaderString, const char *filepath, GLuint shaderType, GLuint shaderID=0);

GLuint createShaderProgram(const char *fileNameVS, const char *fileNameGS, const char *fileNameFS, GLuint programID=0);

void linkShaderProgram(GLuint programID);

void setShadersGlobalMacro(const char *macro, int val);

void setShadersGlobalMacro(const char *macro, float val);

void setShadersGlobalMacro(const char *macro, const char* val);

void resetShadersGlobalMacros();

}//namespace

#endif
