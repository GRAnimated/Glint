#include "block/BlockHooks.h"
#include "block/ItemHooks.h"
#include "hk/gfx/DebugRenderer.h"
#include "hk/hook/Trampoline.h"

#include "Minecraft.Client/Minecraft.h"
#include "addons/ImGui/include/hk/gfx/ImGuiConfig.h"
#include "chunk/LevelChunkExtraData.h"
#include "debug/DebugScreenOverlay.h"
#include "levelgen/DimensionHooks.h"

HkTrampoline<void, Minecraft*> minecraftInit = hk::hook::trampoline([](Minecraft* minecraft) -> void {
    minecraftInit.orig(minecraft);

    hk::gfx::imguiLog("Minecraft::init\n");
});

HkTrampoline<void, Minecraft*> minecraftRunMiddle
    = hk::hook::trampoline([](Minecraft* minecraft) -> void { minecraftRunMiddle.orig(minecraft); });

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// The STL links to this function but it's not defined on the switch.
extern "C" void* __libc_calloc(size_t nmemb, size_t size) {
    if (size != 0 && nmemb > SIZE_MAX / size) {
        return NULL;
    }

    size_t total_size = nmemb * size;
    void* ptr = malloc(total_size);
    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, total_size);
    return ptr;
}

extern "C" void hkMain() {
    minecraftInit.installAtSym<"_ZN9Minecraft4initEv">();
    minecraftRunMiddle.installAtSym<"_ZN9Minecraft10run_middleEv">();

    DebugScreenOverlay::initHooks();
    DimensionHooks::initHooks();
    BlockHooks::initHooks();
    ItemHooks::initHooks();
    LevelChunkExtraData::initHooks();

    hk::gfx::DebugRenderer::instance()->installHooks();
}
