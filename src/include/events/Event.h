#pragma once

class Event {
public:
  virtual const char* GetEventType() const = 0;
  virtual ~Event() = default;

public:
  bool isHandled { false };
};
