#pragma once

#include <variant>

enum class EventType : unsigned {
  kKeyboardEvent,
  kMouseButtonEvent,
  kMousePositionEvent,
  kScrollEvent,
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

struct Event {
  explicit Event(EventType t_type, auto t_content) : type(t_type), content(t_content) {}

  const auto& contents() const { return content; }
  
  EventType type;
  std::variant<KeyboardEvent, MouseButtonEvent, MousePositionEvent, ScrollEvent> content;
};