#pragma once

#include "core/Entities.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

struct ItemTemplate {
    int id;
    std::string name;
    int type; // 0=Weapon, 1=Armor, 2=Accessory, 3=Consumable, 4=Quest
    int subtype; // Specific type (sword, bow, helmet, etc.)
    int minLevel;
    int requiredStat; // STR/HTH/INT/WIS/DEX requirement
    int damageMin;
    int damageMax;
    int defense;
    int durability;
    int maxDurability;
    int price; // Vendor price
    float dropRate; // Base drop rate modifier
    bool isTradeable;
    bool isStackable;
    int maxStack;
    std::vector<int> bonuses; // Stat bonuses
};

class ItemSystem {
public:
    ItemSystem();
    ~ItemSystem();

    // Initialization
    bool LoadItemTemplates();
    
    // Item Creation
    std::shared_ptr<Item> CreateItem(int templateId, int quantity = 1);
    std::shared_ptr<Item> CreateRandomItem(int levelRange, const std::string& category = "");
    
    // Inventory Management
    bool AddItemToInventory(std::shared_ptr<Character> character, std::shared_ptr<Item> item);
    bool RemoveItemFromInventory(std::shared_ptr<Character> character, uint32_t itemId);
    bool MoveItem(std::shared_ptr<Character> character, uint32_t itemId, int slot);
    bool EquipItem(std::shared_ptr<Character> character, uint32_t itemId);
    bool UnequipItem(std::shared_ptr<Character> character, int equipmentSlot);
    
    // Validation
    bool CanEquip(const Character& character, const Item& item) const;
    bool CanUseItem(const Character& character, const Item& item) const;
    
    // Drop System
    std::vector<std::shared_ptr<Item>> GenerateDropTable(int monsterLevel, int monsterType);
    std::shared_ptr<Item> RollDrop(int templateId, float rateMultiplier = 1.0f);
    
    // Accessors
    const ItemTemplate* GetItemTemplate(int templateId) const;
    std::vector<ItemTemplate> GetItemsByType(int type) const;
    std::vector<ItemTemplate> GetItemsByLevelRange(int minLevel, int maxLevel) const;

private:
    mutable std::mutex itemMutex_;
    std::unordered_map<int, ItemTemplate> itemDatabase_;
    std::unordered_map<int, std::vector<int>> itemsByType_; // type -> [templateIds]
    std::unordered_map<int, std::vector<int>> itemsByLevel_; // level -> [templateIds]
    
    uint32_t nextItemId_ = 1;
    
    void GeneratePrefixes(std::shared_ptr<Item> item);
    void CalculateItemStats(std::shared_ptr<Item> item);
};
