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
        virtual void loadSound(const std::string& fileName) = 0;
        virtual void playSong(const std::string& songName) = 0;
        virtual void playEffect(size_t index) = 0;
        virtual void stopSound() = 0;
        virtual void setVolume(int volume) = 0;
        virtual void setSongPos(int songPos) = 0;
    };

    extern IFramebuffer* plugin;
}
