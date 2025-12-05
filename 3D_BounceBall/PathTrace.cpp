#include "PathTrace.h"
#include <cmath>

void TrajectoryPredictor::init(GLuint shaderProgramID) {
	shaderProgramID_ = shaderProgramID;

	// 궤적용 VAO/VBO
	glGenVertexArrays(1, &trajectoryVAO_);
	glGenBuffers(1, &trajectoryVBO_);

	// 원용 VAO/VBO
	glGenVertexArrays(1, &circleVAO_);
	glGenBuffers(1, &circleVBO_);

	// 원 버텍스 생성 (반지름 0.5, 36개 세그먼트)
	createCircleVertices(0.5f, 36);
}

void TrajectoryPredictor::createCircleVertices(float radius, int segments) {
	circleVertices_.clear();

	// 중심점 추가 (TRIANGLE_FAN을 위해 필요)
	circleVertices_.push_back({ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f });

	// 원의 둘레 점들 추가
	for (int i = 0; i <= segments; ++i) {
		float angle = (float)i / (float)segments * 2.0f * 3.14159265f;
		float x = cos(angle) * radius;
		float z = sin(angle) * radius;

		// 노란색 원
		circleVertices_.push_back({ x, 0.0f, z, 1.0f, 1.0f, 0.0f });
	}
}

void TrajectoryPredictor::calculateTrajectory(const glm::vec3& startPos, const glm::vec3& velocity, int numPoints, float timeStep) {
	trajectoryPoints_.clear();

	glm::vec3 pos = startPos;
	glm::vec3 vel = velocity;
	const float gravity = 9.8f;

	for (int i = 0; i < numPoints; ++i) {
		float t = (float)i / (float)numPoints;
		float r = 1.0f;
		float g = 1.0f - t * 0.5f;
		float b = 0.0f;

		trajectoryPoints_.push_back({ pos.x, pos.y, pos.z, r, g, b });

		pos.x += vel.x * timeStep;
		pos.y += vel.y * timeStep;
		pos.z += vel.z * timeStep;
		vel.y -= gravity * timeStep;

		if (pos.y < -10.0f) break;
	}
}

glm::vec3 TrajectoryPredictor::calculateLandingPoint(const glm::vec3& startPos, const glm::vec3& velocity, float groundY) {
	glm::vec3 pos = startPos;
	glm::vec3 vel = velocity;
	const float gravity = 9.8f;
	const float timeStep = 0.01f; // 정밀한 계산을 위해 작은 값
	const int maxSteps = 1000; // 무한 루프 방지

	// 땅(groundY)에 도달할 때까지 시뮬레이션
	for (int i = 0; i < maxSteps; ++i) {
		float nextY = pos.y + vel.y * timeStep;

		// 땅에 도달했는지 체크
		if (nextY <= groundY && pos.y > groundY) {
			// 보간하여 정확한 착지 지점 계산
			float t = (groundY - pos.y) / (nextY - pos.y);
			glm::vec3 landingPoint = pos + vel * timeStep * t;
			landingPoint.y = groundY; // 정확히 땅 위에 놓기
			return landingPoint;
		}

		pos.x += vel.x * timeStep;
		pos.y += vel.y * timeStep;
		pos.z += vel.z * timeStep;
		vel.y -= gravity * timeStep;

		// 너무 아래로 떨어지면 중단
		if (pos.y < -50.0f) break;
	}

	return pos; // 착지하지 못한 경우 마지막 위치 반환
}

void TrajectoryPredictor::drawCircle(const glm::vec3& center, float radius, const glm::vec3& color) {
	// 원 버텍스 업데이트 (색상 적용)
	std::vector<VERTEX> coloredCircle = circleVertices_;
	for (auto& vertex : coloredCircle) {
		vertex.r = color.r;
		vertex.g = color.g;
		vertex.b = color.b;
	}

	glBindBuffer(GL_ARRAY_BUFFER, circleVBO_);
	glBufferData(GL_ARRAY_BUFFER,
		coloredCircle.size() * sizeof(VERTEX),
		coloredCircle.data(),
		GL_DYNAMIC_DRAW);

	glBindVertexArray(circleVAO_);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO_);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// 원의 위치로 이동하는 모델 행렬
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, center);
	model = glm::scale(model, glm::vec3(radius, 1.0f, radius));

	glUseProgram(shaderProgramID_);
	GLint modelLoc = glGetUniformLocation(shaderProgramID_, "modelMat");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// 채워진 원 그리기 (TRIANGLE_FAN 사용)
	glDrawArrays(GL_TRIANGLE_FAN, 0, coloredCircle.size());

	glBindVertexArray(0);
}

void TrajectoryPredictor::draw(const glm::vec3& startPos, const glm::vec3& velocity, int numPoints, float timeStep) {
	// 속도가 거의 없으면 그리지 않음
	if (glm::length(velocity) < 0.1f) return;

	// 1. 궤적 선 그리기
	calculateTrajectory(startPos, velocity, numPoints, timeStep);

	if (!trajectoryPoints_.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO_);
		glBufferData(GL_ARRAY_BUFFER,
			trajectoryPoints_.size() * sizeof(VERTEX),
			trajectoryPoints_.data(),
			GL_DYNAMIC_DRAW);

		glBindVertexArray(trajectoryVAO_);
		glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO_);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glUseProgram(shaderProgramID_);
		GLint modelLoc = glGetUniformLocation(shaderProgramID_, "modelMat");
		glm::mat4 identityMatrix = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identityMatrix));

		glLineWidth(2.0f);
		glDrawArrays(GL_LINE_STRIP, 0, trajectoryPoints_.size());
		glLineWidth(1.0f);

		glBindVertexArray(0);
	}

	// 2. 착지 지점에 노란색 원 그리기
	glm::vec3 landingPoint = calculateLandingPoint(startPos, velocity, -0.8f); // 블록 위 높이
	drawCircle(landingPoint, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f)); // 노란색, 반지름 1.0
}

void TrajectoryPredictor::cleanup() {
	glDeleteVertexArrays(1, &trajectoryVAO_);
	glDeleteBuffers(1, &trajectoryVBO_);
	glDeleteVertexArrays(1, &circleVAO_);
	glDeleteBuffers(1, &circleVBO_);
}