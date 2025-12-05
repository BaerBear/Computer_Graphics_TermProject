#include "Player.h"

void PLAYER::update(float deltaTime) {
	translation_.x += velocity_.x * deltaTime;
	translation_.y += velocity_.y * deltaTime;
	translation_.z += velocity_.z * deltaTime;
	velocity_.y -= 9.8f * deltaTime;  // 중력
}

void PLAYER::move(const glm::vec3& forward, const glm::vec3& right, int direction, float speed) {
	glm::vec3 movement(0.0f);
	// 카메라 정면 방향 = 정면

	switch (direction) {
	case 0: // 전방 (W)
		movement = forward * speed;
		break;
	case 1: // 좌측 (A)
		movement = -right * speed;
		break;
	case 2: // 후방 (S)
		movement = -forward * speed;
		break;
	case 3: // 우측 (D)
		movement = right * speed;
		break;
	}

	// 수평 이동
	glm::vec3 Velocity = glm::vec3(velocity_.x, 0.0f, velocity_.z) + glm::vec3(movement.x, 0.0f, movement.z);

	// 최대 속도 제한
	float horizontalSpeed = glm::length(glm::vec2(Velocity.x, Velocity.z));
	if (horizontalSpeed > maxSpeed_) {
		glm::vec2 normalized = glm::normalize(glm::vec2(Velocity.x, Velocity.z));
		Velocity.x = normalized.x * maxSpeed_;
		Velocity.z = normalized.y * maxSpeed_;
	}

	// 가속도 적용
	velocity_.x = glm::mix(velocity_.x, Velocity.x, acceleration_ * 0.016f);
	velocity_.z = glm::mix(velocity_.z, Velocity.z, acceleration_ * 0.016f);
}

void PLAYER::Deceleration(float deltaTime) {
	// 입력이 없을 때 속도 감소
	if( glm::length(glm::vec2(velocity_.x, velocity_.z)) < 0.01f) {
		velocity_.x = 0.0f;
		velocity_.z = 0.0f;
		return;
	}
	float horizontalSpeed = glm::length(glm::vec2(velocity_.x, velocity_.z));
	if (horizontalSpeed > 0.01f) {
		glm::vec2 horizontalVel(velocity_.x, velocity_.z);
		horizontalVel = glm::normalize(horizontalVel) * std::max(0.0f, horizontalSpeed - deceleration_ * deltaTime);
		velocity_.x = horizontalVel.x;
		velocity_.z = horizontalVel.y;
	}
}

void PLAYER::onCollision(ParentModel* other) {
	CollisionType type = other->getCollisionType();

	switch (type) {
	case CollisionType::NORMAL_BLOCK:
		handleNormalBlockCollision(other);
		break;
	case CollisionType::BOUNCE_BLOCK:
		handleBounceBlockCollision(other);
		break;
	case CollisionType::BREAKABLE_BLOCK:
		handleBreakableBlockCollision(other);
		break;
	case CollisionType::SPIKE_BLOCK:
		handleSpikeBlockCollision(other);
		break;
	case CollisionType::STAR:
		handleStarCollision(other);
		break;
	}
}

void PLAYER::handleNormalBlockCollision(ParentModel* block) {
	glm::vec3 blockPos = block->getPosition();
	glm::vec3 blockScale = block->getScale();
	glm::vec3 playerPos = getPosition();

	// AABB의 최소/최대 점
	glm::vec3 blockMin = blockPos - blockScale * 0.5f;
	glm::vec3 blockMax = blockPos + blockScale * 0.5f;

	// 가장 가까운 점 찾기
	glm::vec3 closestPoint = glm::clamp(playerPos, blockMin, blockMax);
	glm::vec3 diff = playerPos - closestPoint;

	// 충돌 법선 방향 계산
	if (glm::length(diff) > 0.001f) {
		glm::vec3 normal = glm::normalize(diff);

		// 법선 방향으로 속도 반사
		if (abs(normal.y) > 0.5f) {
			velocity_.y = -velocity_.y * 0.8f;  // Y축

			// 블럭 위에 올려놓기
			if (normal.y > 0) {
				translation_.y = blockMax.y + radius_;
			}
		}
		else if (abs(normal.x) > abs(normal.z)) {
			velocity_.x = -velocity_.x * 0.5f;  // X축
		}
		else {
			velocity_.z = -velocity_.z * 0.5f;  // Z축
		}
	}
}

void PLAYER::handleBounceBlockCollision(ParentModel* block) {
	// velocity_.y = abs(velocity_.y) * 1.0f;
	velocity_.y = 10.0f; // 고정된 높이로 점프
}

void PLAYER::handleBreakableBlockCollision(ParentModel* block) {
	handleNormalBlockCollision(block);
	velocity_.y = 10.0f; // 속도 절반으로 감소
}

void PLAYER::handleSpikeBlockCollision(ParentModel* block) {
	reset();
	velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
}

void PLAYER::handleStarCollision(ParentModel* star) {
	// 별 하고 만났을 때 공한테 할 물리적인 처리들 (지금은 없음)
}