#include "debug/DebugScreenOverlay.h"
#include <memory>

#include "Minecraft.World/level/biome/Biome.h"
#include "debug/TextWriter.h"

#include "Minecraft.Client/Minecraft.h"
#include "Minecraft.Client/Options.h"
#include "Minecraft.Server/MinecraftServer.h"
#include "Minecraft.World/entity/Entity.h"
#include "Minecraft.World/level/Level.h"
#include "Minecraft.World/level/storage/LevelData.h"
#include "Minecraft.World/phys/Vec3.h"
#include "Minecraft.Core/System.h"

#include "hk/hook/Trampoline.h"

DebugScreenOverlay::DebugScreenOverlay(Font* font) : mFont(font) {}

int DebugScreenOverlay::calculateFPS() {
    static long lastTime = 0;
    static int frameCount = 0;
    static int fps = 0;

    long currentTime = System::processTimeInMilliSecs();

    frameCount++;

    if (currentTime - lastTime >= 1000) {
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }

    return fps;
}

const char* wstringToCString(const std::wstring& wstr) {
    const int length = wstr.length();
    char* cstr = new char[length + 1];
    int j = 0;

    for (int i = 0; i < length; i++) {
        if (wstr[i] != L'\0') {
            cstr[j] = static_cast<char>(wstr[i]);
            j++;
        }
    }
    cstr[j] = '\0';
    return cstr;
}

void DebugScreenOverlay::render() {
    Minecraft* minecraft = Minecraft::GetInstance();
    Level* level = MinecraftServer::sInstance->getLevel(0);
    Options* options = minecraft->mOptions;
    TextWriter writer = TextWriter(mFont, 1, 1);

    writer.begin();

    writer.setColor(0xFFE0E0E0);

    writer.enableFill();
    writer.setFillColor(0x5F5F5F5F);

    writer.disableShadow();

    writer.printf(L"Minecraft: Nintendo Switch Edition v1.0.17\n");
    writer.printf(L"%i fps\n", calculateFPS());

    writer.printf(L"\n\n\n\n\n\n");

    std::shared_ptr<Entity> camera = minecraft->getCameraEntity();
    Vec3 pos = camera->getPos(0.0f);

    int blockX = static_cast<int>(pos.x);
    int blockY = static_cast<int>(pos.y);
    int blockZ = static_cast<int>(pos.z);

    int chunkX = blockX >> 4;
    int chunkY = blockY >> 4;
    int chunkZ = blockZ >> 4;

    writer.printf(L"XYZ: %.3f / %.5f / %.3f\n", pos.x, pos.y, pos.z);
    writer.printf(L"Block: %i %i %i\n", blockX, blockY, blockZ);
    // writer.printf(L"Chunk: %i %i %i [%i %i in %s]\n", chunkX, chunkY, chunkZ, blockX & 15,
    //               blockZ & 15, "nullptr");
    writer.printf(L"Chunk: %i %i %i [%i, %i]\n", chunkX, chunkY, chunkZ, blockX & 15, blockZ & 15);

    writer.printf(L"Seed: %ld\n", level->getSeed());
    writer.printf(L"Sea Level: %i\n", level->getSeaLevel());
    const char* biomeName
        = wstringToCString(level->getBiome(BlockPos(blockX, blockY, blockZ))->getName(true));
    writer.printf(L"Biome: %s\n\n", biomeName);

    writer.printf(L"Music: %.2f / Sound: %.2f / Sensitivity: %.2f\n", options->mMusicVolume,
                  options->mSoundVolume, options->mMouseSensitivity);

    writer.end();

    delete[] biomeName;
}

static DebugScreenOverlay* sDebugScreenOverlay = nullptr;

HkTrampoline<void, void*, float, bool, int, int> guiRender
    = hk::hook::trampoline([](void* gui, float a2, bool a3, int a4, int a5) -> void {
          guiRender.orig(gui, a2, a3, a4, a5);

          if (!sDebugScreenOverlay)
              sDebugScreenOverlay = new DebugScreenOverlay(Minecraft::GetInstance()->mDefaultFont);

          sDebugScreenOverlay->render();
      });

void DebugScreenOverlay::initHooks() {
    guiRender.installAtSym<"_ZN3Gui6renderEfbii">();
}
