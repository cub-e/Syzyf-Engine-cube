#pragma once

#include <string>

class Event {
public:
   virtual const std::string GetEventType() const = 0;
   virtual ~Event() = default;

public:
   bool isHandled { false };
};
