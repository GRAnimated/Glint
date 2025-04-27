#include "TextWriter.h"
#include <cstdarg>
#include <cwchar>

#include "Minecraft.Client/Minecraft.h"
#include "Minecraft.Client/renderer/GlStateManager.h"
#include "Minecraft.Client/ui/screen/ScreenSizeCalculator.h"

TextWriter::TextWriter(Font* font, int x, int y)
    : mFont(font), mStartingPosX(x), mPosX(x), mPosY(y), mColor(0xFFFFFFFF), mLineHeight(10),
      mFontScale(0.67f), mIsFillEnabled(false), mFillColor(0x000000FF), mIsShadowEnabled(true) {}

void TextWriter::begin() {
    Minecraft* minecraft = Minecraft::sInstance;
    ScreenSizeCalculator screenSizeCalculator = ScreenSizeCalculator(minecraft->mOptions, -1);

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::ortho(0.0f, screenSizeCalculator.getWidth(), screenSizeCalculator.getHeight(), 0.0f,
                          -1.0f, 3000.0f);
    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();

    GlStateManager::disableCull();
    GlStateManager::disableDepthTest();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(mPosX, mPosY, 0.0f);
    GlStateManager::scalef(mFontScale, mFontScale, mFontScale);
}

void TextWriter::printf(const wchar_t* format, ...) {
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];

    va_list args;
    va_start(args, format);
    int len = vswprintf(buffer, bufferSize, format, args);
    va_end(args);

    if (len < 0) {
        return;
    }

    for (int i = 0; i < len; i++) {
        wchar_t ch = buffer[i];

        if (ch == L'\n') {
            handleNewLine();
            continue;
        }

        int textWidth = mFont->width(std::wstring(1, ch));
        if (mIsFillEnabled) {
            fill(mPosX, mPosY, mPosX + textWidth, mPosY + mLineHeight, mFillColor);
        }

        if (mIsShadowEnabled) {
            drawString(mFont, std::wstring(1, ch), mPosX, mPosY + 1, mColor);
        } else {
            mFont->draw(std::wstring(1, ch), mPosX, mPosY + 1, mColor, false, true);
        }

        mPosX += textWidth;
    }
}

void TextWriter::end() {
    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
    GlStateManager::enableDepthTest();
}

void TextWriter::setColor(int color) {
    this->mColor = color;
}

void TextWriter::enableFill() {
    mIsFillEnabled = true;
}

void TextWriter::disableFill() {
    mIsFillEnabled = false;
}

void TextWriter::setFillColor(int color) {
    mFillColor = color;
}

void TextWriter::enableShadow() {
    mIsShadowEnabled = true;
}

void TextWriter::disableShadow() {
    mIsShadowEnabled = false;
}

void TextWriter::setLineHeight(int lineHeight) {
    this->mLineHeight = lineHeight;
}

void TextWriter::handleNewLine() {
    mPosY += mLineHeight;
    mPosX = mStartingPosX;
}
