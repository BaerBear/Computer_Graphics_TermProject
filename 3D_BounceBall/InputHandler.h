#pragma once
#include "Headers.h"
#include "GameWorld.h"
#include "Camera.h"

class InputHandler
{
public:
	InputHandler();
	~InputHandler();

	void setGameWorld(GameWorld* world) { gameWorld_ = world; }
	void setCamera(Camera* cam) { camera_ = cam; }
	
	void setRenderingSettings(bool* depthTest, bool* cullFace, bool* projPerspective,
		                      bool* drawSolid, bool* drawWireframe);
	void setLightingSettings(bool* turnLight, float* intensity);
	void setPlayerMoveSpeed(float speed) { playerMoveSpeed_ = speed; }

	void handleKeyboard(unsigned char key, int x, int y);

private:
	GameWorld* gameWorld_;
	Camera* camera_;

	bool* depthTest_;
	bool* cullFace_;
	bool* projectionPerspective_;
	bool* drawSolid_;
	bool* drawWireframe_;

	bool* turnOnLight_;
	float* lightIntensity_;

	float playerMoveSpeed_;

	void handleRenderingKeys(unsigned char key);
	void handleLightingKeys(unsigned char key);
	void handleGameKeys(unsigned char key);
	void handlePlayerMovement(unsigned char key);
};