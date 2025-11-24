#include "Camera.h"

Camera::Camera()
	: position_(7.0f, 7.0f, 7.0f)
	, target_(0.0f, 0.0f, 0.0f)
	, yaw_(glm::radians(45.0f))
	, pitch_(glm::radians(45.0f))
{
}

Camera::~Camera()
{
}

glm::vec3 Camera::getForward() const
{
	glm::vec3 forward;
	forward.x = cos(yaw_);
	forward.y = 0.0f;
	forward.z = sin(yaw_);
	return glm::normalize(forward);
}

glm::vec3 Camera::getRight() const
{
	glm::vec3 forward = getForward();
	return glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 Camera::getUp() const
{
	return glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position_, target_, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::orbitAroundTarget(float distance, float yawAngle, float pitchAngle)
{
	yaw_ = yawAngle;
	pitch_ = glm::clamp(pitchAngle, glm::radians(-89.0f), glm::radians(89.0f));

	// 구면 좌표계를 이용한 카메라 위치 계산
	position_.x = target_.x + distance * cos(pitch_) * cos(yaw_);
	position_.y = target_.y + distance * sin(pitch_);
	position_.z = target_.z + distance * cos(pitch_) * sin(yaw_);
}