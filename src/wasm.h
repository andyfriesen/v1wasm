#pragma once

namespace verge
{
    enum class EventType {
        None,
        KeyUp,
        KeyDown,
    };

    struct InputEvent {
        EventType type;
        int keyCode;
    };
}
