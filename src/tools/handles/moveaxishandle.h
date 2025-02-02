#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "renderers/painter.h"
#include "geometry/util.h"
#include <QDebug>

namespace omm
{

enum class MoveAxisHandleDirection { X, Y };

template<typename ToolT, MoveAxisHandleDirection direction>
class MoveAxisHandle : public Handle
{
public:
  MoveAxisHandle(ToolT& tool)
    : Handle(tool)
    , m_direction(direction == MoveAxisHandleDirection::X ? Vec2f(100.0, 0.0)
                                                          : Vec2f(0.0, 100.0))
  {
    switch (direction) {
    case MoveAxisHandleDirection::X:
      set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
      set_style(Status::Hovered, omm::ContourStyle(omm::Color(1.0, 0.0, 0.0)));
      set_style(Status::Inactive, omm::ContourStyle(omm::Color(1.0, 0.3, 0.3)));
      break;
    case MoveAxisHandleDirection::Y:
      set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
      set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.0, 1.0, 0.0)));
      set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.3, 1.0, 0.3)));
      break;
    }
  }

  bool contains_global(const Vec2f& point) const override
  {
    const Vec2f global_point = tool.transformation().inverted().apply_to_position(point);
    Vec2f v = project_onto_axis(global_point);

    // clamp v between o and m_direction
    v = v.clamped(Vec2f::min(Vec2f::o(), m_direction), Vec2f::max(Vec2f::o(), m_direction));

    return (global_point - v).euclidean_norm() < interact_epsilon();
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto inv_tool_transformation = tool.transformation().inverted();
      auto total_delta = inv_tool_transformation.apply_to_direction(pos - press_pos());
      total_delta = project_onto_axis(total_delta);
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

  void draw(omm::Painter& renderer) const override
  {
    renderer.push_transformation(tool.transformation());
    const double magnitude = m_direction.euclidean_norm();
    const double argument = m_direction.arg();

    const auto right = to_qpoint(PolarCoordinates(argument-0.1, magnitude*0.9).to_cartesian());
    const auto left = to_qpoint(PolarCoordinates(argument+0.1, magnitude*0.9).to_cartesian());

    renderer.set_style(current_style());
    renderer.painter->drawLine(QPointF(0.0, 0.0), to_qpoint(m_direction));
    const QPointF polyline[] = { left, to_qpoint(m_direction), right, left };
    renderer.painter->drawPolyline(polyline, 4);
    renderer.pop_transformation();
  }

private:
  Vec2f project_onto_axis(const Vec2f& v) const
  {
    const Vec2f s = m_direction;

    // project v onto the line through o and s
    return Vec2f::dot(v, s) / Vec2f::dot(s, s) * s;
  }

  const Vec2f m_direction;
};

}  // namespace omm
