#include "hk/gfx/DebugRenderer.h"
#include "hk/hook/Trampoline.h"

#include "Minecraft.Client/Minecraft.h"
#include "addons/ImGui/include/hk/gfx/ImGuiConfig.h"
#include "debug/DebugScreenOverlay.h"
#include "levelgen/DimensionHooks.h"

HkTrampoline<void, Minecraft*> minecraftInit = hk::hook::trampoline([](Minecraft* minecraft) -> void {
    minecraftInit.orig(minecraft);

    hk::gfx::imguiLog("Minecraft::init\n");
});

HkTrampoline<void, Minecraft*> minecraftRunMiddle = hk::hook::trampoline([](Minecraft* minecraft) -> void {
    minecraftRunMiddle.orig(minecraft);

    /* DebugRenderer */

    // auto* renderer = hk::gfx::DebugRenderer::instance();
    //
    // renderer->clear();
    // renderer->begin(drawContext->getCommandBuffer()->ToData()->pNvnCommandBuffer);
    //
    // renderer->setGlyphSize(0.45);
    //
    // renderer->drawQuad({{30, 30}, {0, 0}, 0xef000000}, {{300, 30}, {1.0, 0}, 0xef000000},
    //                   {{300, 100}, {1.0, 1.0}, 0xef000000}, {{30, 100}, {0, 1.0}, 0xef000000});
    //
    // renderer->setCursor({50, 50});
    //
    // renderer->printf("AAAAAAIRUHihurghiuhgh\n");
    //
    // renderer->end();
});

extern "C" void hkMain() {
    minecraftInit.installAtSym<"_ZN9Minecraft4initEv">();
    minecraftRunMiddle.installAtSym<"_ZN9Minecraft10run_middleEv">();

    DebugScreenOverlay::initHooks();
    DimensionHooks::initHooks();

    hk::gfx::DebugRenderer::instance()->installHooks();
}
