#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

string fileContentToString(string pathToFile)
{
    std::ifstream inputFile;
    inputFile.open(pathToFile);
    if (!inputFile)
    {
        cerr << "Could not open file " << pathToFile << endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    return buffer.str();
}

GLuint createShader(string sourcePath, GLuint type) {
    string source = fileContentToString(sourcePath);
    const char *sourceCStr = source.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourceCStr, NULL);
    glCompileShader(shader);
    // Error check
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
        for (auto const &value : infoLog)
        {
            cout << value;
        }
    }

    return shader;
}

GLuint createShaderProgram(string vertexSourcePath, string tessellationControlSourcePath, string tesselationEvaluationSourcePath, string fragmentSourcePath)
{
    GLuint vertexShader = createShader(vertexSourcePath, GL_VERTEX_SHADER);
    GLuint tessellationControlShader = createShader(tessellationControlSourcePath, GL_TESS_CONTROL_SHADER);
    GLuint tesselationEvaluationShader = createShader(tesselationEvaluationSourcePath, GL_TESS_EVALUATION_SHADER);
    GLuint fragmentShader = createShader(fragmentSourcePath, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, tessellationControlShader);
    glAttachShader(shaderProgram, tesselationEvaluationShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Error check
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
        for (auto const &value : infoLog)
        {
            cout << value;
        }
    }

    return shaderProgram;
}

GLuint createShaderProgram(string vertexSourcePath, string fragmentSourcePath)
{
    GLuint vertexShader = createShader(vertexSourcePath, GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader(fragmentSourcePath, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Error check
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
        for (auto const &value : infoLog)
        {
            cout << value;
        }
    }

    return shaderProgram;
}