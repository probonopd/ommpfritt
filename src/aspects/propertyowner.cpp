#include "aspects/propertyowner.h"

#include <QObject>

#include "external/json.hpp"
#include "serializers/abstractserializer.h"
#include "properties/referenceproperty.h"

namespace
{
  constexpr auto PROPERTIES_POINTER = "properties";
  constexpr auto PROPERTY_TYPE_POINTER = "type";
  constexpr auto PROPERTY_KEY_POINTER = "key";
  constexpr auto ID_POINTER = "id";
}  // namespace

namespace omm
{

const std::string AbstractPropertyOwner::NAME_PROPERTY_KEY = "name";


AbstractPropertyOwner::AbstractPropertyOwner()
{
}

AbstractPropertyOwner::AbstractPropertyOwner(AbstractPropertyOwner&& other)
  : m_properties(std::move(other.m_properties))
{
}

AbstractPropertyOwner::AbstractPropertyOwner(const AbstractPropertyOwner& other)
  : m_properties(other.m_properties)
{
}

AbstractPropertyOwner::~AbstractPropertyOwner()
{
}

const OrderedMap<std::string, Property>& AbstractPropertyOwner::properties() const
{
  return m_properties;
}

Property& AbstractPropertyOwner::property(const std::string& key) const
{
  assert(has_property(key));
  return *m_properties.at(key);
}

bool AbstractPropertyOwner::has_property(const std::string& key) const
{
  return m_properties.contains(key);
}

void AbstractPropertyOwner::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  const auto id_pointer = make_pointer(root, ID_POINTER);
  serializer.set_value(this, id_pointer);

  const auto properties_pointer = make_pointer(root, PROPERTIES_POINTER);
  serializer.start_array(m_properties.size(), properties_pointer);
  for (size_t i = 0; i < m_properties.size(); ++i) {
    const auto property_key = m_properties.keys().at(i);
    const auto property_pointer = make_pointer(properties_pointer, i);
    const auto& property = this->property(property_key);
    serializer.set_value(property_key, make_pointer(property_pointer, PROPERTY_KEY_POINTER));
    serializer.set_value(property.type(), make_pointer(property_pointer, PROPERTY_TYPE_POINTER));
    property.serialize(serializer, property_pointer);
  }
  serializer.end_array();
}

void AbstractPropertyOwner::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);

  const auto id_pointer = make_pointer(root, ID_POINTER);
  const auto id = deserializer.get_size_t(id_pointer);
  deserializer.register_reference(id, *this);

  const auto properties_pointer = make_pointer(root, PROPERTIES_POINTER);
  size_t n_properties = deserializer.array_size(properties_pointer);
  for (size_t i = 0; i < n_properties; ++i) {
    const auto property_pointer = make_pointer(properties_pointer, i);

    const auto property_key =
      deserializer.get_string(make_pointer(property_pointer, PROPERTY_KEY_POINTER));
    const auto property_type =
      deserializer.get_string(make_pointer(property_pointer, PROPERTY_TYPE_POINTER));

    if (properties().contains(property_key)) {
      assert(property_type == property(property_key).type());
    } else {
      try {
        add_property(property_key, Property::make(property_type));
      } catch (const std::out_of_range& e) {
        const auto message = "Failed to retrieve property type '" + property_type + "'.";
        LOG(ERROR) << message;
        throw AbstractDeserializer::DeserializeError(message);
      }
    }

    property(property_key).deserialize(deserializer, property_pointer);
  }
}

void AbstractPropertyOwner::on_property_value_changed(Property& property)
{
}

std::string AbstractPropertyOwner::name() const
{
  return property(NAME_PROPERTY_KEY).value<std::string>();
}

bool AbstractPropertyOwner::has_reference_cycle(const std::string& key) const
{
  const AbstractPropertyOwner* current = this;
  while (current->has_property<ReferenceProperty::value_type>(key)) {
    const auto ref_variant = current->property(key).variant_value();
    const auto* reference = std::get<ReferenceProperty::value_type>(ref_variant);
    if (reference == nullptr) {
      return false;
    } else if (reference == this) {
      return true;
    } else {
      current = reference;
    }
  }
  return false;
}

std::unique_ptr<Property> AbstractPropertyOwner::extract_property(const std::string& key)
{
  auto property = m_properties.extract(key);
  property->unregister_observer(*this);
  return property;
}

}  // namespace omm

omm::AbstractPropertyOwner::Kind operator|( omm::AbstractPropertyOwner::Kind a,
                                            omm::AbstractPropertyOwner::Kind b )
{
  using enum_t = omm::AbstractPropertyOwner::Kind;
  using underlying = std::underlying_type_t<enum_t>;
  return static_cast<enum_t>(static_cast<underlying>(a) | static_cast<underlying>(b));
}

omm::AbstractPropertyOwner::Kind operator&( omm::AbstractPropertyOwner::Kind a,
                                            omm::AbstractPropertyOwner::Kind b )
{
  using enum_t = omm::AbstractPropertyOwner::Kind;
  using underlying = std::underlying_type_t<enum_t>;
  return static_cast<enum_t>(static_cast<underlying>(a) & static_cast<underlying>(b));
}

omm::AbstractPropertyOwner::Kind operator~( omm::AbstractPropertyOwner::Kind a )
{
  using enum_t = omm::AbstractPropertyOwner::Kind;
  using underlying = std::underlying_type_t<enum_t>;
  return static_cast<enum_t>( static_cast<underlying>(omm::AbstractPropertyOwner::Kind::All)
                            & ~static_cast<underlying>(a));
}

bool operator!(omm::AbstractPropertyOwner::Kind a)
{
  using enum_t = omm::AbstractPropertyOwner::Kind;
  using underlying = std::underlying_type_t<enum_t>;
  return !static_cast<underlying>(a);
}
