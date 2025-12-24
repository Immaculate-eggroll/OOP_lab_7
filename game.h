#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "npc.h"

struct BattleTask {
    NPCPtr attacker;
    NPCPtr defender;
};

class Game {
private:
    static const int MAP_WIDTH = 100;
    static const int MAP_HEIGHT = 100;
    static const int INITIAL_NPC_COUNT = 50;
    static const int GAME_DURATION_SECONDS = 30;

    std::vector<NPCPtr> npcs;
    std::queue<BattleTask> battleQueue;
    
    std::atomic<bool> running;
    std::atomic<int> timeElapsed;
    
    std::mutex npcsMutex;
    std::mutex battleQueueMutex;
    std::mutex coutMutex;
    std::condition_variable battleCV;
    
    std::thread movementThread;
    std::thread battleThread;
    std::thread mainThread;
    
    void initializeNPCs();
    void movementWorker();
    void battleWorker();
    void mainWorker();
    void printMap();
    void checkBattles();
    void processBattle(const BattleTask& task);
    
public:
    Game();
    ~Game();
    
    void start();
    void stop();
    void waitForFinish();
    
    const std::vector<NPCPtr>& getNPCs() const;
    int getAliveCount() const;
};

#endif
