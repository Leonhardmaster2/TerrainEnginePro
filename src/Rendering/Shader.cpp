#include "Shader.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace Terrain {

Shader::Shader() {
}

Shader::~Shader() {
    if (m_ID) {
        glDeleteProgram(m_ID);
    }
    if (m_VertexShader) {
        glDeleteShader(m_VertexShader);
    }
    if (m_FragmentShader) {
        glDeleteShader(m_FragmentShader);
    }
}

String Shader::LoadFile(const String& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open shader file: %s", filepath.c_str());
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Shader::CompileShader(uint32 shader, const String& source, const String& type) {
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOG_ERROR("Shader compilation failed (%s): %s", type.c_str(), infoLog);
        return false;
    }

    return true;
}

bool Shader::LoadFromFiles(const String& vertexPath, const String& fragmentPath) {
    // Load source files
    String vertexSource = LoadFile(vertexPath);
    String fragmentSource = LoadFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }

    // Create shaders
    m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
    m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile shaders
    if (!CompileShader(m_VertexShader, vertexSource, "VERTEX")) {
        return false;
    }

    if (!CompileShader(m_FragmentShader, fragmentSource, "FRAGMENT")) {
        return false;
    }

    // Link program
    if (!LinkProgram()) {
        return false;
    }

    LOG_INFO("Shader program loaded successfully");
    return true;
}

bool Shader::LinkProgram() {
    m_ID = glCreateProgram();
    glAttachShader(m_ID, m_VertexShader);
    glAttachShader(m_ID, m_FragmentShader);
    glLinkProgram(m_ID);

    int32 success;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed: %s", infoLog);
        return false;
    }

    // Can delete shaders after linking
    glDeleteShader(m_VertexShader);
    glDeleteShader(m_FragmentShader);
    m_VertexShader = 0;
    m_FragmentShader = 0;

    return true;
}

void Shader::Use() const {
    glUseProgram(m_ID);
}

void Shader::SetBool(const String& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int32)value);
}

void Shader::SetInt(const String& name, int32 value) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetFloat(const String& name, float32 value) const {
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetVec3(const String& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::SetMat4(const String& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

} // namespace Terrain
