#include "OutputHandler.h"
#include <iostream>

OutputHandler::OutputHandler()
	: gameWorld_(nullptr)
	, camera_(nullptr)
	, playerMoveSpeed_(0.2f)
{
}

OutputHandler::~OutputHandler()
{
}


void OutputHandler::handleKeyboard(unsigned char key, int x, int y)
{
	// GameWorld가 없으면 렌더링 관련 키만 처리
	if (!gameWorld_ && (key != 'w' && key != 'W' && key != 'a' && key != 'A' &&
		key != 's' && key != 'S' && key != 'd' && key != 'D')) {
		return;
	}

	handlePlayerMovement(key);

	if (key == 'q' || key == 'Q') {
		exit(0);
	}
}

void OutputHandler::handlePlayerMovement(unsigned char key)
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
		player->setVelocity(glm::vec3(0.0f, player->velocity_.y, 0.0f));
		break;
	case 'a':
	case 'A':
		player->setVelocity(glm::vec3(0.0f, player->velocity_.y, 0.0f));
		break;
	case 's':
	case 'S':
		player->setVelocity(glm::vec3(0.0f, player->velocity_.y, 0.0f));
		break;
	case 'd':
	case 'D':
		player->setVelocity(glm::vec3(0.0f, player->velocity_.y, 0.0f));
		break;
	}
}