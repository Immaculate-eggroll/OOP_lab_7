#include <gtest/gtest.h>
#include "npc.h"
#include "game.h"
#include <thread>
#include <chrono>

TEST(NPCTest, Creation) {
    NPC npc(NPC::Type::ORC, "TestOrc", 10, 20);
    
    EXPECT_EQ(npc.getName(), "TestOrc");
    EXPECT_EQ(npc.getType(), NPC::Type::ORC);
    EXPECT_EQ(npc.getTypeString(), "Orc");
    EXPECT_EQ(npc.getX(), 10);
    EXPECT_EQ(npc.getY(), 20);
    EXPECT_TRUE(npc.isAlive());
}

TEST(NPCTest, MovementDistances) {
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

TEST(NPCTest, CanKill) {
    NPC orc(NPC::Type::ORC, "Orc", 0, 0);
    NPC knight(NPC::Type::KNIGHT, "Knight", 0, 0);
    NPC bear(NPC::Type::BEAR, "Bear", 0, 0);
    
    // Орки убивают медведей
    EXPECT_TRUE(orc.canKill(NPC::Type::BEAR));
    EXPECT_FALSE(orc.canKill(NPC::Type::KNIGHT));
    EXPECT_FALSE(orc.canKill(NPC::Type::ORC));
    
    // Медведи убивают рыцарей
    EXPECT_TRUE(bear.canKill(NPC::Type::KNIGHT));
    EXPECT_FALSE(bear.canKill(NPC::Type::ORC));
    EXPECT_FALSE(bear.canKill(NPC::Type::BEAR));
    
    // Рыцари убивают орков
    EXPECT_TRUE(knight.canKill(NPC::Type::ORC));
    EXPECT_FALSE(knight.canKill(NPC::Type::BEAR));
    EXPECT_FALSE(knight.canKill(NPC::Type::KNIGHT));
}

TEST(NPCTest, MoveWithinBounds) {
    NPC npc(NPC::Type::ORC, "Test", 50, 50);
    
    // Многократное движение должно оставаться в пределах 0-99
    for (int i = 0; i < 100; ++i) {
        npc.move(100, 100);
        EXPECT_GE(npc.getX(), 0);
        EXPECT_LT(npc.getX(), 100);
        EXPECT_GE(npc.getY(), 0);
        EXPECT_LT(npc.getY(), 100);
    }
}

TEST(NPCTest, InRange) {
    NPC npc1(NPC::Type::ORC, "Npc1", 0, 0);
    NPC npc2(NPC::Type::KNIGHT, "Npc2", 5, 0);
    
    EXPECT_TRUE(npc1.inRange(npc2, 10));
    EXPECT_FALSE(npc1.inRange(npc2, 4));
}

TEST(NPCTest, Kill) {
    NPC npc(NPC::Type::ORC, "Test", 0, 0);
    
    EXPECT_TRUE(npc.isAlive());
    npc.kill();
    EXPECT_FALSE(npc.isAlive());
}

TEST(GameTest, Initialization) {
    Game game;
    
    // Должно быть создано 50 NPC
    EXPECT_EQ(game.getNPCs().size(), 50);
    
    // Все NPC должны быть живы при инициализации
    EXPECT_EQ(game.getAliveCount(), 50);
}

TEST(GameTest, ShortSimulation) {
    Game game;
    
    // Запускаем игру на очень короткое время
    game.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    game.stop();
    
    // После остановки должно быть не более 50 живых NPC
    EXPECT_LE(game.getAliveCount(), 50);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
