#include "Camera.h"
#include <algorithm>

namespace Terrain {

Camera::Camera() {
    UpdateCameraVectors();
}

void Camera::Orbit(float32 deltaYaw, float32 deltaPitch) {
    m_Yaw += deltaYaw * m_OrbitSpeed;
    m_Pitch += deltaPitch * m_OrbitSpeed;

    // Clamp pitch
    m_Pitch = glm::clamp(m_Pitch, m_MinPitch, m_MaxPitch);

    UpdateCameraVectors();
}

void Camera::Pan(float32 deltaX, float32 deltaY) {
    m_Target += m_Right * deltaX * m_PanSpeed;
    m_Target += m_Up * deltaY * m_PanSpeed;

    UpdateCameraVectors();
}

void Camera::Zoom(float32 delta) {
    m_Distance -= delta * m_ZoomSpeed;
    m_Distance = glm::clamp(m_Distance, m_MinDistance, m_MaxDistance);

    UpdateCameraVectors();
}

void Camera::ResetToHome() {
    m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Distance = 500.0f;
    m_Yaw = 45.0f;
    m_Pitch = 30.0f;

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() {
    // Calculate position based on spherical coordinates
    float32 yawRad = glm::radians(m_Yaw);
    float32 pitchRad = glm::radians(m_Pitch);

    m_Position.x = m_Target.x + m_Distance * cos(pitchRad) * cos(yawRad);
    m_Position.y = m_Target.y + m_Distance * sin(pitchRad);
    m_Position.z = m_Target.z + m_Distance * cos(pitchRad) * sin(yawRad);

    // Calculate camera vectors
    m_Forward = glm::normalize(m_Target - m_Position);
    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Target, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix(float32 aspectRatio) const {
    return glm::perspective(glm::radians(m_FOV), aspectRatio, 0.1f, 10000.0f);
}

glm::vec3 Camera::GetPosition() const {
    return m_Position;
}

glm::vec3 Camera::GetForward() const {
    return m_Forward;
}

glm::vec3 Camera::GetRight() const {
    return m_Right;
}

glm::vec3 Camera::GetUp() const {
    return m_Up;
}

} // namespace Terrain
