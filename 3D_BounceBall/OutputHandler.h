#pragma once
#include "Headers.h"
#include "GameWorld.h"
#include "Camera.h"

class OutputHandler
{
public:
	OutputHandler();
	~OutputHandler();

	void setGameWorld(GameWorld* world) { gameWorld_ = world; }
	void setCamera(Camera* cam) { camera_ = cam; }
	void setPlayerMoveSpeed(float speed) { playerMoveSpeed_ = speed; }

	void handleKeyboard(unsigned char key, int x, int y);

private:
	GameWorld* gameWorld_;
	Camera* camera_;
	float playerMoveSpeed_;

	void handleRenderingKeys(unsigned char key);
	void handleLightingKeys(unsigned char key);
	void handleGameKeys(unsigned char key);
	void handlePlayerMovement(unsigned char key);
};