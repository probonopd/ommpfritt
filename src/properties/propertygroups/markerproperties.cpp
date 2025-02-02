#include "properties/propertygroups/markerproperties.h"
#include "properties/optionsproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "objects/tip.h"
#include "geometry/util.h"

namespace omm
{

MarkerProperties
::MarkerProperties(const std::string &prefix, omm::AbstractPropertyOwner &property_owner,
                   const Shape default_shape, const double default_size)
  : PropertyGroup(prefix, property_owner)
  , m_default_shape(default_shape)
  , m_default_size(default_size)
{}

void MarkerProperties::make_properties(const std::string &category) const
{
  add_property<OptionsProperty>(SHAPE_PROPERTY_KEY, static_cast<std::size_t>(m_default_shape))
    .set_options(shapes()).set_category(category)
    .set_label(QObject::tr("Shape").toStdString());

  add_property<FloatProperty>(SIZE_PROPERTY_KEY, m_default_size)
    .set_step(0.1).set_range(0.0, FloatPropertyLimits::upper)
    .set_label(QObject::tr("Size").toStdString()).set_category(category);

  add_property<FloatProperty>(ASPECT_RATIO_PROPERTY_KEY)
    .set_step(0.001)
    .set_label(QObject::tr("Aspect Ratio").toStdString()).set_category(category);

  add_property<BoolProperty>(REVERSE_PROPERTY_KEY)
      .set_label(QObject::tr("Reverse").toStdString()).set_category(category);
}

void MarkerProperties
::draw_marker(Painter &painter, const Point &location, const Color& color, const double width) const
{
  QPainter& p = *painter.painter;
  p.save();
  p.translate(to_qpoint(location.position));
  if (property_value<bool>(REVERSE_PROPERTY_KEY)) {
    p.rotate(location.rotation() * 180.0 * M_1_PI + 90);
  } else {
    p.rotate(location.rotation() * 180.0 * M_1_PI - 90);
  }
  p.setPen(Qt::NoPen);
  p.setBrush(Painter::to_qcolor(color));
  QPainterPath path = Painter::path(shape(width), true);
  p.drawPath(path);
  p.restore();
}

std::vector<std::string> MarkerProperties::shapes() const
{
  return { QObject::tr("None").toStdString(),
           QObject::tr("Arrow").toStdString(),
           QObject::tr("Bar").toStdString(),
           QObject::tr("Circle").toStdString(),
           QObject::tr("Diamond").toStdString() };
}

std::vector<Point> MarkerProperties::shape(const double width) const
{
  const double base = width * property_value<double>(SIZE_PROPERTY_KEY);
  const auto shape = property_value<Shape>(SHAPE_PROPERTY_KEY);
  const auto ar = property_value<double>(ASPECT_RATIO_PROPERTY_KEY);

  const Vec2f size(base, base * std::exp(ar));
  switch (shape) {
  case Shape::None: return {};
  case Shape::Arrow: return arrow(size);
  case Shape::Bar: return bar(size);
  case Shape::Circle: return circle(size);
  case Shape::Diamond: return diamond(size);
  }
  Q_UNREACHABLE();
}

std::vector<Point> MarkerProperties::arrow(const Vec2f& size) const
{
  return {
    Point(Vec2f(size.x, 0.0)),
    Point(Vec2f(0.0, size.y)),
    Point(Vec2f(0.0, -size.y)),
  };
}

std::vector<Point> MarkerProperties::bar(const Vec2f& size) const
{
  return {
    Point(Vec2f(-size.x, size.y)),
    Point(Vec2f(-size.x, -size.y)),
    Point(Vec2f(size.x, -size.y)),
    Point(Vec2f(size.x, size.y)),
  };
}

std::vector<Point> MarkerProperties::circle(const Vec2f& size) const
{
  Q_UNUSED(size);
  return {};
}

std::vector<Point> MarkerProperties::diamond(const Vec2f &size) const
{
  return {
    Point(Vec2f(-size.x, 0.0)),
    Point(Vec2f(0.0, -size.y)),
    Point(Vec2f(size.x, 0.0)),
    Point(Vec2f(0.0, size.y)),
  };
}

}  // namespace omm
