#include "GameWorld.h"
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

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
	createArrowBlocks();     // 화살표 블럭
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
	for (auto block : arrowBlocks_) delete block;
	for (auto star : stars_) delete star;

	blocks_.clear();
	bounceBlocks_.clear();
	breakableBlocks_.clear();
	spikeBlocks_.clear();
	arrowBlocks_.clear();
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
	createArrowBlocks();
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
		PlaySound(L"sounds\\dead.wav", NULL, SND_FILENAME | SND_ASYNC);
		reset();
	}
}

void GameWorld::draw()
{
	if (ThirdPersonView_) player_.draw();

	// 궤적 표시가 활성화된 경우에만 그리기
	if (showTrajectory_) {
		// 모든 블록을 하나의 벡터로 수집
		std::vector<ParentModel*> allBlocks;
		for (auto block : blocks_) allBlocks.push_back(block);
		for (auto block : bounceBlocks_) allBlocks.push_back(block);
		for (auto block : breakableBlocks_) allBlocks.push_back(block);

		// 플레이어 궤적 예측 표시 (블록 정보 전달)
		trajectoryPredictor_.draw(
			player_.getPosition(),
			player_.velocity_,
			allBlocks,
			50,
			0.05f
		);
	}

	for (auto block : blocks_) block->draw();
	for (auto block : bounceBlocks_) block->draw();
	for (auto block : breakableBlocks_) block->draw();
	for (auto block : spikeBlocks_) block->draw();
	for (auto block : arrowBlocks_) block->draw();
	for (auto star : stars_) star->draw();
}

void GameWorld::checkCollisions()
{
	// 일반 블럭
	for (auto block : blocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
			PlaySound(L"sounds\\bounce.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
	}

	// 튕기는 블럭
	for (auto block : bounceBlocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
			// 바운스 효과음 재생
			PlaySound(L"sounds\\bounce.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
	}

	// 부서지는 블럭
	for (auto it = breakableBlocks_.begin(); it != breakableBlocks_.end(); ) {
		BREAKABLE_BLOCK* block = *it;
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
			PlaySound(L"sounds\\bounce.wav", NULL, SND_FILENAME | SND_ASYNC);
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
			PlaySound(L"sounds\\dead.wav", NULL, SND_FILENAME | SND_ASYNC);
			player_.onCollision(block); // 닿으면 사망(Reset)
			reset();
		}
	}

	// 화살표 블록 충돌 체크
	for (auto block : arrowBlocks_) {
		if (player_.checkCollision(block)) {
			player_.onCollision(block);
		}
	}

	// 별 (블럭)
	for (auto it = stars_.begin(); it != stars_.end(); ) {
		STAR* star = *it;
		if (player_.checkCollision(star)) {
			player_.onCollision(star);
			star->onCollision(&player_);
			if (star->isCollected_) {
				// 별 수집 효과음 재생
				PlaySound(L"sounds\\star.wav", NULL, SND_FILENAME | SND_ASYNC);
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
void GameWorld::addArrowBlock(ARROW_BLOCK* block) { arrowBlocks_.push_back(block); }
void GameWorld::addStar(STAR* star) { stars_.push_back(star); }

// ==========================================
//              레벨 디자인 구현
// ==========================================

//

void GameWorld::createFloorBlocks()
{
	// ==========================================
	//          SECTION 1: 시작 구역
	// ==========================================
	// 1-1. 시작 플랫폼 (3x3 안전지대) - 바운스로 변경
	for (int x = -1; x <= 1; x++) {
		for (int z = -1; z <= 1; z++) {
			BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
			block->init("obj/uv_cube.obj", shaderProgramID_, 0.3f, 0.3f, 0.3f);
			block->loadTexture("img/default.png");
			block->setTranslation(glm::vec3(x * 2.0f, -1.0f, z * 2.0f));
			block->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
			blocks_.push_back(block);
		}
	}

	// ==========================================
	//     SECTION 2: 좁은 징검다리 (난이도 상)
	// ==========================================
	// 2-1. 일직선 징검다리 (작은 발판들) - 바운스로 변경
	for (int i = 1; i <= 5; i++) {
		BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
		block->loadTexture("img/default.png");
		block->setTranslation(glm::vec3(0.0f, -1.0f - i * 0.3f, -2.0f - (i * 2.5f)));
		block->setSelfScale(glm::vec3(0.6f, 0.6f, 0.6f));
		blocks_.push_back(block);
	}

	// 2-2. 지그재그 구간 - 바운스로 변경
	for (int i = 0; i < 4; i++) {
		BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
		block->loadTexture("img/default.png");
		float xOffset = (i % 2 == 0) ? -2.0f : 2.0f;
		block->setTranslation(glm::vec3(xOffset, -2.5f, -17.0f - (i * 3.0f)));
		block->setSelfScale(glm::vec3(0.7f, 0.7f, 0.7f));
		blocks_.push_back(block);
	}

	// ==========================================
	//     SECTION 3: 첫 번째 화살표 + 착지 구역
	// ==========================================
	// 3-1. 화살표 발사대 발판 - 바운스로 변경
	BOUNCE_BLOCK* arrowPlatform1 = new BOUNCE_BLOCK();
	arrowPlatform1->init("obj/uv_cube.obj", shaderProgramID_, 0.4f, 0.4f, 0.4f);
	arrowPlatform1->loadTexture("img/default.png");
	arrowPlatform1->setTranslation(glm::vec3(2.0f, -2.5f, -30.0f));
	arrowPlatform1->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	blocks_.push_back(arrowPlatform1);

	// 3-2. 화살표 착지 플랫폼 (높은 곳) - 바운스로 변경
	for (int x = -1; x <= 1; x++) {
		BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.6f, 0.6f, 0.6f);
		block->loadTexture("img/default.png");
		block->setTranslation(glm::vec3(x * 2.0f, 5.0f, -45.0f));
		block->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
		blocks_.push_back(block);
	}

	// ==========================================
	//     SECTION 4: 부서지는 다리 + 바운스 조합
	// ==========================================
	// 4-1. 좁은 안전 발판 - 바운스로 변경
	BOUNCE_BLOCK* safe1 = new BOUNCE_BLOCK();
	safe1->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
	safe1->loadTexture("img/default.png");
	safe1->setTranslation(glm::vec3(0.0f, 5.0f, -50.0f));
	safe1->setSelfScale(glm::vec3(0.8f, 0.8f, 0.8f));
	blocks_.push_back(safe1);

	// 4-2. 낮은 바운스 착지 구역 - 바운스로 변경
	for (int x = -1; x <= 1; x++) {
		BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
		block->loadTexture("img/default.png");
		block->setTranslation(glm::vec3(x * 2.0f, -1.0f, -60.0f));
		block->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
		blocks_.push_back(block);
	}

	// ==========================================
	//     SECTION 5: 연속 화살표 스테이지
	// ==========================================
	// 5-1. 두 번째 화살표 발사대 - 바운스로 변경
	BOUNCE_BLOCK* arrowPlatform2 = new BOUNCE_BLOCK();
	arrowPlatform2->init("obj/uv_cube.obj", shaderProgramID_, 0.4f, 0.4f, 0.4f);
	arrowPlatform2->loadTexture("img/default.png");
	arrowPlatform2->setTranslation(glm::vec3(-3.0f, -1.0f, -65.0f));
	arrowPlatform2->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	blocks_.push_back(arrowPlatform2);

	// 5-2. 중간 전환 플랫폼 - 바운스로 변경
	BOUNCE_BLOCK* midPlatform = new BOUNCE_BLOCK();
	midPlatform->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
	midPlatform->loadTexture("img/default.png");
	midPlatform->setTranslation(glm::vec3(-3.0f, 2.0f, -80.0f));
	midPlatform->setSelfScale(glm::vec3(1.2f, 1.2f, 1.2f));
	blocks_.push_back(midPlatform);

	// 5-3. 세 번째 화살표 발사대 - 바운스로 변경
	BOUNCE_BLOCK* arrowPlatform3 = new BOUNCE_BLOCK();
	arrowPlatform3->init("obj/uv_cube.obj", shaderProgramID_, 0.4f, 0.4f, 0.4f);
	arrowPlatform3->loadTexture("img/default.png");
	arrowPlatform3->setTranslation(glm::vec3(0.0f, 2.0f, -85.0f));
	arrowPlatform3->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	blocks_.push_back(arrowPlatform3);

	// ==========================================
	//     SECTION 6: 가시 미로 구간
	// ==========================================
	// 6-1. 가시 앞 발판 - 바운스로 변경
	BOUNCE_BLOCK* spikeEntry = new BOUNCE_BLOCK();
	spikeEntry->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
	spikeEntry->loadTexture("img/default.png");
	spikeEntry->setTranslation(glm::vec3(0.0f, 7.0f, -100.0f));
	spikeEntry->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	blocks_.push_back(spikeEntry);

	// 6-2. 가시 후 안전 발판 - 바운스로 변경
	BOUNCE_BLOCK* spikeExit = new BOUNCE_BLOCK();
	spikeExit->init("obj/uv_cube.obj", shaderProgramID_, 0.5f, 0.5f, 0.5f);
	spikeExit->loadTexture("img/default.png");	
	spikeExit->setTranslation(glm::vec3(0.0f, 7.0f, -115.0f));
	spikeExit->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	blocks_.push_back(spikeExit);

	// ==========================================
	//     SECTION 7: 최종 점프 챌린지
	// ==========================================
	// 7-1. 높이가 다른 계단식 발판 - 바운스로 변경
	for (int i = 0; i < 5; i++) {
		BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 0.6f, 0.6f, 0.6f);
		block->loadTexture("img/default.png");
		block->setTranslation(glm::vec3(0.0f, 7.0f + i * 1.5f, -120.0f - (i * 4.0f)));
		block->setSelfScale(glm::vec3(0.8f, 0.8f, 0.8f));
		blocks_.push_back(block);
	}

	// ==========================================
	//     SECTION 8: 도착 지점 (Goal)
	// ==========================================
	for (int x = -2; x <= 2; x++) {
		for (int z = 0; z < 3; z++) {
			BOUNCE_BLOCK* block = new BOUNCE_BLOCK();
			block->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.84f, 0.0f); // 금색
			block->loadTexture("img/default.png");
			block->setTranslation(glm::vec3(x * 2.0f, 14.0f, -142.0f - (z * 2.0f)));
			block->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
			blocks_.push_back(block);
		}
	}
}

void GameWorld::createBounceBlocks()
{
	// 바운스 1: 첫 번째 높이 극복용
	BOUNCE_BLOCK* bounce1 = new BOUNCE_BLOCK();
	bounce1->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 0.5f, 1.0f);
	bounce1->loadTexture("img/default.png");
	bounce1->setTranslation(glm::vec3(0.0f, -4.0f, -18.0f));
	bounce1->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	bounceBlocks_.push_back(bounce1);

	// 바운스 2: 고공 점프용 (부서지는 다리 전)
	BOUNCE_BLOCK* bounce2 = new BOUNCE_BLOCK();
	bounce2->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 0.5f, 1.0f);
	bounce2->loadTexture("img/default.png");
	bounce2->setTranslation(glm::vec3(0.0f, 3.5f, -52.0f));
	bounce2->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	bounceBlocks_.push_back(bounce2);

	// 바운스 3: 최종 구간 진입용
	BOUNCE_BLOCK* bounce3 = new BOUNCE_BLOCK();
	bounce3->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 0.5f, 1.0f);
	bounce3->loadTexture("img/default.png");
	bounce3->setTranslation(glm::vec3(0.0f, 5.0f, -117.0f));
	bounce3->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	bounceBlocks_.push_back(bounce3);
}

void GameWorld::createBreakableBlocks()
{
	// 부서지는 다리 1: 바운스 후 빠르게 통과해야 함
	for (int i = 0; i < 4; i++) {
		BREAKABLE_BLOCK* block = new BREAKABLE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 1.0f);
		block->loadTexture("img/breakable.png");
		block->setTranslation(glm::vec3(0.0f, 5.0f, -54.0f - (i * 1.8f)));
		block->setSelfScale(glm::vec3(0.7f, 0.7f, 0.7f));
		breakableBlocks_.push_back(block);
	}

	// 부서지는 다리 2: 연속 점프 필요
	for (int i = 0; i < 3; i++) {
		BREAKABLE_BLOCK* block = new BREAKABLE_BLOCK();
		block->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 1.0f);
		block->loadTexture("img/breakable.png");
		float xOffset = (i % 2 == 0) ? -1.5f : 1.5f;
		block->setTranslation(glm::vec3(xOffset, 7.0f, -105.0f - (i * 3.0f)));
		block->setSelfScale(glm::vec3(0.8f, 0.8f, 0.8f));
		breakableBlocks_.push_back(block);
	}
}

void GameWorld::createSpikeBlocks()
{
	// 가시 1: 낮은 가시 (점프로 통과)
	SPIKE_BLOCK* spike1 = new SPIKE_BLOCK();
	spike1->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 0.0f);
	spike1->loadTexture("img/spike.png");
	spike1->setTranslation(glm::vec3(0.0f, -2.5f, -28.0f));
	spike1->setSelfScale(glm::vec3(1.0f, 0.4f, 1.5f));
	spikeBlocks_.push_back(spike1);

	// 가시 2: 지그재그 구간 중간
	SPIKE_BLOCK* spike2 = new SPIKE_BLOCK();
	spike2->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 0.0f);
	spike2->loadTexture("img/spike.png");
	spike2->setTranslation(glm::vec3(0.0f, -2.5f, -23.0f));
	spike2->setSelfScale(glm::vec3(0.8f, 0.4f, 0.8f));
	spikeBlocks_.push_back(spike2);

	// 가시 3: 가시 미로 구간 (3개 배치)
	for (int i = 0; i < 3; i++) {
		SPIKE_BLOCK* spike = new SPIKE_BLOCK();
		spike->init("obj/uv_cube.obj", shaderProgramID_, 1.0f, 0.0f, 0.0f);
		spike->loadTexture("img/spike.png");
		spike->setTranslation(glm::vec3(0.0f, 6.5f, -107.0f - (i * 2.5f)));
		spike->setSelfScale(glm::vec3(0.6f, 0.5f, 1.2f));
		spikeBlocks_.push_back(spike);
	}
}

void GameWorld::createArrowBlocks()
{
	// 화살표 1: 수평 + 상승 (첫 번째 화살표 구간)
	ARROW_BLOCK* arrow1 = new ARROW_BLOCK();
	arrow1->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 1.0f, 0.0f);
	arrow1->loadTexture("img/arrow.png");
	arrow1->setTranslation(glm::vec3(2.0f, -1.5f, -32.0f));
	arrow1->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	arrow1->setArrowDirection(glm::vec3(0.0f, 0.5f, -1.0f));
	arrow1->launchSpeed_ = 15.0f;
	arrowBlocks_.push_back(arrow1);

	// 화살표 2: 옆으로 + 하강 (두 번째 화살표 구간)
	ARROW_BLOCK* arrow2 = new ARROW_BLOCK();
	arrow2->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 1.0f, 0.0f);
	arrow2->loadTexture("img/arrow.png");
	arrow2->setTranslation(glm::vec3(-3.0f, 0.5f, -67.0f));
	arrow2->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	arrow2->setArrowDirection(glm::vec3(0.0f, 0.3f, -1.0f));
	arrow2->launchSpeed_ = 18.0f;
	arrowBlocks_.push_back(arrow2);

	// 화살표 3: 급상승 (세 번째 화살표 구간)
	ARROW_BLOCK* arrow3 = new ARROW_BLOCK();
	arrow3->init("obj/uv_cube.obj", shaderProgramID_, 0.0f, 1.0f, 0.0f);
	arrow3->loadTexture("img/arrow.png");
	arrow3->setTranslation(glm::vec3(0.0f, 3.0f, -87.0f));
	arrow3->setSelfScale(glm::vec3(1.0f, 1.0f, 1.0f));
	arrow3->setArrowDirection(glm::vec3(0.0f, 0.8f, -0.5f));
	arrow3->launchSpeed_ = 20.0f;
	arrowBlocks_.push_back(arrow3);
}

void GameWorld::createStars()
{
	//// 별 1: 시작 지점 (튜토리얼)
	//STAR* star1 = new STAR();
	//star1->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	//star1->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	//star1->setTranslation(glm::vec3(0.0f, 2.0f, 0.0f));
	//star1->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	//stars_.push_back(star1);

	// 별 2: 징검다리 중간 (어려움)
	STAR* star2 = new STAR();
	star2->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star2->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star2->setTranslation(glm::vec3(0.0f, 0.5f, -10.0f));
	star2->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	stars_.push_back(star2);

	// 별 3: 첫 화살표 착지 지점
	STAR* star3 = new STAR();
	star3->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star3->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star3->setTranslation(glm::vec3(0.0f, 8.0f, -45.0f));
	star3->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	stars_.push_back(star3);

	// 별 4: 부서지는 다리 끝
	STAR* star4 = new STAR();
	star4->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star4->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star4->setTranslation(glm::vec3(0.0f, 2.0f, -60.0f));
	star4->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	stars_.push_back(star4);

	// 별 5: 중간 전환 플랫폼
	STAR* star5 = new STAR();
	star5->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star5->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star5->setTranslation(glm::vec3(-3.0f, 5.0f, -80.0f));
	star5->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	stars_.push_back(star5);

	// 별 6: 가시 미로 위 (매우 어려움)
	STAR* star6 = new STAR();
	star6->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star6->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star6->setTranslation(glm::vec3(0.0f, 10.0f, -107.0f));
	star6->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	stars_.push_back(star6);

	// 별 7: 최종 계단 중간
	STAR* star7 = new STAR();
	star7->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star7->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star7->setTranslation(glm::vec3(0.0f, 12.0f, -130.0f));
	star7->setSelfScale(glm::vec3(0.15f, 0.3f, 0.15f));
	stars_.push_back(star7);

	// 별 8: 골인 지점 (보너스)
	STAR* star8 = new STAR();
	star8->init("obj/star.obj", shaderProgramID_, 1.0f, 1.0f, 0.0f);
	star8->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	star8->setTranslation(glm::vec3(0.0f, 17.0f, -143.0f));
	star8->setSelfScale(glm::vec3(0.2f, 0.4f, 0.2f)); // 큰 별
	stars_.push_back(star8);
}