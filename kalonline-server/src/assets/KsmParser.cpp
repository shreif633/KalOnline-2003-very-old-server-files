#include "KsmParser.h"
#include "Logger.h"
#include <fstream>
#include <cstring>
#include <optional>

namespace kal {

bool KsmParser::ValidateMagic(uint32_t magic) {
    // KalOnline KSM files typically start with 'KSM\x01' or similar magic bytes
    // Adjust based on actual decompilation findings
    return (magic == 0x014D534B); // 'KSM\x01' in little-endian
}

std::optional<KsmParser::MapData> KsmParser::ParseFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Logger::Error("Failed to open KSM file: {}", filepath);
        return std::nullopt;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        Logger::Error("Failed to read KSM file: {}", filepath);
        return std::nullopt;
    }

    file.close();
    return ParseMemory(buffer.data(), buffer.size());
}

std::optional<KsmParser::MapData> KsmParser::ParseMemory(const uint8_t* data, size_t size) {
    if (size < sizeof(KsmHeader)) {
        Logger::Error("KSM file too small");
        return std::nullopt;
    }

    // Read and validate magic number
    uint32_t magic = *reinterpret_cast<const uint32_t*>(data);
    if (!ValidateMagic(magic)) {
        Logger::Warn("Invalid KSM magic number: 0x{:08X}", magic);
        // Try to parse anyway as some maps might have different signatures
    }

    // Determine format version and parse accordingly
    uint32_t version = *reinterpret_cast<const uint32_t*>(data + 4);
    
    if (version <= 2) {
        Logger::Info("Parsing legacy KSM format (v{})", version);
        return ReadLegacyFormat(data, size);
    } else {
        Logger::Info("Parsing modern KSM format (v{})", version);
        return ReadModernFormat(data, size);
    }
}

KsmParser::MapData KsmParser::ReadLegacyFormat(const uint8_t* data, size_t size) {
    MapData map;
    size_t offset = 0;

    // Read header (legacy format: 20 bytes)
    std::memcpy(&map.header.magic, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.version, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.width, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.height, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.tileSize, data + offset, sizeof(float)); offset += 4;
    map.header.numLayers = 1; // Legacy format has single layer

    Logger::Info("KSM Map: {}x{} tiles, tile size: {:.2f}", 
                 map.header.width, map.header.height, map.header.tileSize);

    // Calculate total tiles
    size_t numTiles = map.header.width * map.header.height;
    map.tiles.resize(numTiles);
    map.heightmap.resize(numTiles);

    // Read heightmap (float per tile)
    for (size_t i = 0; i < numTiles; ++i) {
        std::memcpy(&map.heightmap[i], data + offset, sizeof(float));
        offset += sizeof(float);
    }

    // Read tile attributes (walkable/zone type per tile)
    for (size_t i = 0; i < numTiles; ++i) {
        map.tiles[i].height = map.heightmap[i];
        map.tiles[i].walkable = data[offset++];
        map.tiles[i].zoneType = data[offset++];
        map.tiles[i].textureIndex = 0; // Legacy format doesn't have texture indices
    }

    Logger::Info("Loaded {} tiles from legacy KSM", numTiles);
    return map;
}

KsmParser::MapData KsmParser::ReadModernFormat(const uint8_t* data, size_t size) {
    MapData map;
    size_t offset = 0;

    // Read header (modern format: 24+ bytes)
    std::memcpy(&map.header.magic, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.version, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.width, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.height, data + offset, sizeof(uint32_t)); offset += 4;
    std::memcpy(&map.header.tileSize, data + offset, sizeof(float)); offset += 4;
    std::memcpy(&map.header.numLayers, data + offset, sizeof(uint32_t)); offset += 4;

    Logger::Info("KSM Map: {}x{} tiles, {} layers, tile size: {:.2f}", 
                 map.header.width, map.header.height, map.header.numLayers, map.header.tileSize);

    // Read texture names per layer
    for (uint32_t layer = 0; layer < map.header.numLayers; ++layer) {
        char texName[64] = {0};
        std::strncpy(texName, reinterpret_cast<const char*>(data + offset), 63);
        offset += 64;
        map.textures.push_back(std::string(texName));
    }

    // Calculate total tiles
    size_t numTiles = map.header.width * map.header.height;
    map.tiles.resize(numTiles);
    map.heightmap.resize(numTiles);

    // Modern format: Heightmap first (float per tile)
    for (size_t i = 0; i < numTiles; ++i) {
        std::memcpy(&map.heightmap[i], data + offset, sizeof(float));
        offset += sizeof(float);
    }

    // Tile attributes structure (8 bytes per tile in modern format)
    struct ModernTile {
        uint8_t walkable;
        uint8_t zoneType;
        uint16_t textureIndex;
        float reserved; // Padding
    };
    static_assert(sizeof(ModernTile) == 8, "ModernTile must be 8 bytes");

    for (size_t i = 0; i < numTiles; ++i) {
        ModernTile tile;
        std::memcpy(&tile, data + offset, sizeof(ModernTile));
        offset += sizeof(ModernTile);

        map.tiles[i].walkable = tile.walkable;
        map.tiles[i].zoneType = tile.zoneType;
        map.tiles[i].textureIndex = tile.textureIndex;
        map.tiles[i].height = map.heightmap[i];
    }

    Logger::Info("Loaded {} tiles from modern KSM", numTiles);
    return map;
}

bool KsmParser::IsPositionWalkable(const MapData& map, float x, float y) {
    if (map.header.width == 0 || map.header.height == 0) {
        return false;
    }

    // Convert world coordinates to tile coordinates
    int tileX = static_cast<int>(x / map.header.tileSize);
    int tileY = static_cast<int>(y / map.header.tileSize);

    // Bounds check
    if (tileX < 0 || tileX >= static_cast<int>(map.header.width) ||
        tileY < 0 || tileY >= static_cast<int>(map.header.height)) {
        return false;
    }

    size_t tileIndex = tileY * map.header.width + tileX;
    if (tileIndex >= map.tiles.size()) {
        return false;
    }

    return map.tiles[tileIndex].walkable != 0;
}

float KsmParser::GetHeightAt(const MapData& map, float x, float y) {
    if (map.header.width == 0 || map.header.height == 0 || map.heightmap.empty()) {
        return 0.0f;
    }

    // Convert world coordinates to tile coordinates
    int tileX = static_cast<int>(x / map.header.tileSize);
    int tileY = static_cast<int>(y / map.header.tileSize);

    // Clamp to bounds
    tileX = std::clamp(tileX, 0, static_cast<int>(map.header.width) - 1);
    tileY = std::clamp(tileY, 0, static_cast<int>(map.header.height) - 1);

    size_t tileIndex = tileY * map.header.width + tileX;
    if (tileIndex >= map.heightmap.size()) {
        return 0.0f;
    }

    return map.heightmap[tileIndex];
}

} // namespace kal
