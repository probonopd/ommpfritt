#include "objects/tip.h"
#include "properties/optionsproperty.h"
#include "properties/floatproperty.h"

namespace
{

static constexpr double default_marker_size = 100.0;
static constexpr auto default_marker_shape = omm::MarkerProperties::Shape::Arrow;

}  // namespace

namespace omm
{

Tip::Tip(Scene *scene)
  : AbstractProceduralPath(scene)
  , m_marker_properties("", *this, default_marker_shape, default_marker_size)
{
  const auto tip_category = QObject::tr("Tip").toStdString();
  m_marker_properties.make_properties(tip_category);
}

Tip::Tip(const Tip &other)
  : AbstractProceduralPath(other)
  , m_marker_properties("", *this, default_marker_shape, default_marker_size)
{
}
std::vector<Point> Tip::points() const { return m_marker_properties.shape(1.0); }
std::string Tip::type() const { return "Tip"; }
std::unique_ptr<Object> Tip::clone() const { return std::make_unique<Tip>(*this); }
bool Tip::is_closed() const { return true; }

}  // namespace omm
