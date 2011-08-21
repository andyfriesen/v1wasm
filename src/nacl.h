#pragma once

struct IFramebuffer {
    virtual void vgadump(unsigned char* backBuffer, unsigned char* palette) = 0;
};
