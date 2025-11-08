#pragma once

#include "Core/Types.h"
#include <glm/glm.hpp>
#include <string>

namespace Terrain {

class Shader {
public:
    Shader();
    ~Shader();

    bool LoadFromFiles(const String& vertexPath, const String& fragmentPath);
    void Use() const;

    // Uniform setters
    void SetBool(const String& name, bool value) const;
    void SetInt(const String& name, int32 value) const;
    void SetFloat(const String& name, float32 value) const;
    void SetVec3(const String& name, const glm::vec3& value) const;
    void SetMat4(const String& name, const glm::mat4& value) const;

    uint32 GetID() const { return m_ID; }

private:
    bool CompileShader(uint32 shader, const String& source, const String& type);
    bool LinkProgram();
    String LoadFile(const String& filepath);

    uint32 m_ID = 0;
    uint32 m_VertexShader = 0;
    uint32 m_FragmentShader = 0;
};

} // namespace Terrain
