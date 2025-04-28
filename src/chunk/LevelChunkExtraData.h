#pragma once

#include "Minecraft.World/ArrayWithLength.h"
#include "Minecraft.World/level/chunk/storage/CompressedBlockStorage.h"
#include <unordered_map>

class CompoundTag;
class LevelChunk;

class LevelChunkExtraData {
public:
    CompressedBlockStorage extraBlockDataLower;
    CompressedBlockStorage extraBlockDataUpper;

    LevelChunkExtraData();

    static LevelChunkExtraData& get(LevelChunk* chunk);

    static void onLevelChunkDestroyed(LevelChunk* chunk);

    static void initHooks();

    void save(DataOutputStream* stream);
    void load(DataInputStream* stream);

private:
    static std::unordered_map<LevelChunk*, LevelChunkExtraData> s_extraDataMap;
};
