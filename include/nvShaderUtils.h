
#ifndef NV_SHADER_UTILS_H
#define NV_SHADER_UTILS_H

#include <stdio.h>
#include <string.h>
#include <GL/glew.h>

namespace nv
{

//perform simple macro replacement, where the two macros must have the same length
inline void replaceMacro(char *text, const char *originMacro, const char *newMacro)
{
    if (originMacro!=NULL && newMacro!=NULL){
        int n0 = strlen(originMacro);
        int n1 = strlen(newMacro);
        if (n1!=0 && n0==n1){
            char *p = text;
            const char *src = newMacro;
            while (*p){
                char *q = strstr(p, originMacro);
                if (q){
                    for (int i=0; i<n0; i++)
                        q[i]=src[i];
                    p= q + n0;
                }
                else{
                    break;
                }
            }
        }
    }
}

//
//
////////////////////////////////////////////////////////////
inline GLuint CompileGLSLShader(GLenum target, const char* shader)
{
    GLuint object;

    object = glCreateShader( target);

    if (!object)
        return object;

    glShaderSource( object, 1, &shader, NULL);

    glCompileShader(object);

    // check if shader compiled
    GLint compiled = 0;
    glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
#ifdef NV_REPORT_COMPILE_ERRORS
        char temp[256] = "";
        glGetShaderInfoLog( object, 256, NULL, temp);
        fprintf(stderr, "Compile failed:\n%s\n", temp);
#endif
        glDeleteShader( object);
        return 0;
    }

    return object;
}

inline GLuint CompileGLSLShader(
    GLenum target, const char* shader, const char *originMacros, const char *newMacro)
{
    if (originMacros!=NULL && newMacro!=NULL){
        const int n = strlen(shader);
        char* strProgram = NULL;
        try{
            strProgram = new char [n+1];
            for (int i=0; i<n; i++) strProgram[i]=shader[i];
            strProgram[n]=0;
        }
        catch(...){
            fprintf(stderr, "Compile failed when allocating space.\n");
        }
        replaceMacro(strProgram, originMacros, newMacro);
        GLuint id = CompileGLSLShader(target, strProgram);
        delete [] strProgram;
        return id;
    }
    else
        return CompileGLSLShader(target, shader);
}

//
//
////////////////////////////////////////////////////////////
inline GLuint CompileGLSLShaderFromFile(
    GLenum target, const char* filename, const char *originMacros=NULL, const char *newMacro=NULL)
{
    FILE *shaderFile;
    char *text;
    long size;

    //must read files as binary to prevent problems from newline translation
    shaderFile = fopen( filename, "rb");

    if ( shaderFile == NULL)
        return 0;

    fseek( shaderFile, 0, SEEK_END);
    size = ftell(shaderFile);
    fseek( shaderFile, 0, SEEK_SET);
    text = new char[size+1];
    fread( text, size, 1, shaderFile);
    fclose( shaderFile);
    text[size] = '\0';

    //perform macro replacement
    if (originMacros && newMacro)
		replaceMacro(text, originMacros, newMacro);
    GLuint object = CompileGLSLShader( target, text);
    delete []text;
    return object;
}


// Create a program composed of vertex and fragment shaders.
inline GLuint LinkGLSLProgram( GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

#ifdef NV_REPORT_COMPILE_ERRORS
    // Get error log.
    GLint charsWritten, infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    char * infoLog = new char[infoLogLength];
    glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
    printf(infoLog);
    delete [] infoLog;
#endif

    // Test linker result.
    GLint linkSucceed = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSucceed);
    if (linkSucceed == GL_FALSE){
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

// Create a program composed of vertex, geometry and fragment shaders.
inline GLuint LinkGLSLProgram( GLuint vertexShader, GLuint geometryShader, GLint inputType, GLint vertexOut, GLint outputType, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, geometryShader);
	glProgramParameteriEXT(program, GL_GEOMETRY_INPUT_TYPE_EXT, inputType);
    glProgramParameteriEXT(program, GL_GEOMETRY_VERTICES_OUT_EXT, vertexOut);
    glProgramParameteriEXT(program, GL_GEOMETRY_OUTPUT_TYPE_EXT, outputType);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

#ifdef NV_REPORT_COMPILE_ERRORS
    // Get error log.
    GLint charsWritten, infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    char * infoLog = new char[infoLogLength];
    glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
    printf(infoLog);
    delete [] infoLog;
#endif

    // Test linker result.
    GLint linkSucceed = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSucceed);
    
    if (linkSucceed == GL_FALSE)
    {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}


//
//
////////////////////////////////////////////////////////////
inline GLuint CompileASMShader(GLenum program_type, const char *code)
{
    GLuint program_id;
    glGenProgramsARB(1, &program_id);
    glBindProgramARB(program_type, program_id);
    glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) strlen(code), (GLubyte *) code);

    GLint error_pos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
    if (error_pos != -1) {
#ifdef NV_REPORT_COMPILE_ERRORS
        const GLubyte *error_string;
        error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        fprintf(stderr, "Program error at position: %d\n%s\n", (int)error_pos, error_string);
#endif
        return 0;
    }
    return program_id;
}

//
//
////////////////////////////////////////////////////////////
inline GLuint CompileASMShaderFromFile( GLenum target, const char* filename)
{
    FILE *shaderFile;
    char *text;
    long size;

    //must read files as binary to prevent problems from newline translation
    shaderFile = fopen( filename, "rb");

    if ( shaderFile == NULL)
        return 0;

    fseek( shaderFile, 0, SEEK_END);

    size = ftell(shaderFile);

    fseek( shaderFile, 0, SEEK_SET);

    text = new char[size+1];

    fread( text, size, 1, shaderFile);

    fclose( shaderFile);

    text[size] = '\0';

    GLuint program_id = CompileASMShader( target, text);

    delete []text;

    return program_id;
}

} // nv namespace

#endif 