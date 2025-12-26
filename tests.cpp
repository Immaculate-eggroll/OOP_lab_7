#include <gtest/gtest.h>
#include "npc.h"
#include "game.h"
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

TEST(NPCTest, Creation) {
    NPC npc(NPC::Type::ORC, "TestOrc", 10, 20);
    
    EXPECT_EQ(npc.getName(), "TestOrc");
    EXPECT_EQ(npc.getType(), NPC::Type::ORC);
    EXPECT_EQ(NPC::getTypeString(NPC::Type::ORC), "Orc");
    EXPECT_EQ(npc.getX(), 10);
    EXPECT_EQ(npc.getY(), 20);
    EXPECT_TRUE(npc.isAlive());
}

TEST(NPCTest, MoveDistances) {
    NPC orc(NPC::Type::ORC, "Orc", 0, 0);
    NPC knight(NPC::Type::KNIGHT, "Knight", 0, 0);
    NPC bear(NPC::Type::BEAR, "Bear", 0, 0);
    
    EXPECT_EQ(orc.getMoveDistance(), 20);
    EXPECT_EQ(knight.getMoveDistance(), 30);
    EXPECT_EQ(bear.getMoveDistance(), 5);
}

TEST(NPCTest, KillDistances) {
    NPC orc(NPC::Type::ORC, "Orc", 0, 0);
    NPC knight(NPC::Type::KNIGHT, "Knight", 0, 0);
    NPC bear(NPC::Type::BEAR, "Bear", 0, 0);
    
    EXPECT_EQ(orc.getKillDistance(), 10);
    EXPECT_EQ(knight.getKillDistance(), 10);
    EXPECT_EQ(bear.getKillDistance(), 10);
}

TEST(NPCTest, CanKillRules) {
    EXPECT_TRUE(NPC(NPC::Type::ORC, "", 0, 0).canKill(NPC::Type::BEAR));
    EXPECT_FALSE(NPC(NPC::Type::ORC, "", 0, 0).canKill(NPC::Type::KNIGHT));
    EXPECT_FALSE(NPC(NPC::Type::ORC, "", 0, 0).canKill(NPC::Type::ORC));
    
    EXPECT_TRUE(NPC(NPC::Type::BEAR, "", 0, 0).canKill(NPC::Type::KNIGHT));
    EXPECT_FALSE(NPC(NPC::Type::BEAR, "", 0, 0).canKill(NPC::Type::ORC));
    EXPECT_FALSE(NPC(NPC::Type::BEAR, "", 0, 0).canKill(NPC::Type::BEAR));
    
    EXPECT_TRUE(NPC(NPC::Type::KNIGHT, "", 0, 0).canKill(NPC::Type::ORC));
    EXPECT_FALSE(NPC(NPC::Type::KNIGHT, "", 0, 0).canKill(NPC::Type::BEAR));
    EXPECT_FALSE(NPC(NPC::Type::KNIGHT, "", 0, 0).canKill(NPC::Type::KNIGHT));
}

TEST(NPCTest, MoveWithinBounds) {
    NPC npc(NPC::Type::ORC, "Test", 50, 50);
    
    for (int i = 0; i < 10; ++i) {
        npc.move(100, 100);
        EXPECT_GE(npc.getX(), 0);
        EXPECT_LT(npc.getX(), 100);
        EXPECT_GE(npc.getY(), 0);
        EXPECT_LT(npc.getY(), 100);
    }
}

TEST(NPCTest, Kill) {
    NPC npc(NPC::Type::ORC, "Test", 0, 0);
    
    EXPECT_TRUE(npc.isAlive());
    npc.kill();
    EXPECT_FALSE(npc.isAlive());
}

TEST(NPCTest, InRange) {
    NPC npc1(NPC::Type::ORC, "Npc1", 0, 0);
    NPC npc2(NPC::Type::KNIGHT, "Npc2", 5, 0);
    
    EXPECT_TRUE(npc1.inRange(npc2, 10));
    EXPECT_FALSE(npc1.inRange(npc2, 4));
}

TEST(GameTest, Initialization) {
    Game game;
    
    EXPECT_EQ(game.getNPCs().size(), 50);
    EXPECT_EQ(game.getAliveCount(), 50);
}

TEST(GameTest, ThreadSafetyBasic) {
    Game game;
    
    EXPECT_NO_THROW(game.getAliveCount());
    EXPECT_NO_THROW(game.getNPCs());
    
    game.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    game.stop();
    
    EXPECT_LE(game.getAliveCount(), 50);
}

TEST(GameTest, BattleLogic) {
    NPC attacker(NPC::Type::ORC, "Attacker", 0, 0);
    NPC defender(NPC::Type::BEAR, "Defender", 1, 1);
    
    EXPECT_TRUE(attacker.canKill(defender.getType()));
    EXPECT_TRUE(attacker.inRange(defender, attacker.getKillDistance()));
}

TEST(GameTest, TypeStrings) {
    EXPECT_EQ(NPC::getTypeString(NPC::Type::ORC), "Orc");
    EXPECT_EQ(NPC::getTypeString(NPC::Type::KNIGHT), "Knight");
    EXPECT_EQ(NPC::getTypeString(NPC::Type::BEAR), "Bear");
}

TEST(IntegrationTest, MultipleNPCsCreation) {
    const int TEST_COUNT = 5;
    
    std::vector<std::unique_ptr<NPC>> npcs;
    
    for (int i = 0; i < TEST_COUNT; ++i) {
        NPC::Type type = static_cast<NPC::Type>(i % 3);
        npcs.push_back(std::make_unique<NPC>(type, "NPC_" + std::to_string(i), i * 10, i * 10));
    }
    
    EXPECT_EQ(npcs.size(), TEST_COUNT);
    
    for (int i = 0; i < TEST_COUNT; ++i) {
        EXPECT_TRUE(npcs[i]->isAlive());
        EXPECT_EQ(npcs[i]->getName(), "NPC_" + std::to_string(i));
    }
}

TEST(IntegrationTest, DiceRolling) {
    NPC npc(NPC::Type::ORC, "Test", 0, 0);
    
    for (int i = 0; i < 10; ++i) {
        auto [attack, defense] = npc.rollDice();
        
        EXPECT_GE(attack, 1);
        EXPECT_LE(attack, 6);
        EXPECT_GE(defense, 1);
        EXPECT_LE(defense, 6);
    }
}

TEST(ConcurrencyTest, BasicThreadOperations) {
    std::atomic<int> counter{0};
    
    std::thread t1([&counter]() {
        for (int i = 0; i < 50; ++i) {
            ++counter;
        }
    });
    
    std::thread t2([&counter]() {
        for (int i = 0; i < 50; ++i) {
            ++counter;
        }
    });
    
    t1.join();
    t2.join();
    
    EXPECT_EQ(counter, 100);
}

TEST(SimpleTest, BasicAssertions) {
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(2 + 2, 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
