#pragma once

#include <functional>
#include "geometry/vec2.h"

namespace omm
{

class ObjectTransformation;

class MousePanController
{
private:
  using set_cursor_pos_type = std::function<void(const Vec2f& pos)>;
public:
  explicit MousePanController(const set_cursor_pos_type& set_cursor_pos);
  enum class Action { None, Pan, Zoom };
  void start_move(const Vec2f& global_pos, Action actiono);
  bool end_move();
  Vec2f apply(const Vec2f& delta, ObjectTransformation& t);

private:
  Vec2f m_global_start_position;
  Action m_action = Action::None;
  const set_cursor_pos_type m_set_cursor_pos;
  bool m_was_applied = false;
};

}  // namespace omm
