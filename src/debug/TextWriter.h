#pragma once

#include "Minecraft.Client/gui/Font.h"
#include "Minecraft.Client/gui/GuiComponent.h"

class TextWriter : public GuiComponent {
public:
    TextWriter(Font* font, int x, int y);

    void begin();
    void printf(const wchar_t* format, ...);
    void end();

    void setColor(int color);

    void enableFill();
    void disableFill();
    void setFillColor(int color);

    void enableShadow();
    void disableShadow();

    void setLineHeight(int lineHeight);

private:
    Font* mFont;
    int mStartingPosX;
    int mPosX;
    int mPosY;
    int mColor;
    int mLineHeight;
    float mFontScale;
    bool mIsFillEnabled;
    int mFillColor;
    bool mIsShadowEnabled;

    void handleNewLine();
};
