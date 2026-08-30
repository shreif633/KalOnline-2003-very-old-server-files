#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

namespace kal {

/**
 * @brief Binary KSM map file parser for KalOnline 2003
 * 
 * Parses the proprietary .ksm format to extract:
 * - Map dimensions and tile data
 * - Heightmap (Z-coordinates) for terrain following
 * - Tile attributes (walkable, water, death zones)
 * - Texture indices
 */
class KsmParser {
public:
    struct KsmHeader {
        uint32_t magic;         // Expected: 'KSM\x01' or similar
        uint32_t version;
        uint32_t width;         // Map width in tiles
        uint32_t height;        // Map height in tiles
        float tileSize;         // Size of each tile in world units
        uint32_t numLayers;     // Number of texture layers
    };

    struct TileData {
        uint8_t walkable;       // 1 = walkable, 0 = blocked
        uint8_t zoneType;       // 0 = normal, 1 = water, 2 = death, etc.
        uint16_t textureIndex;  // Index into texture array
        float height;           // Z-coordinate at this tile
    };

    struct MapData {
        std::string filename;
        KsmHeader header;
        std::vector<TileData> tiles;      // width * height tiles
        std::vector<std::string> textures;// Texture filenames per layer
        std::vector<float> heightmap;     // Flattened height data
    };

    /**
     * @brief Parse a .ksm file from disk
     * @param filepath Full path to the .ksm file
     * @return Parsed MapData if successful, nullopt on failure
     */
    static std::optional<MapData> ParseFile(const std::string& filepath);

    /**
     * @brief Parse KSM data from memory buffer
     * @param data Pointer to binary data
     * @param size Size of data in bytes
     * @return Parsed MapData if successful, nullopt on failure
     */
    static std::optional<MapData> ParseMemory(const uint8_t* data, size_t size);

    /**
     * @brief Check if a position is walkable on the map
     * @param map Parsed map data
     * @param x X coordinate in world space
     * @param y Y coordinate in world space
     * @return true if walkable, false otherwise
     */
    static bool IsPositionWalkable(const MapData& map, float x, float y);

    /**
     * @brief Get the height (Z) at a given position
     * @param map Parsed map data
     * @param x X coordinate in world space
     * @param y Y coordinate in world space
     * @return Z coordinate (height) at that position
     */
    static float GetHeightAt(const MapData& map, float x, float y);

private:
    static bool ValidateMagic(uint32_t magic);
    static MapData ReadLegacyFormat(const uint8_t* data, size_t size);
    static MapData ReadModernFormat(const uint8_t* data, size_t size);
};

} // namespace kal
