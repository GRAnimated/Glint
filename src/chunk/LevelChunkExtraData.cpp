#include "addons/ImGui/include/hk/gfx/ImGuiConfig.h"
#include "chunk/LevelChunkExtraData.h"

#include "Minecraft.Nbt/CompoundTag.h"
#include "Minecraft.Nbt/NbtIo.h"
#include "Minecraft.World/level/Level.h"
#include "Minecraft.World/level/chunk/storage/OldChunkStorage.h"
#include "Minecraft.Core/io/DataInputStream.h"
#include "Minecraft.Core/io/FileInputStream.h"
#include "block/CustomBlocks.h"

#include "hk/hook/InstrUtil.h"
#include "hk/hook/Trampoline.h"

std::unordered_map<LevelChunk*, LevelChunkExtraData> LevelChunkExtraData::sExtraDataMap;

LevelChunkExtraData::LevelChunkExtraData() : extraBlockDataLower(true), extraBlockDataUpper(true) {}

LevelChunkExtraData& LevelChunkExtraData::get(LevelChunk* chunk) {
    auto [it, inserted] = sExtraDataMap.try_emplace(chunk);

    return it->second;
}

void LevelChunkExtraData::save(DataOutputStream* stream) {
    if (!stream)
        return;

    extraBlockDataLower.write(stream);
    extraBlockDataUpper.write(stream);
}

void LevelChunkExtraData::load(DataInputStream* stream) {
    if (!stream)
        return;

    extraBlockDataLower.read(stream);
    extraBlockDataUpper.read(stream);
}

void LevelChunkExtraData::onLevelChunkDestroyed(LevelChunk* chunk) {
    auto it = sExtraDataMap.find(chunk);
    if (it != sExtraDataMap.end()) {
        sExtraDataMap.erase(it);
    }
}

HkTrampoline<void, LevelChunk*, Level*, int, int> levelChunkInit
    = hk::hook::trampoline([](LevelChunk* chunk, Level* level, int x, int z) -> void {
          levelChunkInit.orig(chunk, level, x, z);

          hk::gfx::imguiLog("Initializing extra data for chunk at (%d, %d)\n", x, z);
          LevelChunkExtraData::get(chunk);  // initialize extra data
      });

HkTrampoline<void, LevelChunk*> levelChunkDtor = hk::hook::trampoline([](LevelChunk* chunk) -> void {
    levelChunkDtor.orig(chunk);
    LevelChunkExtraData::onLevelChunkDestroyed(chunk);
});

HkTrampoline<int, LevelChunk*, int, int, int> getBlockId = hk::hook::trampoline([](LevelChunk* chunk, int x,
                                                                                   int y, int z) -> int {
    int baseId = getBlockId.orig(chunk, x, y, z);

    LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);
    int extraBits = (y < 128) ? extraData.extraBlockDataLower.get(x, y, z) :
                                extraData.extraBlockDataUpper.get(x, y - 128, z);

    int fullId = (extraBits << 8) | baseId;

    if (fullId > 255)
        hk::gfx::imguiLog("Getting block ID at (%d, %d, %d) with base ID: %d, extra bits: %d, full ID: %d\n",
                          x, y, z, baseId, extraBits, fullId);

    return fullId;
});

int storedY = 0;

HkTrampoline<void, LevelChunk*, int, int, int, int, int, bool> setBlockAndData
    = hk::hook::trampoline([](LevelChunk* chunk, int x, int y, int z, int id, int data, bool flag) -> void {
          storedY = y;

          if (id > 255) {
              hk::gfx::imguiLog("Setting block and data at (%d, %d, %d) with ID: %d, data: %d\n", x, y, z, id,
                                data);
          }
          setBlockAndData.orig(chunk, x, y, z, id, data, flag);
      });

void setExtraBlockData(CompressedBlockStorage* storage, int x, int y, int z, int id) {
    int baseId = id & 0xFF;
    int extraBits = (id >> 8) & 0x0F;

    // if (extraBits > 0) {
    //     hk::gfx::imguiLog("Setting extra block data at (%d, %d, %d) with ID: %d\n", x, y, z, id);
    // }

    storage->set(x, y, z, baseId);

    // Chunk exists in X19
    LevelChunk* chunk;
    __asm volatile("MOV %0, X19" : "=r"(chunk));
    if (!chunk) {
        hk::gfx::imguiLog("Chunk is null, cannot set extra block data\n");
        return;
    }

    LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);

    if (extraBits == 0) {
        // We need to clear the extra bits
        if (storedY < 128) {
            extraData.extraBlockDataLower.set(x, storedY, z, 0);
        } else {
            extraData.extraBlockDataUpper.set(x, storedY - 128, z, 0);
        }
        return;
    }

    if (storedY < 128) {
        int prev = extraData.extraBlockDataLower.get(x, storedY, z);
        if (prev)
            hk::gfx::imguiLog("Previous extra data at (%d, %d, %d): %d\n", x, storedY, z, prev);
        extraData.extraBlockDataLower.set(x, storedY, z, extraBits);
    } else {
        int prev = extraData.extraBlockDataUpper.get(x, storedY, z);
        if (prev)
            hk::gfx::imguiLog("Previous extra data at (%d, %d, %d): %d\n", x, storedY, z, prev);
        extraData.extraBlockDataUpper.set(x, storedY - 128, z, extraBits);
    }
}

HkTrampoline<void, LevelChunk*, DataInputStream*> readCompressedBlockDataHook
    = hk::hook::trampoline([](LevelChunk* chunk, DataInputStream* stream) -> void {
          readCompressedBlockDataHook.orig(chunk, stream);

          // hk::gfx::imguiLog("Reading extra data for chunk at (%d, %d)\n", chunk->xPos, chunk->zPos);

          LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);
          extraData.extraBlockDataLower.read(stream);
          extraData.extraBlockDataUpper.read(stream);
      });

HkTrampoline<void, LevelChunk*, DataOutputStream*> writeCompressedBlockDataHook
    = hk::hook::trampoline([](LevelChunk* chunk, DataOutputStream* stream) -> void {
          writeCompressedBlockDataHook.orig(chunk, stream);

          // hk::gfx::imguiLog("Writing extra data for chunk at (%d, %d)\n", chunk->xPos, chunk->zPos);

          LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);
          extraData.extraBlockDataLower.write(stream);
          extraData.extraBlockDataUpper.write(stream);
      });

HkTrampoline<Block*, int> blockById = hk::hook::trampoline([](int id) -> Block* {
    Block* block = blockById.orig(id);

    if (block)
        return block;

    hk::gfx::imguiLog("Block not found for ID, checking custom case: %d\n", id);
    if (id == 3000) {
        return CustomBlocks::CUSTOM_BLOCK;
    }
    return nullptr;
});

void LevelChunkExtraData::initHooks() {
    levelChunkInit.installAtSym<"_ZN10LevelChunk4initEP5Levelii">();
    levelChunkDtor.installAtSym<"_ZN10LevelChunkD1Ev">();

    getBlockId.installAtSym<"_ZN10LevelChunk10getBlockIdEiii">();

    readCompressedBlockDataHook.installAtSym<"_ZN10LevelChunk23readCompressedBlockDataEP15DataInputStream">();
    writeCompressedBlockDataHook
        .installAtSym<"_ZN10LevelChunk24writeCompressedBlockDataEP16DataOutputStream">();

    setBlockAndData.installAtSym<"_ZN10LevelChunk15setBlockAndDataEiiiiib">();

    blockById.installAtSym<"_ZN5Block4byIdEi">();

    const hk::ro::RoModule* main = hk::ro::getMainModule();
    hk::hook::writeBranchLink(main, 0x209A78, setExtraBlockData);  // setBlock
    hk::hook::writeBranchLink(main, 0x209FF8, setExtraBlockData);  // setBlockAndData
}
