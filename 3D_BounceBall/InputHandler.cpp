#include "InputHandler.h"
#include <iostream>

InputHandler::InputHandler()
	: gameWorld_(nullptr)
	, camera_(nullptr)
	, depthTest_(nullptr)
	, cullFace_(nullptr)
	, projectionPerspective_(nullptr)
	, drawSolid_(nullptr)
	, drawWireframe_(nullptr)
	, turnOnLight_(nullptr)
	, lightIntensity_(nullptr)
	, playerMoveSpeed_(0.2f)
{
}

InputHandler::~InputHandler()
{
}

void InputHandler::setRenderingSettings(bool* depthTest, bool* cullFace, bool* projPerspective, bool* drawSolid, bool* drawWireframe)
{
	this->depthTest_ = depthTest;
	this->cullFace_ = cullFace;
	this->projectionPerspective_ = projPerspective;
	this->drawSolid_ = drawSolid;
	this->drawWireframe_ = drawWireframe;
}

void InputHandler::setLightingSettings(bool* turnLight, float* intensity)
{
	this->turnOnLight_ = turnLight;
	this->lightIntensity_ = intensity;
}

void InputHandler::handleKeyboard(unsigned char key, int x, int y)
{
	// GameWorld가 없으면 렌더링 관련 키만 처리
	if (!gameWorld_ && (key != 'h' && key != 'H' && key != 'u' && key != 'U' &&
		key != 'p' && key != 'P' && key != 'm' && key != 'M' &&
		key != 'r' && key != 'R' && key != '+' && key != '-' &&
		key != 'q' && key != 'Q')) {
		return;
	}

	// 키 타입별로 처리
	handleRenderingKeys(key);
	handleLightingKeys(key);
	handleGameKeys(key);
	handlePlayerMovement(key);

	// 종료 키
	if (key == 'q' || key == 'Q') {
		std::cout << "Exiting game..." << std::endl;
		exit(0);
	}
}

void InputHandler::handleRenderingKeys(unsigned char key)
{
	if (!depthTest_ || !cullFace_ || !projectionPerspective_ || !drawSolid_ || !drawWireframe_)
		return;

	switch (key)
	{
	case 'h':
	case 'H':
		*depthTest_ = !(*depthTest_);
		std::cout << "Depth Test: " << (*depthTest_ ? "Enabled" : "Disabled") << std::endl;
		break;

	case 'u':
	case 'U':
		*cullFace_ = !(*cullFace_);
		std::cout << "Cull Face: " << (*cullFace_ ? "Enabled" : "Disabled") << std::endl;
		break;

	case 'p':
		*projectionPerspective_ = false;
		std::cout << "Orthographic Projection" << std::endl;
		break;

	case 'P':
		*projectionPerspective_ = true;
		std::cout << "Perspective Projection" << std::endl;
		break;

	case 'm':
		*drawSolid_ = true;
		*drawWireframe_ = false;
		std::cout << "Draw Solid Mode" << std::endl;
		break;

	case 'M':
		*drawSolid_ = false;
		*drawWireframe_ = true;
		std::cout << "Draw Wireframe Mode" << std::endl;
		break;
	}
}

void InputHandler::handleLightingKeys(unsigned char key)
{
	if (!turnOnLight_ || !lightIntensity_)
		return;

	switch (key)
	{
	case 'r':
	case 'R':
		*turnOnLight_ = !(*turnOnLight_);
		std::cout << "Lighting: " << (*turnOnLight_ ? "ON" : "OFF") << std::endl;
		break;

	case '+':
		*lightIntensity_ += 0.1f;
		if (*lightIntensity_ > 1.0f) *lightIntensity_ = 1.0f;
		std::cout << "Light Intensity: " << *lightIntensity_ << std::endl;
		break;

	case '-':
		*lightIntensity_ -= 0.1f;
		if (*lightIntensity_ < 0.0f) *lightIntensity_ = 0.0f;
		std::cout << "Light Intensity: " << *lightIntensity_ << std::endl;
		break;
	}
}

void InputHandler::handleGameKeys(unsigned char key)
{
	if (!gameWorld_)
		return;

	switch (key)
	{
	case 'c':
	case 'C':
		gameWorld_->setGameStarted(!gameWorld_->isGameStarted());
		std::cout << "Game " << (gameWorld_->isGameStarted() ? "Started" : "Paused") << std::endl;
		break;

	case 'x':
	case 'X':
		gameWorld_->reset();
		std::cout << "Game Reset! Score: " << gameWorld_->getScore() << std::endl;
		break;
	}
}

void InputHandler::handlePlayerMovement(unsigned char key)
{
	if (!gameWorld_)
		return;

	PLAYER* player = gameWorld_->getPlayer();
	if (!player)
		return;

	switch (key)
	{
	case 'w':
	case 'W':
		player->move(camera_->getForward(), camera_->getRight(), 0, playerMoveSpeed_);
		std::cout << "Move Forward" << std::endl;
		break;

	case 'a':
	case 'A':
		player->move(camera_->getForward(), camera_->getRight(), 1, playerMoveSpeed_);
		std::cout << "Move Left" << std::endl;
		break;
	case 's':
	case 'S':
		player->move(camera_->getForward(), camera_->getRight(), 2, playerMoveSpeed_);
		std::cout << "Move Forward" << std::endl;
		break;

	case 'd':
	case 'D':
		player->move(camera_->getForward(), camera_->getRight(), 3, playerMoveSpeed_);
		std::cout << "Move Right" << std::endl;
		break;
	}
}