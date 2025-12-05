#pragma once
#include "Headers.h"
#include "Vertex.h"
#include <vector>

class TrajectoryPredictor
{
public:
	void init(GLuint shaderProgramID);
	void draw(const glm::vec3& startPos, const glm::vec3& velocity, int numPoints = 50, float timeStep = 0.05f);
	void cleanup();

private:
	GLuint trajectoryVAO_, trajectoryVBO_;
	GLuint circleVAO_, circleVBO_;
	GLuint shaderProgramID_;
	std::vector<VERTEX> trajectoryPoints_;
	std::vector<VERTEX> circleVertices_;

	// 궤적 계산 (중력 적용)
	void calculateTrajectory(const glm::vec3& startPos, const glm::vec3& velocity, int numPoints, float timeStep);

	// 착지 지점 계산
	glm::vec3 calculateLandingPoint(const glm::vec3& startPos, const glm::vec3& velocity, float groundY = -1.0f);

	// 원 그리기 (착지 지점 표시)
	void drawCircle(const glm::vec3& center, float radius, const glm::vec3& color);

	// 원 버텍스 생성
	void createCircleVertices(float radius, int segments = 36);
};