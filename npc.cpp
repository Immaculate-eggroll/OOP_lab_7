#include "npc.h"
#include <cmath>
#include <random>
#include <chrono>
#include <mutex>

NPC::NPC(Type type, const std::string& name, int x, int y) 
    : type(type), name(name), x(x), y(y), alive(true) {}

std::string NPC::getName() const {
    std::shared_lock lock(mutex);
    return name;
}

NPC::Type NPC::getType() const {
    return type;
}

int NPC::getX() const {
    std::shared_lock lock(mutex);
    return x;
}

int NPC::getY() const {
    std::shared_lock lock(mutex);
    return y;
}

bool NPC::isAlive() const {
    std::shared_lock lock(mutex);
    return alive;
}

void NPC::setPosition(int x, int y) {
    std::unique_lock lock(mutex);
    this->x = x;
    this->y = y;
}

void NPC::kill() {
    std::unique_lock lock(mutex);
    alive = false;
}

int NPC::getMoveDistance() const {
    // По таблице из задания для варианта 7:
    // Орк: 20, Рыцарь: 30, Медведь: 5
    switch (type) {
        case Type::ORC: return 20;
        case Type::KNIGHT: return 30;
        case Type::BEAR: return 5;
        default: return 0;
    }
}

int NPC::getKillDistance() const {
    switch (type) {
        case Type::ORC: return 10;
        case Type::KNIGHT: return 10;
        case Type::BEAR: return 10;
        default: return 0;
    }
}

bool NPC::canKill(Type otherType) const {
    switch (type) {
        case Type::ORC: return otherType == Type::BEAR;    // Орки убивают медведей
        case Type::BEAR: return otherType == Type::KNIGHT; // Медведи убивают рыцарей
        case Type::KNIGHT: return otherType == Type::ORC;  // Рыцари убивают орков
        default: return false;
    }
}

bool NPC::inRange(const NPC& other, int range) const {
    std::shared_lock lock1(mutex, std::defer_lock);
    std::shared_lock lock2(other.mutex, std::defer_lock);
    std::lock(lock1, lock2);
    
    int dx = x - other.x;
    int dy = y - other.y;
    return std::sqrt(dx*dx + dy*dy) <= range;
}

double NPC::distanceTo(const NPC& other) const {
    std::shared_lock lock1(mutex, std::defer_lock);
    std::shared_lock lock2(other.mutex, std::defer_lock);
    std::lock(lock1, lock2);
    
    int dx = x - other.x;
    int dy = y - other.y;
    return std::sqrt(dx*dx + dy*dy);
}

void NPC::move(int maxX, int maxY) {
    if (!isAlive()) return;
    
    std::unique_lock lock(mutex);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(-getMoveDistance(), getMoveDistance());
    
    int newX = x + dist(gen);
    int newY = y + dist(gen);
    
    newX = std::max(0, std::min(maxX - 1, newX));
    newY = std::max(0, std::min(maxY - 1, newY));
    
    x = newX;
    y = newY;
}

std::pair<int, int> NPC::rollDice() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 6);
    
    return {dist(gen), dist(gen)};
}
