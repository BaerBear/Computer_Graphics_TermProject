#include "GameWorld.h"
#include <iostream>

GameWorld::GameWorld(GLuint shaderID)
	: shaderProgramID_(shaderID)
	, gameStarted_(true)
	, score_(0)
{
}

GameWorld::~GameWorld()
{
	cleanup();
}

void GameWorld::initialize()
{
	std::cout << "Initializing GameWorld..." << std::endl;

	// 플레이어 초기화
	player_.init("obj/sphere.obj", shaderProgramID_);
	player_.setTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
	player_.radius_ = 0.5f;

	// 블럭들 생성
	createFloorBlocks();
	createBounceBlocks();
	createBreakableBlocks();
	createSpikeBlocks();

	std::cout << "** GameWorld initialized **" << std::endl;
	std::cout << "Normal Blocks: " << blocks_.size() << std::endl;
	std::cout << "Bounce Blocks: " << bounceBlocks_.size() << std::endl;
	std::cout << "Breakable Blocks: " << breakableBlocks_.size() << std::endl;
	std::cout << "Spike Blocks: " << spikeBlocks_.size() << std::endl;
}

void GameWorld::cleanup()
{
	// 메모리 해제
	for (auto block : blocks_) delete block;
	for (auto block : bounceBlocks_) delete block;
	for (auto block : breakableBlocks_) delete block;
	for (auto block : spikeBlocks_) delete block;

	blocks_.clear();
	bounceBlocks_.clear();
	breakableBlocks_.clear();
	spikeBlocks_.clear();
}

void GameWorld::reset()
{
	// 기존 객체 정리
	cleanup();

	// 플레이어 리셋
	player_.reset();
	player_.setTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
	player_.velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);

	// 블럭 재생성
	createFloorBlocks();
	createBounceBlocks();
	createBreakableBlocks();
	createSpikeBlocks();

	// 게임 상태 리셋
	score_ = 0;
	gameStarted_ = true;

	std::cout << "GameWorld reset!" << std::endl;
}

void GameWorld::update(float deltaTime)
{
	if (!gameStarted_) return;

	// 플레이어 업데이트
	player_.update(deltaTime);

	// 충돌 검사
	checkCollisions();

	// 플레이어가 떨어지면 리셋
	if (player_.getTranslation().y < -10.0f)
	{
		player_.setTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
		player_.velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
		reset();
	}
}

void GameWorld::draw()
{
	player_.draw();

	for (auto block : blocks_) {
		block->draw();
	}
	for (auto block : bounceBlocks_) {
		block->draw();
	}
	for (auto block : breakableBlocks_) {
		block->draw();
	}
	for (auto block : spikeBlocks_) {
		block->draw();
	}
}

void GameWorld::checkCollisions()
{
	// 일반 블럭과 충돌
	for (auto block : blocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
		}
	}

	// 튕기는 블럭과 충돌
	for (auto block : bounceBlocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
		}
	}

	// 부서지는 블럭과 충돌
	for (auto it = breakableBlocks_.begin(); it != breakableBlocks_.end(); ) {
		BREAKABLE_BLOCK* block = *it;

		if (player_.checkCollision(block)) {
			player_.onCollision(block);
			block->onCollision(&player_);

			// 부서진 블럭 제거
			if (block->isBroken_) {
				addScore(10);
				std::cout << "Block broken! Score: " << score_ << std::endl;
				delete block;
				it = breakableBlocks_.erase(it);
				continue;
			}
		}
		++it;
	}

	// 가시 블럭과 충돌
	for (auto block : spikeBlocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
			std::cout << "Hit spike! Resetting player..." << std::endl;
		}
	}
}

void GameWorld::addBlock(BLOCK* block)
{
	blocks_.push_back(block);
}

void GameWorld::addBounceBlock(BOUNCE_BLOCK* block)
{
	bounceBlocks_.push_back(block);
}

void GameWorld::addBreakableBlock(BREAKABLE_BLOCK* block)
{
	breakableBlocks_.push_back(block);
}

void GameWorld::addSpikeBlock(SPIKE_BLOCK* block)
{
	spikeBlocks_.push_back(block);
}

// === 내부 헬퍼 함수들 ===	

void GameWorld::createFloorBlocks()
{
	// 바닥 블럭 생성 (일반 블럭)
	for (int i = -5; i <= 5; i++) {
		for (int j = -5; j <= 5; j++) {
			BLOCK* block = new BLOCK();
			block->init("obj/cube.obj", shaderProgramID_);
			block->setTranslation(glm::vec3(i * 2.0f, -1.0f, j * 2.0f));
			block->setSelfScale(glm::vec3(0.9f, 0.2f, 0.9f));
			blocks_.push_back(block);
		}
	}
}

void GameWorld::createBounceBlocks()
{
	// 튕기는 블럭 생성
	for (int i = 0; i < 5; i++) {
		BOUNCE_BLOCK* bounceBlock = new BOUNCE_BLOCK();
		bounceBlock->init("obj/cube.obj", shaderProgramID_);
		bounceBlock->setTranslation(glm::vec3((i - 2) * 3.0f, 0.0f, 0.0f));
		bounceBlock->setSelfScale(glm::vec3(0.8f, 0.3f, 0.8f));
		bounceBlocks_.push_back(bounceBlock);
	}
}

void GameWorld::createBreakableBlocks()
{
	// 부서지는 블럭 생성
	for (int i = 0; i < 3; i++) {
		BREAKABLE_BLOCK* breakableBlock = new BREAKABLE_BLOCK();
		breakableBlock->init("obj/cube.obj", shaderProgramID_);
		breakableBlock->setTranslation(glm::vec3(i * 2.0f, 1.0f, 3.0f));
		breakableBlock->setSelfScale(glm::vec3(0.7f, 0.7f, 0.7f));
		breakableBlocks_.push_back(breakableBlock);
	}
}

void GameWorld::createSpikeBlocks()
{
	// 가시 블럭 생성
	for (int i = 0; i < 2; i++) {
		SPIKE_BLOCK* spikeBlock = new SPIKE_BLOCK();
		spikeBlock->init("obj/cube.obj", shaderProgramID_); // spike.obj가 없으면 obj/cube.obj 사용
		spikeBlock->setTranslation(glm::vec3(-3.0f + i * 6.0f, 2.0f, -3.0f));
		spikeBlock->setSelfScale(glm::vec3(0.5f, 0.5f, 0.5f));
		spikeBlocks_.push_back(spikeBlock);
	}
}