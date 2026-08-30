#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <regex>
#include "Logger.h"

// Item Template Structure
struct ItemTemplate {
    int index;                  // Item ID
    std::string name;           // Display name ID (from LangEN.txt)
    std::string image;          // Sprite name
    int actionType;             // Animation type
    int actionValue;            // Animation value
    std::string itemClass;      // e.g., "weapon sword", "defense upperarmor"
    std::vector<int> code;      // [4, 11, 0, 0] - category codes
    std::vector<int> country;   // [0, 1, 2] - faction availability
    int level;                  // Required level
    int wearLocation;           // Equip slot
    std::string limitClass;     // "Knight", "Archer", etc.
    int range;                  // Attack range
    int buyPrice;               // NPC buy price
    int sellPrice;              // NPC sell price
    int endurance;              // Max durability
    
    // Weapon stats
    int attackSpeed;
    int minAttack;
    int maxAttack;
    int hitRate;
    
    // Defense stats
    int defense;
    int dodge;
    int absorb;
    
    // Special properties
    int useType;                // For consumables
    int coolTime;               // Cooldown in ms
    int effectId;               // Effect when used
    int refreshHP;              // HP restored
    int refreshMP;              // MP restored
    
    // Class restrictions
    bool isKnightOnly;
    bool isArcherOnly;
};

// Monster Template Structure
struct MonsterTemplate {
    int index;
    std::string name;           // Name ID from LangEN.txt
    std::vector<int> country;   // Faction
    int race;                   // Monster type
    int level;
    int aiType;                 // AI behavior pattern
    int range;                  // Melee range
    std::pair<int, int> sight;  // [min, max] detection range
    int exp;                    // Experience reward
    int itemGroup;              // Drop table ID
    int itemGroupChance;        // Drop chance modifier
    
    // Stats
    int str, hth, int_, wis, dex;
    int hp, mp;
    int attackSpeed;
    int hit, dodge;
    int minAttack, maxAttack, magicAttack;
    int minDefense, maxDefense;
    int absorb;
    std::pair<int, int> moveSpeed; // [walk, run]
    std::vector<int> resist;    // Elemental resistances
    
    // Quest associations
    std::vector<std::tuple<int, int, int, int>> questDrops; // (questId, type, itemId, count)
};

// NPC Template Structure
struct NPCTemplate {
    int index;
    std::vector<int> country;
    int kind;                   // NPC type (0=normal, 1=shop, 2=service, 3=quest)
    int shape;                  // Sprite/model ID
    int htmlId;                 // Dialog text ID
    int mapId;
    std::pair<int, int> position; // X, Y coordinates
    int z;                      // Z height
    std::pair<int, int> direction; // Facing direction
    int questId;                // Associated quest ID
    int cid;                    // Castle ID (for castle NPCs)
    int warRelation;            // War status
};

// Skill Template Structure
struct SkillTemplate {
    int classId;                // 0=Knight, 1=Archer, etc.
    int index;
    int redistribute;           // Can be redistributed?
    std::vector<int> limit;     // [level, speciality, ...] requirements
    int maxLevel;
    int mpCost;
    int lastTime;               // Duration in ms
    std::vector<int> delay;     // [cast, cooldown, ...]
    int value1, value2;         // Skill-specific values
    std::string description;    // Skill name/description ID
};

// Quest Data Structure
struct QuestData {
    int index;
    int step;                   // Quest step number
    struct Condition {
        int requiredClass;
        int requiredLevel;
        int requiredSpecialty;
        int requiredContribute;
        std::vector<std::pair<int, int>> requiredItems; // (itemId, count)
        int requiredParty;
        std::pair<int, int> requiredQuest; // (questId, step)
        int clearCheck;
    };
    struct Reward {
        int htmlId;               // Completion dialog
        int guide;                // Guide flag
        std::vector<std::tuple<int, int, int>> giveItems; // (itemId, ?, count)
        int exp;
        int contribute;
        int suPoint;
        int saveType;             // Save method
        std::pair<int, int> clear; // (questId, step) to clear
        int linked;               // Linked quest flag
    };
    Condition condition;
    Reward reward;
};

// Goods List (Shop Inventory)
struct GoodsList {
    int index;
    std::vector<std::tuple<int, int, int>> items; // (itemId, ?, count)
};

// Item Group (Drop Table)
struct ItemGroup {
    int index;
    std::pair<int, int> moneyDrop; // (itemId, chance)
    std::vector<std::tuple<int, int, int>> items; // (itemId, chance, ?)
};

// GenMonster Spawn Point
struct GenMonsterSpawn {
    int index;                  // Monster template ID
    int mapId;
    int area;                   // Spawn area ID
    int maxCount;               // Max monsters alive
    int cycle;                  // Respawn time in seconds
    std::tuple<int, int, int, int> rect; // (minX, maxX, minY, maxY)
};

// Teleporter Data
struct TeleporterData {
    int id;
    int targetMapId;
    int sourceMapId;
    std::tuple<int, int, int> sourcePos; // X, Y, Z
    std::tuple<int, int, int> targetPos; // Not always specified
};

// Prefix Data (Item enchantments)
struct PrefixData {
    int index;
    std::string name;           // Prefix name ID
    int grade;                  // Quality level
    int optionType;             // Stat type modified
    int optionValue;            // Stat value
    std::vector<int> applyTo;   // Item types this applies to
};

class AssetLoader {
private:
    std::string configPath_;
    
    // Parsed data containers
    std::unordered_map<int, std::shared_ptr<ItemTemplate>> items_;
    std::unordered_map<int, std::shared_ptr<MonsterTemplate>> monsters_;
    std::unordered_map<int, std::shared_ptr<NPCTemplate>> npcs_;
    std::unordered_map<int, std::shared_ptr<SkillTemplate>> skills_;
    std::unordered_map<int, std::shared_ptr<QuestData>> quests_;
    std::unordered_map<int, std::shared_ptr<GoodsList>> goods_;
    std::unordered_map<int, std::shared_ptr<ItemGroup>> itemGroups_;
    std::vector<std::shared_ptr<GenMonsterSpawn>> genMonsterSpawns_;
    std::vector<std::shared_ptr<TeleporterData>> teleporters_;
    std::unordered_map<int, std::shared_ptr<PrefixData>> prefixes_;
    
    // Helper functions for parsing S-expression-like format
    std::string trim(const std::string& str);
    std::vector<std::string> tokenize(const std::string& line);
    std::string extractValue(const std::string& line, const std::string& key);
    std::vector<int> extractIntArray(const std::string& line, const std::string& key);
    std::pair<int, int> extractIntPair(const std::string& line, const std::string& key);
    std::tuple<int, int, int> extractIntTriple(const std::string& line, const std::string& key);
    std::tuple<int, int, int, int> extractIntQuad(const std::string& line, const std::string& key);
    std::string extractQuotedString(const std::string& line, const std::string& key);
    
public:
    explicit AssetLoader(const std::string& configPath);
    ~AssetLoader() = default;

    // Load all configuration files
    bool LoadAll();
    
    // Individual file loaders
    bool LoadInitItem();
    bool LoadInitMonster();
    bool LoadInitNPC();
    bool LoadInitSkill();
    bool LoadQuest();
    bool LoadGoods();
    bool LoadItemGroup();
    bool LoadGenMonster();
    bool LoadEtc(); // Teleporters
    bool LoadPrefix();
    
    // Getters for parsed data
    const std::unordered_map<int, std::shared_ptr<ItemTemplate>>& GetItems() const { return items_; }
    const std::unordered_map<int, std::shared_ptr<MonsterTemplate>>& GetMonsters() const { return monsters_; }
    const std::unordered_map<int, std::shared_ptr<NPCTemplate>>& GetNPCs() const { return npcs_; }
    const std::unordered_map<int, std::shared_ptr<SkillTemplate>>& GetSkills() const { return skills_; }
    const std::unordered_map<int, std::shared_ptr<QuestData>>& GetQuests() const { return quests_; }
    const std::unordered_map<int, std::shared_ptr<GoodsList>>& GetGoods() const { return goods_; }
    const std::unordered_map<int, std::shared_ptr<ItemGroup>>& GetItemGroups() const { return itemGroups_; }
    const std::vector<std::shared_ptr<GenMonsterSpawn>>& GetGenMonsterSpawns() const { return genMonsterSpawns_; }
    const std::vector<std::shared_ptr<TeleporterData>>& GetTeleporters() const { return teleporters_; }
    const std::unordered_map<int, std::shared_ptr<PrefixData>>& GetPrefixes() const { return prefixes_; }
    
    // Lookup helpers
    const ItemTemplate* GetItem(int index) const;
    const MonsterTemplate* GetMonster(int index) const;
    const NPCTemplate* GetNPC(int index) const;
    const SkillTemplate* GetSkill(int classId, int index) const;
};
