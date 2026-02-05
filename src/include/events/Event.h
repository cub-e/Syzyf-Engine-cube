#pragma once

#include <string>

class Event {
public:
   virtual const std::string GetEventType() const = 0;

public:
   bool isHandled { false };
};
