#pragma once

#include <string>
#include <vector>
#include "ppapi/c/ppb_input_event.h"

namespace verge {
    enum class EventType {
        None,
        KeyUp,
        KeyDown
    };

    struct InputEvent {
        EventType type;
        int keyCode;
    };

    struct IFramebuffer {
        virtual void getInputEvents(std::vector<verge::InputEvent>& events) = 0;
        virtual void vgadump(unsigned char* backBuffer, unsigned char* palette) = 0;
        virtual void persistSave(const std::string& fileName, const std::string& saveData) = 0;
    };

    extern IFramebuffer* plugin;
}
