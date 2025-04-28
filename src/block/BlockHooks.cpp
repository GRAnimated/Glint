#include "block/BlockHooks.h"
#include "block/CustomBlocks.h"

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

void BlockHooks::initHooks() {
    blockStaticCtor.installAtSym<"_ZN5Block10staticCtorEv">();

    const hk::ro::RoModule* main = hk::ro::getMainModule();
    hk::hook::writeBranchLink(main, 0x5778BC, addBlocks);
}
