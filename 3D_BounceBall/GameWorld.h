#pragma once
#include "Headers.h"
#include "Player.h"
#include "Block.h"
#include <vector>
#include "PathTrace.h"

class GameWorld
{
public:
	GameWorld(GLuint shaderID);
	~GameWorld();

	// 초기화 및 정리
	void initialize();
	void cleanup();
	void reset();

	// 게임 로직
	void update(float deltaTime);
	void draw();
	void checkCollisions();

	// 플레이어 접근
	PLAYER* getPlayer() { return &player_; }

	// 블럭 추가/제거
	void addBlock(BLOCK* block);
	void addBounceBlock(BOUNCE_BLOCK* block);
	void addBreakableBlock(BREAKABLE_BLOCK* block);
	void addSpikeBlock(SPIKE_BLOCK* block);
	void addStar(STAR* star);

	// 게임 상태
	int getScore() const { return score_; }
	void addScore(int points) { score_ += points; }
	void setGameStarted(bool started) { gameStarted_ = started; }
	bool isGameStarted() const { return gameStarted_; }

private:
	GLuint shaderProgramID_;

	PLAYER player_;
	std::vector<BLOCK*> blocks_;
	std::vector<BOUNCE_BLOCK*> bounceBlocks_;
	std::vector<BREAKABLE_BLOCK*> breakableBlocks_;
	std::vector<SPIKE_BLOCK*> spikeBlocks_;
	std::vector<STAR*> stars_;

	bool gameStarted_;
	int score_;

	void createFloorBlocks();
	void createBounceBlocks();
	void createBreakableBlocks();
	void createSpikeBlocks();
	void createStars();

	TrajectoryPredictor trajectoryPredictor_;
};