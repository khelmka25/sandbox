#pragma once

#include <any>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <utility>
#include <variant>

namespace sb {
enum class EventType : unsigned {
  kViewportEvent,
  kKeyboardEvent,
  kMouseButtonEvent,
  kMousePositionEvent,
  kScrollEvent,
  kCursorEnterEvent,
  kDropEvent,
};

struct ViewportEvent {
  int width;
  int height;
};

struct KeyboardEvent {
  int key;
  int scancode;
  int action;
  int mods;
};

struct MouseButtonEvent {
  int button;
  int action;
  int mods;
};

struct MousePositionEvent {
  float xpos;
  float ypos;
};

struct ScrollEvent {
  float dx;
  float dy;
};

struct DropEvent {
  std::vector<std::string> paths;
};

struct Event {
  explicit Event(EventType t_type, auto t_content) : type(t_type), content(t_content) {}
  explicit Event(EventType t_type) : type(t_type) {}

  template <typename T, typename... Args>
  explicit Event(EventType t_type, std::in_place_type_t<T>, Args&&... args)
      : type(t_type), content(std::in_place_type_t<T>{}, std::forward<Args>(args)...) {}

  const auto& contents() const { return content; }

  EventType type;
  std::any content;
};

extern std::queue<Event> eventQueue;
}  // namespace sb