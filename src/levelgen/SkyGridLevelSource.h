#pragma once

#include "Minecraft.World/level/levelgen/ChunkGenerator.h"
#include "Minecraft.World/level/levelgen/GenericOverworldLevelSource.h"

class Random;
class ChunkPrimer;

class SkyGridLevelSource : public ChunkGenerator, public GenericOverworldLevelSource {
public:
    SkyGridLevelSource(Level*, bool, long long);
    ~SkyGridLevelSource() override;
    LevelChunk* createChunk(int, int) override;
    void postProcess(int, int) override;
    bool postProcessLoadedChunk(LevelChunk*, int, int) override;
    void getMobsAt(MobCategory*, BlockPos const&) override;
    void* findNearestMapFeature(Level*, std::wstring const&, BlockPos const&, bool) override;
    void recreateLogicStructuresForChunk(LevelChunk*, int, int) override;
    bool isPosInFeature(Level*, std::wstring const&, BlockPos const&) override;
    void lightChunk(LevelChunk*) override;
    void prepareHeights(int, int, ChunkPrimer*) override;
    void buildSurfaces(int, int, ChunkPrimer*);

    int field_8;
    int mSize;
    Random* mSeed;
    Random* mRandom;
    bool mIsGenerateMapFeatures;
    Level* mLevel;
};
