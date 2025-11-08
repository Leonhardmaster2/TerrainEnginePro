#pragma once

#include "Core/Types.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Terrain {

class Camera {
public:
    Camera();

    // Camera controls
    void Orbit(float32 deltaYaw, float32 deltaPitch);
    void Pan(float32 deltaX, float32 deltaY);
    void Zoom(float32 delta);
    void ResetToHome();

    // Getters
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float32 aspectRatio) const;
    glm::vec3 GetPosition() const;
    glm::vec3 GetTarget() const { return m_Target; }
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

    // Setters
    void SetTarget(const glm::vec3& target) { m_Target = target; }
    void SetDistance(float32 distance) { m_Distance = glm::clamp(distance, m_MinDistance, m_MaxDistance); }
    void SetFOV(float32 fov) { m_FOV = fov; }
    void SetYaw(float32 yaw) { m_Yaw = yaw; UpdateCameraVectors(); }
    void SetPitch(float32 pitch) { m_Pitch = glm::clamp(pitch, m_MinPitch, m_MaxPitch); UpdateCameraVectors(); }

    // Parameters
    float32 GetDistance() const { return m_Distance; }
    float32 GetYaw() const { return m_Yaw; }
    float32 GetPitch() const { return m_Pitch; }
    float32 GetFOV() const { return m_FOV; }

private:
    void UpdateCameraVectors();

    // Camera parameters
    glm::vec3 m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
    float32 m_Distance = 500.0f;
    float32 m_Yaw = 45.0f;
    float32 m_Pitch = 30.0f;
    float32 m_FOV = 45.0f;

    // Constraints
    float32 m_MinDistance = 50.0f;
    float32 m_MaxDistance = 5000.0f;
    float32 m_MinPitch = -89.0f;
    float32 m_MaxPitch = 89.0f;

    // Cached vectors
    glm::vec3 m_Position;
    glm::vec3 m_Forward;
    glm::vec3 m_Right;
    glm::vec3 m_Up;
    glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Movement speeds
    float32 m_OrbitSpeed = 0.5f;
    float32 m_PanSpeed = 1.0f;
    float32 m_ZoomSpeed = 10.0f;
};

} // namespace Terrain
