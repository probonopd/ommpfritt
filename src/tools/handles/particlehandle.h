#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "geometry/util.h"
#include "tools/tool.h"

namespace omm
{

class ParticleHandle : public Handle
{
public:
  explicit ParticleHandle(Tool& tool);
  bool contains_global(const Vec2f& point) const override;
  void draw(omm::Painter& renderer) const override;
  Vec2f position = Vec2f::o();

  static const Style hovered_style;
  static const Style active_style;
  static const Style inactive_style;

protected:
  bool transform_in_tool_space;
};

template<typename ToolT>
class MoveParticleHandle : public ParticleHandle
{
public:
  MoveParticleHandle(ToolT& tool) : ParticleHandle(tool) {}
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto inv_tool_transformation = tool.transformation().inverted();
      auto total_delta = inv_tool_transformation.apply_to_direction(pos - press_pos());
      discretize(total_delta);
      {
        auto transformation = omm::ObjectTransformation().translated(total_delta);
        transformation = transformation.transformed(inv_tool_transformation);
        static_cast<ToolT&>(tool).transform_objects(transformation);
      }
      total_delta = tool.viewport_transformation.inverted().apply_to_direction(total_delta);
      const auto tool_info = QString("%1").arg(total_delta.euclidean_norm());
      static_cast<ToolT&>(tool).tool_info = tool_info.toStdString();
      return true;
    } else {
      return false;
    }
  }
};

}  // namespace omm
