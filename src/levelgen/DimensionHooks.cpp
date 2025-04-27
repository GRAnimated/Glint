#include "levelgen/DimensionHooks.h"
#include "levelgen/SkyGridLevelSource.h"

#include "Minecraft.World/level/Level.h"
#include "Minecraft.World/level/dimension/Dimension.h"
#include "Minecraft.World/level/storage/LevelData.h"

#include "hk/hook/InstrUtil.h"
#include "hk/hook/Trampoline.h"

bool returnFalse() {
    return false;
}

HkTrampoline<ChunkGenerator*, Dimension*> createLevelGenerator
    = hk::hook::trampoline([](Dimension* dimension) -> ChunkGenerator* {
          return new SkyGridLevelSource(dimension->mLevel,
                                        dimension->mLevel->getLevelData()->isGenerateMapFeatures(),
                                        dimension->mLevel->getSeed());
          // return new OverworldLevelSource(dimension->mLevel, dimension->mLevel->getSeed(),
          //                                 dimension->mLevel->getLevelData()->isGenerateMapFeatures(),
          //                                 nullptr);
      });

void DimensionHooks::initHooks() {
    // createLevelGenerator.installAtSym<"_ZNK9Dimension26createRandomLevelGeneratorEv">();

    // const hk::ro::RoModule* main = hk::ro::getMainModule();
    // hk::hook::writeBranchLink(main, 0x206B04, returnFalse);  // required for custom level source
}
