#include <memory>

#include "EventHandler.h"

class EventManager {
public:
   void Shutdown();

   void Subscribe(const std::string& eventId, std::unique_ptr<EventHandlerWrapperInterface>&& handler);
   void Unsubscribe(const std::string& eventId, const std::string& handlerName);
   void TriggerEvent(const Event& event);
   void QueueEvent(std::unique_ptr<Event>&& event);
   void DispatchEvents();

private:
   std::vector<std::unique_ptr<Event>> m_eventsQueue;
   std::unordered_map<std::string, std::vector<std::unique_ptr<EventHandlerWrapperInterface>>> m_subscribers;
};
