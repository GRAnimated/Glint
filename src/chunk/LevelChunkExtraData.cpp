#include "addons/ImGui/include/hk/gfx/ImGuiConfig.h"
#include "chunk/LevelChunkExtraData.h"

#include "Minecraft.Nbt/CompoundTag.h"
#include "Minecraft.Nbt/NbtIo.h"
#include "Minecraft.World/level/Level.h"
#include "Minecraft.World/level/chunk/storage/OldChunkStorage.h"
#include "Minecraft.Core/io/DataInputStream.h"
#include "Minecraft.Core/io/FileInputStream.h"

#include "hk/hook/InstrUtil.h"
#include "hk/hook/Trampoline.h"

std::unordered_map<LevelChunk*, LevelChunkExtraData> LevelChunkExtraData::s_extraDataMap;

LevelChunkExtraData::LevelChunkExtraData() : extraBlockDataLower(true), extraBlockDataUpper(true) {}

LevelChunkExtraData& LevelChunkExtraData::get(LevelChunk* chunk) {
    auto [it, inserted] = s_extraDataMap.try_emplace(chunk);  // C++17: emplace if not exists

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
    auto it = s_extraDataMap.find(chunk);
    if (it != s_extraDataMap.end()) {
        s_extraDataMap.erase(it);
    }
}

HkTrampoline<void, LevelChunk*, Level*, int, int> levelChunkInit
    = hk::hook::trampoline([](LevelChunk* chunk, Level* level, int x, int z) -> void {
          levelChunkInit.orig(chunk, level, x, z);

          hk::gfx::imguiLog("Initializing extra data for chunk at (%d, %d)\n", x, z);
          LevelChunkExtraData::get(chunk);  // initialize extra data
      });

// THESE ARE UNUSED BY THE GAME!!!
/*
HkTrampoline<void, OldChunkStorage*, LevelChunk*, Level*, CompoundTag*> chunkStorageSave
    = hk::hook::trampoline(
        [](OldChunkStorage* chunkStorage, LevelChunk* chunk, Level* level, CompoundTag* tag) -> void {
            chunkStorageSave.orig(chunkStorage, chunk, level, tag);

            hk::gfx::imguiLog("Saving extra data for chunk at (%d, %d)\n", chunk->xPos, chunk->zPos);
            LevelChunkExtraData::get(chunk).save(tag);
        });

*/

class DataFixerUpper;

HkTrampoline<void, Level*, DataInputStream*, DataFixerUpper*, LevelChunk*> chunkStorageLoad
    = hk::hook::trampoline(
        [](Level* level, DataInputStream* stream, DataFixerUpper* fixerUpper, LevelChunk* chunk) -> void {
            chunkStorageLoad.orig(level, stream, fixerUpper, chunk);

            hk::gfx::imguiLog("Loading extra data for chunk at (%d, %d)\n", chunk->xPos, chunk->zPos);

            LevelChunkExtraData::get(chunk).load(stream);
        });

HkTrampoline<void, LevelChunk*> levelChunkDtor = hk::hook::trampoline([](LevelChunk* chunk) -> void {
    levelChunkDtor.orig(chunk);
    LevelChunkExtraData::onLevelChunkDestroyed(chunk);
});

HkTrampoline<int, LevelChunk*, int, int, int> getBlockId
    = hk::hook::trampoline([](LevelChunk* chunk, int x, int y, int z) -> int {
          int baseId = getBlockId.orig(chunk, x, y, z);

          LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);
          int extraBits = (y < 128) ? extraData.extraBlockDataLower.get(x, y, z) :
                                      extraData.extraBlockDataUpper.get(x, y - 128, z);

          int fullId = (extraBits << 8) | baseId;
          return fullId;
      });

void setExtraBlockData(CompressedBlockStorage* storage, int x, int y, int z, int id) {
    int baseId = id & 0xFF;
    int extraBits = (id >> 8) & 0x0F;

    storage->set(x, y, z, baseId);

    LevelChunk* chunk;
    __asm volatile("MOV %0, X19" : "=r"(chunk));
    if (!chunk)
        return;

    LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);

    if (y < 128) {
        extraData.extraBlockDataLower.set(x, y, z, extraBits);
    } else {
        extraData.extraBlockDataUpper.set(x, y - 128, z, extraBits);
    }
}

LevelChunk* savedChunk;

HkTrampoline<void, LevelChunk*, Level*, DataOutputStream*> chunkStorageSaveNew
    = hk::hook::trampoline([](LevelChunk* chunk, Level* level, DataOutputStream* stream) -> void {
          hk::gfx::imguiLog("Storing chunk ptr at (%d, %d)\n", chunk->xPos, chunk->zPos);
          savedChunk = chunk;

          chunkStorageSaveNew.orig(chunk, level, stream);

          hk::gfx::imguiLog("Saving extra data for chunk at (%d, %d)\n", savedChunk->xPos, savedChunk->zPos);

          // LevelChunkExtraData::get(savedChunk).save(stream);
      });

void saveHook(CompoundTag* tag, DataOutput* out) {
    hk::gfx::imguiLog("Saving extra data for chunk at (%d, %d)\n", savedChunk->xPos, savedChunk->zPos);

    // The output stream gets casted into a DataOutput, so we need to get the DataOutputStream back
    DataOutputStream* outStream;
    __asm volatile("MOV %0, X24" : "=r"(outStream));

    LevelChunkExtraData::get(savedChunk).save(outStream);

    if (tag == nullptr) {
        hk::gfx::imguiLog("Tag is null\n");
    }

    if (out == nullptr) {
        hk::gfx::imguiLog("Output stream is null\n");
    }

    hk::gfx::imguiLog("Calling NbtIo::write now\n");
    NbtIo::write(tag, out);
}

void doNothing() {
    hk::gfx::imguiLog("Avoiding updateData call\n");
}

HkTrampoline<void, LevelChunk*, DataInputStream*> readCompressedBlockDataHook
    = hk::hook::trampoline([](LevelChunk* chunk, DataInputStream* stream) -> void {
          readCompressedBlockDataHook.orig(chunk, stream);

          hk::gfx::imguiLog("Reading extra data for chunk at (%d, %d)\n", chunk->xPos, chunk->zPos);

          LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);
          extraData.extraBlockDataLower.read(stream);
          extraData.extraBlockDataUpper.read(stream);
      });

HkTrampoline<void, LevelChunk*, DataOutputStream*> writeCompressedBlockDataHook
    = hk::hook::trampoline([](LevelChunk* chunk, DataOutputStream* stream) -> void {
          writeCompressedBlockDataHook.orig(chunk, stream);

          hk::gfx::imguiLog("Writing extra data for chunk at (%d, %d)\n", chunk->xPos, chunk->zPos);

          LevelChunkExtraData& extraData = LevelChunkExtraData::get(chunk);
          extraData.extraBlockDataLower.write(stream);
          extraData.extraBlockDataUpper.write(stream);
      });

void LevelChunkExtraData::initHooks() {
    // chunkStorageSave
    //     .installAtSym<"_ZN15OldChunkStorage4saveEP10LevelChunkP5LevelP11CompoundTag">();  // UNUSED
    // chunkStorageLoad
    //     .installAtSym<"_ZN15OldChunkStorage4loadEP5LevelP15DataInputStreamP14DataFixerUpperP10LevelChunk">();
    //  chunkStorageLoad2.installAtSym<"_ZN15OldChunkStorage4loadEP5LeveliiP10LevelChunk">();

    levelChunkInit.installAtSym<"_ZN10LevelChunk4initEP5Levelii">();
    levelChunkDtor.installAtSym<"_ZN10LevelChunkD1Ev">();

    getBlockId.installAtSym<"_ZN10LevelChunk10getBlockIdEiii">();

    // chunkStorageSaveNew.installAtSym<"_ZN15OldChunkStorage4saveEP10LevelChunkP5LevelP16DataOutputStream">();

    readCompressedBlockDataHook.installAtSym<"_ZN10LevelChunk23readCompressedBlockDataEP15DataInputStream">();
    writeCompressedBlockDataHook
        .installAtSym<"_ZN10LevelChunk24writeCompressedBlockDataEP16DataOutputStream">();

    const hk::ro::RoModule* main = hk::ro::getMainModule();
    hk::hook::writeBranchLink(main, 0x209A78, setExtraBlockData);  // setBlock
    hk::hook::writeBranchLink(main, 0x209FF8, setExtraBlockData);  // setBlockAndData

    // hk::hook::writeBranchLink(
    //     main, 0x2329FC,
    //     saveHook);  // NbtIo::write inside of
    //                 //  _ZN15OldChunkStorage4saveEP10LevelChunkP5LevelP16DataOutputStream

    // hk::hook::writeBranchLink(main, 0x2317D0, doNothing);  // Fuck you DataFixerUpper
}
