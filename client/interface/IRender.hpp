#pragma once

struct Color {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 255;
};

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void clear(const Color& color = {0,0,0,255}) = 0;
    virtual void present() = 0;
    virtual void drawRect(int x, int y, int w, int h, const Color& color = {255,255,255,255}) = 0;

    virtual int getWindowWidth() const = 0;
    virtual int getWindowHeight() const = 0;
};
