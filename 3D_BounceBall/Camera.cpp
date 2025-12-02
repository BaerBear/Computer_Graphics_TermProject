#include "Camera.h"

Camera::Camera()
	: position_(0.0f, 5.0f, 5.0f)
	, target_(0.0f, 0.0f, 0.0f)
	, yaw_(glm::radians(-90.0f))
	, pitch_(glm::radians(0.0f))
	, roll_(glm::radians(45.0f))
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
	glm::vec3 eye;
	glm::vec3 target;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	// 카메라 방향 벡터 계산
	glm::vec3 cameraDirection;
	cameraDirection.x = cos(pitch_) * cos(yaw_);
	cameraDirection.y = sin(pitch_);
	cameraDirection.z = cos(pitch_) * sin(yaw_);
	cameraDirection = glm::normalize(cameraDirection);

	target = target_ + glm::vec3(0.0f, playerScale_, 0.0f);
	eye = target - cameraDirection * distance_ + glm::vec3(0.0f, 3.0f, 0.0f);
	return glm::lookAt(eye, target, up);
}

void Camera::orbitAroundTarget(float distance, float yawAngle, float pitchAngle)
{
	yaw_ = yawAngle;
	pitch_ = glm::clamp(pitchAngle, glm::radians(-maxPitch_), glm::radians(maxPitch_));
	distance_ = std::min(5.0f, distance);

	// 구면 좌표계를 이용한 카메라 위치 계산
	/*position_.x = target_.x + distance * cos(pitch_) * cos(yaw_);
	position_.y = target_.y + distance * sin(pitch_);
	position_.z = target_.z + distance * cos(pitch_) * sin(yaw_);*/
}