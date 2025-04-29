#include "addons/ImGui/include/hk/gfx/ImGuiConfig.h"
#include "block/BlockHooks.h"
#include "block/CustomBlocks.h"

#include "Minecraft.Client/resources/MappedRegistry.h"
#include "Minecraft.Client/resources/ResourceLocation.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/Trampoline.h"

#include "Minecraft.Client/gui/MenuBuilder.h"

HkTrampoline<void> blockStaticCtor = hk::hook::trampoline([]() -> void {
    blockStaticCtor.orig();
    CustomBlocks::CUSTOM_BLOCK = (FireBlock*)(new FireBlock())
                                     ->setNotCollectStatistics()
                                     ->setNameAndDescriptionId(0xB59DECE7, -1)
                                     ->disableMipmap();

    Block::registerBlock(3000, L"custom_block", CustomBlocks::CUSTOM_BLOCK);
});

void addBlocks(MenuBuilder* menuBuilder, Block* orig) {
    menuBuilder->ITEM_BLOCK(orig);
    menuBuilder->ITEM_BLOCK(CustomBlocks::CUSTOM_BLOCK);
}

void overrideRegistrySize(void* ptr, int a2, int size) {
    size = 0x800;  // Crashes if this is any higher, this is the vanilla amount
    memset(ptr, 0, size);
}

typedef DefaultedMappedRegistry<ResourceLocation, Block*, std::hash<ResourceLocation>,
                                std::equal_to<ResourceLocation>, Block*, std::equal_to<Block*>>
    BlockRegistry;

HkTrampoline<bool, BlockRegistry*, int> containsId
    = hk::hook::trampoline([](BlockRegistry* registry, int id) -> bool {
          hk::gfx::imguiLog("Checking if block ID exists in registry: %d\n", id);

          bool exists = containsId.orig(registry, id);
          if (id == 3000) {
              return true;
          }
          return exists;
      });

HkTrampoline<bool, int> blockExistsWithId = hk::hook::trampoline([](int id) -> bool {
    // hk::gfx::imguiLog("Checking if block ID %d exists in registry...", id);

    bool exists = blockExistsWithId.orig(id);
    return exists;
    // if (exists) {
    //     hk::gfx::imguiLog("Block ID %d exists in registry.", id);
    // }
    //
    // if (id == 3000) {
    //     hk::gfx::imguiLog("Custom block! %d", id);
    //     return true;
    // }
    //
    // return exists;
});

void BlockHooks::initHooks() {
    blockStaticCtor.installAtSym<"_ZN5Block10staticCtorEv">();

    const hk::ro::RoModule* main = hk::ro::getMainModule();
    hk::hook::writeBranchLink(main, 0x5778BC, addBlocks);
    hk::hook::writeBranch(main, 0x5C8E4, overrideRegistrySize);

    containsId.installAtSym<"_ZN23DefaultedMappedRegistryI16ResourceLocationP5BlockNSt3__14hashIS0_EENS3_"
                            "8equal_toIS0_EENS4_IS2_EENS6_IS2_EEE10containsIdEi">();

    // blockExistsWithId.installAtSym<"_ZN5Block12existsWithIdEi">();
}
