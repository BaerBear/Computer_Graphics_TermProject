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

	// 플레이어 초기화 (위치: 시작 지점 위)
	player_.init("obj/uv_sphere.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	player_.setTranslation(glm::vec3(0.0f, 2.0f, 0.0f));
	glm::vec3 scale = player_.getSelfScale() * player_.getScaleFactor();
	player_.setSelfScale(glm::vec3(0.5f, 0.5f, 0.5f));
	player_.radius_ = 0.5f * player_.getScaleFactor();

	// 맵 생성 (순서대로 배치)
	createFloorBlocks();      // 기본 바닥 및 안전지대
	createBounceBlocks();     // 점프 구간
	createBreakableBlocks();  // 함정 다리
	createSpikeBlocks();      // 가시 장애물
	createStars();            // 별 배치

	trajectoryPredictor_.init(shaderProgramID_);

	std::cout << "** GameWorld initialized **" << std::endl;
	std::cout << "Level Generated with Obstacle Course!" << std::endl;
}

void GameWorld::cleanup()
{
	for (auto block : blocks_) delete block;
	for (auto block : bounceBlocks_) delete block;
	for (auto block : breakableBlocks_) delete block;
	for (auto block : spikeBlocks_) delete block;
	for (auto star : stars_) delete star;

	blocks_.clear();
	bounceBlocks_.clear();
	breakableBlocks_.clear();
	spikeBlocks_.clear();
	stars_.clear();
}

void GameWorld::reset()
{
	cleanup();

	// 플레이어 리셋
	player_.reset();
	player_.setTranslation(glm::vec3(0.0f, 2.0f, 0.0f));
	glm::vec3 scale = player_.getSelfScale() * player_.getScaleFactor();
	player_.setSelfScale(glm::vec3(0.5f, 0.5f, 0.5f));
	player_.radius_ = 0.5f * player_.getScaleFactor();
	player_.velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);

	createFloorBlocks();
	createBounceBlocks();
	createBreakableBlocks();
	createSpikeBlocks();
	createStars();

	score_ = 0;
	gameStarted_ = true;
	std::cout << "GameWorld reset!" << std::endl;
}

void GameWorld::update(float deltaTime)
{
	if (!gameStarted_) return;

	player_.update(deltaTime);

	// 별 회전 애니메이션
	for (auto star : stars_) {
		glm::vec3 currentRot = star->getRotation();
		star->setRotation(currentRot + glm::vec3(0.0f, 90.0f * deltaTime, 0.0f));
	}

	checkCollisions();

	// 낙사 처리 (맵 아래로 떨어지면 리셋)
	if (player_.getTranslation().y < -15.0f)
	{
		std::cout << "Game Over! Restarting..." << std::endl;
		reset();
	}
}

void GameWorld::draw()
{
	player_.draw();

	// 플레이어 궤적 예측 표시
	trajectoryPredictor_.draw(
		player_.getPosition(),  // 현재 위치
		player_.velocity_,      // 현재 속도
		50,                     // 점 개수 (많을수록 부드러움)
		0.05f                   // 시간 간격 (작을수록 정밀)
	);

	for (auto block : blocks_) block->draw();
	for (auto block : bounceBlocks_) block->draw();
	for (auto block : breakableBlocks_) block->draw();
	for (auto block : spikeBlocks_) block->draw();
	for (auto star : stars_) star->draw();
}

void GameWorld::checkCollisions()
{
	// 일반 블럭
	for (auto block : blocks_) {
		if (player_.checkCollision(block)) player_.onCollision(block);
	}

	// 튕기는 블럭
	for (auto block : bounceBlocks_) {
		if (player_.checkCollision(block)) player_.onCollision(block);
	}

	// 부서지는 블럭
	for (auto it = breakableBlocks_.begin(); it != breakableBlocks_.end(); ) {
		BREAKABLE_BLOCK* block = *it;
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
			block->onCollision(&player_); // 부서짐 체크

			if (block->isBroken_) {
				addScore(10);
				delete block;
				it = breakableBlocks_.erase(it);
				continue;
			}
		}
		++it;
	}

	// 가시 블럭
	for (auto block : spikeBlocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block); // 닿으면 사망(Reset)
		}
	}

	// 별 (블럭)
	for (auto it = stars_.begin(); it != stars_.end(); ) {
		STAR* star = *it;
		if (player_.checkCollision(star)) {
			player_.onCollision(star);
			star->onCollision(&player_);
			if (star->isCollected_) {
				addScore(50);
				delete star;
				it = stars_.erase(it);
				std::cout << "Star collected! Score: " << score_ << std::endl;
				continue;
			}
		}
		++it;
	}
}

void GameWorld::addBlock(BLOCK* block) { blocks_.push_back(block); }
void GameWorld::addBounceBlock(BOUNCE_BLOCK* block) { bounceBlocks_.push_back(block); }
void GameWorld::addBreakableBlock(BREAKABLE_BLOCK* block) { breakableBlocks_.push_back(block); }
void GameWorld::addSpikeBlock(SPIKE_BLOCK* block) { spikeBlocks_.push_back(block); }
void GameWorld::addStar(STAR* star) { stars_.push_back(star); }

// ==========================================
//              레벨 디자인 구현
// ==========================================

void GameWorld::createFloorBlocks()
{
	// 1. 시작 지점 (Start Zone) - 3x3 안전지대
	for (int x = -1; x <= 1; x++) {
		for (int z = -1; z <= 1; z++) {
			BLOCK* block = new BOUNCE_BLOCK();
			// 회색 바닥
			block->init("obj/uv_cube.obj", shaderProgramID_, 0.8f, 0.8f, 0.8f);
			block->setTranslation(glm::vec3(x * 2.0f, -1.0f, z * 2.0f));
			block->setSelfScale(glm::vec3(1.0f, 0.2f, 1.0f));
			blocks_.push_back(block);
		}
	}

	// 2. 징검다리 (Stepping Stones) - 일반 점프 구간
	for (int i = 1; i <= 3; i++) {
		BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.6f, 0.6f, 0.6f);
		// Z축으로 -4, -7, -10 만큼 떨어뜨려서 배치
		block->setTranslation(glm::vec3(0.0f, -1.0f, -2.0f - (i * 3.0f)));
		block->setSelfScale(glm::vec3(0.8f, 0.2f, 0.8f));
		blocks_.push_back(block);
	}

	// 3. 높은 플랫폼 (High Platform) - 바운스 블럭 착지 지점
	// 위치: Z = -20, 높이 Y = 2.0 (위로 올라감)
	for (int x = -1; x <= 1; x++) {
		BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.8f, 0.8f, 0.8f);
		block->setTranslation(glm::vec3(x * 2.0f, 2.0f, -20.0f));
		block->setSelfScale(glm::vec3(1.0f, 0.2f, 1.0f));
		blocks_.push_back(block);
	}

	// 4. 가시 함정 앞뒤 안전 발판
	// 가시 전 발판
	BLOCK* safe1 = new BOUNCE_BLOCK();
	safe1->init("obj/uv_cube.obj", shaderProgramID_, 0.6f, 0.6f, 0.6f);
	safe1->setTranslation(glm::vec3(0.0f, 2.0f, -32.0f));
	safe1->setSelfScale(glm::vec3(1.0f, 0.2f, 1.0f));
	blocks_.push_back(safe1);

	// 가시 후 발판
	BLOCK* safe2 = new BOUNCE_BLOCK();
	safe2->init("obj/uv_cube.obj", shaderProgramID_, 0.6f, 0.6f, 0.6f);
	safe2->setTranslation(glm::vec3(0.0f, 2.0f, -40.0f));
	safe2->setSelfScale(glm::vec3(1.0f, 0.2f, 1.0f));
	blocks_.push_back(safe2);

	// 5. 도착 지점 (Goal Zone) - 넓은 플랫폼
	for (int x = -2; x <= 2; x++) {
		for (int z = 0; z < 3; z++) {
			BLOCK* block = new BLOCK();
			// 도착 지점은 금색(노란색) 느낌
			block->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.84f, 0.0f);
			block->setTranslation(glm::vec3(x * 2.0f, 2.0f, -48.0f - (z * 2.0f)));
			block->setSelfScale(glm::vec3(1.0f, 0.2f, 1.0f));
			blocks_.push_back(block);
		}
	}
}

void GameWorld::createBounceBlocks()
{
	// 바운스 구간: 징검다리와 높은 플랫폼 사이
	// 위치: Z = -15 (낮은 곳에 배치해서 밟고 올라가게 함)
	BOUNCE_BLOCK* bounceBlock = new BOUNCE_BLOCK();
	// 파란색
	bounceBlock->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 0.5f, 1.0f);
	bounceBlock->setTranslation(glm::vec3(0.0f, -1.0f, -15.0f));
	bounceBlock->setSelfScale(glm::vec3(1.0f, 0.2f, 1.0f));
	bounceBlocks_.push_back(bounceBlock);
}

void GameWorld::createBreakableBlocks()
{
	// 부서지는 다리: 높은 플랫폼(Z=-20)에서 다음 구간으로 연결
	// 위치: Z = -23, -26, -29
	for (int i = 0; i < 3; i++) {
		BREAKABLE_BLOCK* block = new BREAKABLE_BLOCK();
		// 자주색 (경고 느낌)
		block->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 1.0f);
		block->setTranslation(glm::vec3(0.0f, 2.0f, -23.0f - (i * 2.5f)));
		block->setSelfScale(glm::vec3(0.8f, 0.2f, 0.8f));
		breakableBlocks_.push_back(block);
	}
}

void GameWorld::createSpikeBlocks()
{
	// 가시 함정 구간: 두 안전 발판 사이 (Z = -36)
	// 점프해서 넘어가야 함. 가시에 닿으면 죽음.
	SPIKE_BLOCK* spike = new SPIKE_BLOCK();
	// 빨간색
	spike->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 0.0f);
	// 발판보다 약간 낮게 배치하거나, 길게 배치해서 점프 유도
	spike->setTranslation(glm::vec3(0.0f, 1.5f, -36.0f));
	spike->setSelfScale(glm::vec3(0.5f, 0.5f, 2.0f)); // 길쭉한 가시밭
	spikeBlocks_.push_back(spike);
}

void GameWorld::createStars() {
	// 일단은 맵 곳곳에 별 배치
	STAR* star1 = new STAR();
	star1->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star1->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star1->setTranslation(glm::vec3(0.0f, 3.0f, 0.0f));
	star1->setSelfScale(glm::vec3(0.1f, 0.1f, 0.1f));
	stars_.push_back(star1);
}