#include "block/CustomBlocks.h"
#include "block/ItemHooks.h"

#include "hk/hook/Trampoline.h"

#include "Minecraft.World/item/Item.h"

HkTrampoline<void> itemStaticCtor = hk::hook::trampoline([]() -> void {
    itemStaticCtor.orig();

    Item::registerBlock(CustomBlocks::CUSTOM_BLOCK);
});

void ItemHooks::initHooks() {
    itemStaticCtor.installAtSym<"_ZN4Item10staticCtorEv">();
}
