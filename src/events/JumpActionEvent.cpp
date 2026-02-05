#include "events/Event.h"

class JumpActionEvent : public Event {
public:
    JumpActionEvent() {}

    static const std::string GetStaticEventType() { return "7CCF9526-19A3-431E-B9CB-B6AA7C775469"; };

    const std::string GetEventType() const override { return GetStaticEventType(); };
};
