#include "MonsterAI.h"
#include "EntityManager.h"
#include "WorldManager.h"
#include "CombatSystem.h"
#include "Logger.h"
#include <cmath>

namespace kal {

MonsterAI::MonsterAI(EntityManager& entityManager, WorldManager& worldManager)
    : entityManager_(entityManager)
    , worldManager_(worldManager) {
}

void MonsterAI::Update(Monster* monster, float deltaTime) {
    if (!monster || monster->GetType() != EntityType::Monster) {
        return;
    }

    // Skip dead monsters
    if (monster->IsDead()) {
        return;
    }

    UpdateState(monster, deltaTime);
}

void MonsterAI::UpdateAll(float deltaTime) {
    // Get all monsters from entity manager
    auto monsters = entityManager_.GetAllMonsters();
    
    for (auto& monster : monsters) {
        if (monster && !monster->IsDead()) {
            Update(monster.get(), deltaTime);
        }
    }
}

void MonsterAI::TriggerAggro(uint32_t monsterId, uint32_t targetId, int damage) {
    auto monster = std::dynamic_pointer_cast<Monster>(entityManager_.GetEntityById(monsterId));
    if (!monster) {
        return;
    }

    // Add threat to the target
    monster->AddThreat(targetId, damage);

    // If not already chasing, switch to chase state
    if (monster->aiState != static_cast<uint8_t>(State::Chase) &&
        monster->aiState != static_cast<uint8_t>(State::Attack)) {
        
        monster->aiState = static_cast<uint8_t>(State::Chase);
        monster->currentTargetId = targetId;
        
        Logger::Debug("Monster {} triggered aggro on target {}", monsterId, targetId);
    }
}

bool MonsterAI::IsInAggroRange(const Monster* monster, float x, float y) const {
    if (!monster) return false;
    
    float dx = monster->x - x;
    float dy = monster->y - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    return distance <= AGGRO_RADIUS;
}

void MonsterAI::UpdateState(Monster* monster, float deltaTime) {
    auto state = static_cast<State>(monster->aiState);
    
    switch (state) {
        case State::Idle:
            UpdateIdle(monster, deltaTime);
            break;
        case State::Patrol:
            UpdatePatrol(monster, deltaTime);
            break;
        case State::Chase:
            UpdateChase(monster, deltaTime);
            break;
        case State::Attack:
            UpdateAttack(monster, deltaTime);
            break;
        case State::Return:
            UpdateReturn(monster, deltaTime);
            break;
        case State::Dead:
            // Do nothing
            break;
    }
}

void MonsterAI::UpdateIdle(Monster* monster, float deltaTime) {
    // Check for nearby players to aggro
    FindTarget(monster);
    
    // Random chance to start patrolling
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0f, 1.0f);
    
    if (dis(gen) < 0.01f) { // 1% chance per update to start patrolling
        monster->aiState = static_cast<uint8_t>(State::Patrol);
    }
}

void MonsterAI::UpdatePatrol(Monster* monster, float deltaTime) {
    // Check for nearby players to aggro (higher priority than patrol)
    FindTarget(monster);
    
    // Move to next patrol point
    // TODO: Implement patrol path following
}

void MonsterAI::UpdateChase(Monster* monster, float deltaTime) {
    if (monster->currentTargetId == 0) {
        // No target, return to spawn
        monster->aiState = static_cast<uint8_t>(State::Return);
        return;
    }
    
    auto target = entityManager_.GetEntityById(monster->currentTargetId);
    if (!target || target->IsDead()) {
        // Target lost or dead, return to spawn
        monster->aiState = static_cast<uint8_t>(State::Return);
        monster->currentTargetId = 0;
        return;
    }
    
    float distance = CalculateDistanceToTarget(monster);
    
    // Check if target is too far (leash range)
    if (distance > CHASE_RADIUS) {
        Logger::Debug("Monster {} lost target (too far)", monster->GetId());
        monster->aiState = static_cast<uint8_t>(State::Return);
        monster->currentTargetId = 0;
        return;
    }
    
    // Check if in attack range
    float attackRange = 50.0f; // Melee range
    if (distance <= attackRange) {
        monster->aiState = static_cast<uint8_t>(State::Attack);
    } else {
        // Move towards target
        MoveTowardsTarget(monster, deltaTime);
    }
}

void MonsterAI::UpdateAttack(Monster* monster, float deltaTime) {
    if (monster->currentTargetId == 0) {
        monster->aiState = static_cast<uint8_t>(State::Return);
        return;
    }
    
    auto target = entityManager_.GetEntityById(monster->currentTargetId);
    if (!target || target->IsDead()) {
        monster->aiState = static_cast<uint8_t>(State::Return);
        monster->currentTargetId = 0;
        return;
    }
    
    float distance = CalculateDistanceToTarget(monster);
    float attackRange = 50.0f;
    
    // If target moved out of range, chase again
    if (distance > attackRange) {
        monster->aiState = static_cast<uint8_t>(State::Chase);
        return;
    }
    
    // Perform attack if cooldown is ready
    // TODO: Check attack cooldown and perform attack
    PerformAttack(monster);
}

void MonsterAI::UpdateReturn(Monster* monster, float deltaTime) {
    // Return to spawn point
    ResetToSpawn(monster);
    
    // Once at spawn, go back to idle
    float distToSpawn = std::sqrt(
        std::pow(monster->x - monster->spawnX, 2) +
        std::pow(monster->y - monster->spawnY, 2)
    );
    
    if (distToSpawn < 10.0f) {
        monster->aiState = static_cast<uint8_t>(State::Idle);
        monster->currentTargetId = 0;
        // Clear threat table
        monster->threatTable.clear();
    }
}

void MonsterAI::FindTarget(Monster* monster) {
    // Get nearby characters
    auto chars = worldManager_.GetCharactersInRange(
        monster->mapId, 
        monster->x, 
        monster->y, 
        AGGRO_RADIUS
    );
    
    if (chars.empty()) {
        return;
    }
    
    // Find closest character that we can see
    Character* closest = nullptr;
    float closestDist = AGGRO_RADIUS;
    
    for (auto& ch : chars) {
        if (!ch || ch->IsDead()) continue;
        
        float dist = CalculateDistanceToTarget(monster);
        if (dist < closestDist && CanSeeTarget(monster, ch.get())) {
            closest = ch.get();
            closestDist = dist;
        }
    }
    
    if (closest) {
        monster->aiState = static_cast<uint8_t>(State::Chase);
        monster->currentTargetId = closest->GetId();
        Logger::Debug("Monster {} found target {}", monster->GetId(), closest->GetId());
    }
}

bool MonsterAI::CanSeeTarget(const Monster* monster, const Character* target) const {
    // Simple line-of-sight check (can be enhanced with raycasting)
    // For now, just check if on same map and within range
    if (monster->mapId != target->GetMapId()) {
        return false;
    }
    
    // Check if position is walkable (basic LOS)
    return worldManager_.IsPositionValid(monster->mapId, target->GetX(), target->GetY());
}

float MonsterAI::CalculateDistanceToTarget(const Monster* monster) const {
    if (monster->currentTargetId == 0) {
        return std::numeric_limits<float>::max();
    }
    
    auto target = entityManager_.GetEntityById(monster->currentTargetId);
    if (!target) {
        return std::numeric_limits<float>::max();
    }
    
    float dx = monster->x - target->GetX();
    float dy = monster->y - target->GetY();
    return std::sqrt(dx * dx + dy * dy);
}

void MonsterAI::MoveTowardsTarget(Monster* monster, float deltaTime) {
    if (monster->currentTargetId == 0) return;
    
    auto target = entityManager_.GetEntityById(monster->currentTargetId);
    if (!target) return;
    
    float targetX = target->GetX();
    float targetY = target->GetY();
    
    // Calculate direction
    float dx = targetX - monster->x;
    float dy = targetY - monster->y;
    float dist = std::sqrt(dx * dx + dy * dy);
    
    if (dist < 1.0f) return; // Already at target
    
    // Normalize and apply speed
    float moveSpeed = monster->moveSpeed * deltaTime;
    float newX = monster->x + (dx / dist) * moveSpeed;
    float newY = monster->y + (dy / dist) * moveSpeed;
    
    // Validate new position
    if (worldManager_.IsPositionValid(monster->mapId, newX, newY)) {
        monster->SetPosition(monster->mapId, newX, newY);
    }
}

void MonsterAI::PerformAttack(Monster* monster) {
    if (monster->currentTargetId == 0) return;
    
    auto target = entityManager_.GetEntityById(monster->currentTargetId);
    if (!target) return;
    
    // Use CombatSystem to deal damage
    // TODO: Implement actual attack logic with combat system
    Logger::Debug("Monster {} attacks target {}", monster->GetId(), monster->currentTargetId);
}

void MonsterAI::ResetToSpawn(Monster* monster) {
    // Move monster back to spawn point
    float dx = monster->spawnX - monster->x;
    float dy = monster->spawnY - monster->y;
    float dist = std::sqrt(dx * dx + dy * dy);
    
    if (dist < 1.0f) return; // Already at spawn
    
    // Teleport or move back (depending on distance)
    if (dist > 100.0f) {
        // Teleport if too far
        monster->SetPosition(monster->mapId, monster->spawnX, monster->spawnY);
    } else {
        // Move normally
        float moveSpeed = monster->moveSpeed * 2.0f; // Return faster
        float deltaTime = 0.5f; // Assume update interval
        
        float newX = monster->x + (dx / dist) * moveSpeed * deltaTime;
        float newY = monster->y + (dy / dist) * moveSpeed * deltaTime;
        
        monster->SetPosition(monster->mapId, newX, newY);
    }
}

} // namespace kal
