#ifndef NPC_H
#define NPC_H

#include <string>
#include <memory>
#include <random>
#include <mutex>
#include <shared_mutex>

class NPC {
public:
    enum class Type { ORC, KNIGHT, BEAR };

private:
    std::string name;
    Type type;
    int x;
    int y;
    bool alive;
    mutable std::shared_mutex mutex;

    static std::string typeToString(Type t) {
        switch (t) {
            case Type::ORC: return "Orc";
            case Type::KNIGHT: return "Knight";
            case Type::BEAR: return "Bear";
            default: return "Unknown";
        }
    }

public:
    NPC(Type type, const std::string& name, int x, int y);
    virtual ~NPC() = default;

    std::string getName() const;
    Type getType() const;
    std::string getTypeString() const;
    int getX() const;
    int getY() const;
    bool isAlive() const;
    void setPosition(int x, int y);
    void kill();

    int getMoveDistance() const;
    int getKillDistance() const;
    
    bool canKill(Type otherType) const;
    
    bool inRange(const NPC& other, int range) const;
    double distanceTo(const NPC& other) const;
    
    void move(int maxX, int maxY);
    std::pair<int, int> rollDice() const;

    void readLock() const { mutex.lock_shared(); }
    void readUnlock() const { mutex.unlock_shared(); }
    void writeLock() { mutex.lock(); }
    void writeUnlock() { mutex.unlock(); }
};

using NPCPtr = std::shared_ptr<NPC>;

#endif
