#pragma once
#include "ParentModel.h"

class PLAYER : public ParentModel
{
public:
	glm::vec3 velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
	float radius_ = 0.5f;  // 구체 반지름

	Shape getShape() const override {		 // 모양 게터
		return Shape::SPHERE;
	}

	float getRadius() const override {		 // 반지름 게터
		return radius_;
	}

	float getSpeed() const {				 // 속도 게터
		return speed_;
	}

	float getScaleFactor() const {			 // 스케일 팩터 게터
		return scaleFactor_;
	}

	void setVelocity(const glm::vec3& vel) { // 속도 세터
		velocity_ = vel;
	}

	// 화살표 모드 상태
	bool isInArrowMode() const { return isFlying_; }
	void exitArrowMode() { isFlying_ = false; }	// WASD 입력 시 호출

	void update(float deltaTime);
	void move(const glm::vec3& forward, const glm::vec3& right, int direction, float speed);
	void Deceleration(float deltaTime);  // 입력 없을 때 감속

	// 충돌 처리
	void onCollision(ParentModel* other) override;

	CollisionType getCollisionType() const override {
		return CollisionType::NONE;
	}

private:
	float speed_ = 1.0f;
	float scaleFactor_ = 0.5f;						// 구체 크기 조절
	float maxSpeed_ = 5.0f;							// 최대 속도
	float acceleration_ = 6.0f;						// 가속도
	float deceleration_ = 6.0f;						// 감속도
	// glm::vec3 inputDirection_ = glm::vec3(0.0f);	// 현재 입력 방향

	bool isFlying_ = false;		// 화살표 모드 상태

	void handleNormalBlockCollision(ParentModel* block);
	void handleBounceBlockCollision(ParentModel* block);
	void handleBreakableBlockCollision(ParentModel* block);
	void handleSpikeBlockCollision(ParentModel* block);
	void handleArrowBlockCollision(ParentModel* block);
	void handleStarCollision(ParentModel* star);
};