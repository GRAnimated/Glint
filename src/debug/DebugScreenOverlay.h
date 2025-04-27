#pragma once

class Font;

class DebugScreenOverlay {
public:
    DebugScreenOverlay(Font* font);

    int calculateFPS();
    void render();

    static void initHooks();

private:
    Font* mFont;
};
