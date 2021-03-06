//===========================================================
// File: ShaderProgram.h	
// StudentName: Per-Morten Straume                          
//                                                          
// Exam 2015: IMT-2531 Graphics Programming Exam.                                
//===========================================================
#pragma once
#include <string>
#include <unordered_map>

#include <GL\glew.h>
#include <glm\glm.hpp>

class ShaderProgram
{
public:
    using ProgramHandle = GLuint;
    using UniformAddress = GLuint;
    using UniformName = std::string;

    ShaderProgram() = delete;
    ShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) noexcept;
    ~ShaderProgram() noexcept;

    void bind() noexcept;

    UniformAddress getUniformAddress(const UniformName& uniformName) noexcept; 

    static const UniformName color;
    static const UniformName viewMatrix;
    static const UniformName modelMatrix;
    static const UniformName normalMatrix;
    static const UniformName ambientFactor;
    static const UniformName lightDirection;
    static const UniformName textureOffset;
    static const UniformName worldScale;
    static const UniformName projectionMatrix;
    static const UniformName numberOfRows;

private:
    ProgramHandle _programHandle{};
    
    std::unordered_map<UniformName, UniformAddress> _uniformAddresses{};

};

