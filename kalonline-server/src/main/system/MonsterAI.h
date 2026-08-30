#pragma once
#include "Entities.h"
#include "WorldManager.h"
#include <cstdint>
#include <memory>

namespace kal {

/**
 * @brief Monster AI system for KalOnline 2003
 * 
 * Implements behavior states: Idle, Patrol, Chase, Attack, Return, Dead
 * Handles aggro detection, pathfinding, and combat decisions
 */
class MonsterAI {
public:
    enum class State : uint8_t {
        Idle = 0,
        Patrol = 1,
        Chase = 2,
        Attack = 3,
        Return = 4,
        Dead = 5
    };

    explicit MonsterAI(EntityManager& entityManager, WorldManager& worldManager);

    /**
     * @brief Update AI for a specific monster
     * @param monster The monster entity to update
     * @param deltaTime Time elapsed since last update in seconds
     */
    void Update(Monster* monster, float deltaTime);

    /**
     * @brief Update all monsters in the world
     * @param deltaTime Time elapsed since last update
     */
    void UpdateAll(float deltaTime);

    /**
     * @brief Trigger aggro on a monster towards a target
     * @param monsterId The monster entity ID
     * @param targetId The target entity ID (player or other)
     * @param damage Amount of damage that triggered aggro
     */
    void TriggerAggro(uint32_t monsterId, uint32_t targetId, int damage);

    /**
     * @brief Check if a position is within monster's aggro radius
     */
    bool IsInAggroRange(const Monster* monster, float x, float y) const;

private:
    EntityManager& entityManager_;
    WorldManager& worldManager_;

    // AI Configuration
    static constexpr float UPDATE_INTERVAL = 0.5f; // Update AI every 500ms
    static constexpr float AGGRO_RADIUS = 150.0f;  // Default aggro range
    static constexpr float CHASE_RADIUS = 300.0f;  // Max chase distance
    static constexpr float RETURN_TIMEOUT = 10.0f; // Seconds before returning

    void UpdateState(Monster* monster, float deltaTime);
    void UpdateIdle(Monster* monster, float deltaTime);
    void UpdatePatrol(Monster* monster, float deltaTime);
    void UpdateChase(Monster* monster, float deltaTime);
    void UpdateAttack(Monster* monster, float deltaTime);
    void UpdateReturn(Monster* monster, float deltaTime);

    void FindTarget(Monster* monster);
    bool CanSeeTarget(const Monster* monster, const Character* target) const;
    float CalculateDistanceToTarget(const Monster* monster) const;
    void MoveTowardsTarget(Monster* monster, float deltaTime);
    void PerformAttack(Monster* monster);
    void ResetToSpawn(Monster* monster);
};

} // namespace kal
