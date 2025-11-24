#pragma once
#include "ParentModel.h"

// 기본 블럭 클래스
class BLOCK : public ParentModel
{
public:
	CollisionType getCollisionType() const override;
	void onCollision(ParentModel* other) override;
};

// 튕기는 블럭
class BOUNCE_BLOCK : public BLOCK
{
public:
	CollisionType getCollisionType() const override;
};

// 부서지는 블럭
class BREAKABLE_BLOCK : public BLOCK
{
public:
	bool isBroken_ = false;

	CollisionType getCollisionType() const override;
	void onCollision(ParentModel* other) override;
};

// 가시 블럭
class SPIKE_BLOCK : public BLOCK
{
public:
	CollisionType getCollisionType() const override;
};

