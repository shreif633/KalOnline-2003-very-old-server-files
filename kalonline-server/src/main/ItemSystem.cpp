#include "main/core/Entities.hpp"
#include "common/logger/Logger.h"
#include "common/database/Database.h"
#include <algorithm>
#include <random>

namespace kal::main {

struct ItemTemplate {
    uint32_t itemId;
    std::string name;
    uint8_t type; // 0:weapon, 1:armor, 2:accessory, 3:consumable, 4:material
    uint8_t subtype; // weapon: 0:sword, 1:axe, etc.
    uint8_t grade; // 0:common, 1:rare, 2:epic, 3:legendary
    uint16_t minLevel;
    uint16_t requiredSTR;
    uint16_t requiredHTH;
    uint16_t requiredINT;
    uint16_t requiredWIS;
    uint16_t requiredDEX;
    uint16_t attackPower;
    uint16_t defense;
    uint16_t magicAttack;
    uint16_t magicDefense;
    uint16_t accuracy;
    uint16_t evasion;
    uint32_t maxDurability;
    bool isStackable;
    uint16_t stackSize;
    uint32_t basePrice;
    float dropRate; // 0.0-1.0
};

struct PlayerItem {
    uint64_t uniqueId;
    uint32_t itemId;
    uint16_t count;
    int8_t slot; // -1: not in inventory
    uint8_t location; // 0:inventory, 1:equipped, 2:warehouse
    uint32_t prefixId;
    uint8_t quality; // 0-100
    uint32_t currentDurability;
    uint64_t socketFlags;
    std::array<uint32_t, 4> gems;
    uint64_t expireTime; // 0: permanent
};

class ItemSystemImpl {
private:
    std::unordered_map<uint32_t, ItemTemplate> itemTemplates_;
    std::unordered_map<int, std::vector<uint32_t>> dropTables_; // mapId -> itemIds
    mutable std::shared_mutex mutex_;

public:
    ItemSystemImpl() {
        Logger::Info("ItemSystem initialized");
        LoadItemTemplates();
        LoadDropTables();
    }

    ~ItemSystemImpl() {
        Logger::Info("ItemSystem destroyed");
    }

    void LoadItemTemplates() {
        try {
            auto db = Database::GetInstance();
            auto result = db->ExecuteQuery(
                "SELECT id, name, type, subtype, grade, min_level, req_str, req_hth, req_int, "
                "req_wis, req_dex, attack, defense, magic_atk, magic_def, accuracy, evasion, "
                "max_durability, is_stackable, stack_size, base_price, drop_rate "
                "FROM items ORDER BY id"
            );
            
            while (result && result->Next()) {
                ItemTemplate item;
                item.itemId = result->GetInt(0);
                item.name = result->GetString(1);
                item.type = result->GetInt(2);
                item.subtype = result->GetInt(3);
                item.grade = result->GetInt(4);
                item.minLevel = result->GetInt(5);
                item.requiredSTR = result->GetInt(6);
                item.requiredHTH = result->GetInt(7);
                item.requiredINT = result->GetInt(8);
                item.requiredWIS = result->GetInt(9);
                item.requiredDEX = result->GetInt(10);
                item.attackPower = result->GetInt(11);
                item.defense = result->GetInt(12);
                item.magicAttack = result->GetInt(13);
                item.magicDefense = result->GetInt(14);
                item.accuracy = result->GetInt(15);
                item.evasion = result->GetInt(16);
                item.maxDurability = result->GetInt(17);
                item.isStackable = result->GetInt(18);
                item.stackSize = result->GetInt(19);
                item.basePrice = result->GetInt(20);
                item.dropRate = result->GetFloat(21);
                
                itemTemplates_[item.itemId] = item;
            }
            
            Logger::Info("Loaded {} item templates", itemTemplates_.size());
        } catch (const std::exception& e) {
            Logger::Error("Failed to load item templates: {}", e.what());
        }
    }

    void LoadDropTables() {
        try {
            auto db = Database::GetInstance();
            auto result = db->ExecuteQuery(
                "SELECT map_id, item_id FROM drop_tables ORDER BY map_id, rarity"
            );
            
            while (result && result->Next()) {
                int mapId = result->GetInt(0);
                uint32_t itemId = result->GetInt(1);
                dropTables_[mapId].push_back(itemId);
            }
            
            Logger::Info("Loaded drop tables for {} maps", dropTables_.size());
        } catch (const std::exception& e) {
            Logger::Error("Failed to load drop tables: {}", e.what());
        }
    }

    const ItemTemplate* GetItemTemplate(uint32_t itemId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = itemTemplates_.find(itemId);
        return it != itemTemplates_.end() ? &it->second : nullptr;
    }

    std::vector<PlayerItem> GenerateRandomDrop(int mapId, uint8_t monsterLevel) {
        std::vector<PlayerItem> drops;
        
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = dropTables_.find(mapId);
        if (it == dropTables_.end()) {
            return drops;
        }

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dropRoll(0.0, 1.0);

        for (uint32_t itemId : it->second) {
            const ItemTemplate* tmpl = GetItemTemplate(itemId);
            if (!tmpl || tmpl->dropRate <= 0.0) continue;

            if (dropRoll(gen) <= tmpl->dropRate) {
                PlayerItem drop;
                drop.uniqueId = generateUniqueId();
                drop.itemId = itemId;
                drop.count = tmpl->isStackable ? 
                    static_cast<uint16_t>(std::uniform_int_distribution<>(1, tmpl->stackSize)(gen)) : 1;
                drop.slot = -1;
                drop.location = 0; // inventory
                drop.prefixId = 0;
                drop.quality = 100;
                drop.currentDurability = tmpl->maxDurability;
                drop.socketFlags = 0;
                drop.gems.fill(0);
                drop.expireTime = 0;
                
                drops.push_back(drop);
            }
        }

        return drops;
    }

    bool CanEquip(const Character& character, uint32_t itemId) const {
        const ItemTemplate* tmpl = GetItemTemplate(itemId);
        if (!tmpl) return false;

        if (character.level() < tmpl->minLevel) {
            Logger::Debug("Level requirement not met: {} < {}", character.level(), tmpl->minLevel);
            return false;
        }

        if (tmpl->requiredSTR > 0) {
            // TODO: Check character STR
        }
        if (tmpl->requiredHTH > 0) {
            // TODO: Check character HTH
        }
        if (tmpl->requiredINT > 0) {
            // TODO: Check character INT
        }
        if (tmpl->requiredWIS > 0) {
            // TODO: Check character WIS
        }
        if (tmpl->requiredDEX > 0) {
            // TODO: Check character DEX
        }

        return true;
    }

    uint64_t generateUniqueId() const {
        static std::atomic<uint64_t> counter{1};
        return counter.fetch_add(1);
    }
};

// Global instance
static std::unique_ptr<ItemSystemImpl> g_itemSystem;

void InitializeItemSystem() {
    g_itemSystem = std::make_unique<ItemSystemImpl>();
}

void ShutdownItemSystem() {
    g_itemSystem.reset();
}

const ItemTemplate* GetItemTemplate(uint32_t itemId) {
    return g_itemSystem ? g_itemSystem->GetItemTemplate(itemId) : nullptr;
}

std::vector<PlayerItem> GenerateDrop(int mapId, uint8_t monsterLevel) {
    return g_itemSystem ? g_itemSystem->GenerateRandomDrop(mapId, monsterLevel) : std::vector<PlayerItem>{};
}

bool CanEquipItem(const Character& character, uint32_t itemId) {
    return g_itemSystem ? g_itemSystem->CanEquip(character, itemId) : false;
}

} // namespace kal::main
