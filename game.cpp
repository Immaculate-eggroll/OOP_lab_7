#include "game.h"
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>

Game::Game() : running(false), timeElapsed(0) {
    initializeNPCs();
}

Game::~Game() {
    stop();
}

void Game::initializeNPCs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> typeDist(0, 2);
    std::uniform_int_distribution<> posXDist(0, MAP_WIDTH - 1);
    std::uniform_int_distribution<> posYDist(0, MAP_HEIGHT - 1);
    
    std::lock_guard lock(npcsMutex);
    
    for (int i = 0; i < INITIAL_NPC_COUNT; ++i) {
        NPC::Type type = static_cast<NPC::Type>(typeDist(gen));
        std::string name = NPC::typeToString(type) + "_" + std::to_string(i);
        int x = posXDist(gen);
        int y = posYDist(gen);
        
        npcs.push_back(std::make_shared<NPC>(type, name, x, y));
    }
}

void Game::start() {
    running = true;
    
    movementThread = std::thread(&Game::movementWorker, this);
    battleThread = std::thread(&Game::battleWorker, this);
    mainThread = std::thread(&Game::mainWorker, this);
    
    std::cout << "Game started! Duration: " << GAME_DURATION_SECONDS << " seconds" << std::endl;
}

void Game::stop() {
    running = false;
    battleCV.notify_all();
    
    if (movementThread.joinable()) movementThread.join();
    if (battleThread.joinable()) battleThread.join();
    if (mainThread.joinable()) mainThread.join();
}

void Game::waitForFinish() {
    if (mainThread.joinable()) mainThread.join();
}

void Game::movementWorker() {
    while (running && timeElapsed < GAME_DURATION_SECONDS) {
        {
            std::lock_guard lock(npcsMutex);
            for (auto& npc : npcs) {
                if (npc->isAlive()) {
                    npc->move(MAP_WIDTH, MAP_HEIGHT);
                }
            }
        }
        
        checkBattles();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Game::battleWorker() {
    while (running) {
        std::unique_lock lock(battleQueueMutex);
        battleCV.wait(lock, [this]() { return !battleQueue.empty() || !running; });
        
        if (!running) break;
        
        if (!battleQueue.empty()) {
            BattleTask task = battleQueue.front();
            battleQueue.pop();
            lock.unlock();
            
            processBattle(task);
        }
    }
}

void Game::mainWorker() {
    auto startTime = std::chrono::steady_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        
        if (timeElapsed >= GAME_DURATION_SECONDS) {
            running = false;
            break;
        }
        
        printMap();
        
        {
            std::lock_guard lock(coutMutex);
            std::cout << "Time: " << timeElapsed << "s, Alive: " << getAliveCount() 
                      << "/" << npcs.size() << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    printMap();
    
    {
        std::lock_guard lock(coutMutex);
        std::cout << "\n=== GAME OVER ===" << std::endl;
        std::cout << "Survivors after " << GAME_DURATION_SECONDS << " seconds:" << std::endl;
        
        std::lock_guard lock2(npcsMutex);
        for (const auto& npc : npcs) {
            if (npc->isAlive()) {
                std::cout << npc->getTypeString() << " " << npc->getName() 
                          << " at (" << npc->getX() << ", " << npc->getY() << ")" << std::endl;
            }
        }
        std::cout << "Total survivors: " << getAliveCount() << std::endl;
    }
}

void Game::printMap() {
    const int DISPLAY_WIDTH = 50;
    const int DISPLAY_HEIGHT = 20;
    
    std::lock_guard lock(coutMutex);
    std::cout << "\033[2J\033[H"; // Clear screen
    
    std::vector<std::vector<char>> grid(DISPLAY_HEIGHT, std::vector<char>(DISPLAY_WIDTH, '.'));
    
    {
        std::shared_lock lock(npcsMutex);
        for (const auto& npc : npcs) {
            if (npc->isAlive()) {
                int displayX = (npc->getX() * DISPLAY_WIDTH) / MAP_WIDTH;
                int displayY = (npc->getY() * DISPLAY_HEIGHT) / MAP_HEIGHT;
                
                if (displayX >= 0 && displayX < DISPLAY_WIDTH && 
                    displayY >= 0 && displayY < DISPLAY_HEIGHT) {
                    
                    char symbol = '.';
                    switch (npc->getType()) {
                        case NPC::Type::ORC: symbol = 'O'; break;
                        case NPC::Type::KNIGHT: symbol = 'K'; break;
                        case NPC::Type::BEAR: symbol = 'B'; break;
                    }
                    grid[displayY][displayX] = symbol;
                }
            }
        }
    }
    
    std::cout << "+";
    for (int i = 0; i < DISPLAY_WIDTH; ++i) std::cout << "-";
    std::cout << "+\n";
    
    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        std::cout << "|";
        for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            std::cout << grid[y][x];
        }
        std::cout << "|\n";
    }
    
    std::cout << "+";
    for (int i = 0; i < DISPLAY_WIDTH; ++i) std::cout << "-";
    std::cout << "+\n";
    
    std::cout << "Legend: O=Orc, K=Knight, B=Bear, .=empty\n";
}

void Game::checkBattles() {
    std::shared_lock lock(npcsMutex);
    
    for (size_t i = 0; i < npcs.size(); ++i) {
        if (!npcs[i]->isAlive()) continue;
        
        for (size_t j = i + 1; j < npcs.size(); ++j) {
            if (!npcs[j]->isAlive()) continue;
            
            if (npcs[i]->inRange(*npcs[j], npcs[i]->getKillDistance()) ||
                npcs[j]->inRange(*npcs[i], npcs[j]->getKillDistance())) {
                
                if (npcs[i]->canKill(npcs[j]->getType())) {
                    std::lock_guard lock2(battleQueueMutex);
                    battleQueue.push({npcs[i], npcs[j]});
                    battleCV.notify_one();
                }
                else if (npcs[j]->canKill(npcs[i]->getType())) {
                    std::lock_guard lock2(battleQueueMutex);
                    battleQueue.push({npcs[j], npcs[i]});
                    battleCV.notify_one();
                }
            }
        }
    }
}

void Game::processBattle(const BattleTask& task) {
    if (!task.attacker->isAlive() || !task.defender->isAlive()) {
        return;
    }
    
    auto [attackRoll, defenseRoll] = task.attacker->rollDice();
    
    {
        std::lock_guard lock(coutMutex);
        std::cout << "BATTLE: " << task.attacker->getName() << " (attack: " << attackRoll 
                  << ") vs " << task.defender->getName() << " (defense: " << defenseRoll << ")" << std::endl;
    }
    
    if (attackRoll > defenseRoll) {
        task.defender->kill();
        
        std::lock_guard lock(coutMutex);
        std::cout << "  -> " << task.defender->getName() << " was killed by " 
                  << task.attacker->getName() << std::endl;
    } else {
        std::lock_guard lock(coutMutex);
        std::cout << "  -> " << task.defender->getName() << " survived the attack" << std::endl;
    }
}

const std::vector<NPCPtr>& Game::getNPCs() const {
    return npcs;
}

int Game::getAliveCount() const {
    std::shared_lock lock(npcsMutex);
    int count = 0;
    for (const auto& npc : npcs) {
        if (npc->isAlive()) count++;
    }
    return count;
}
