#include "properties/transformationproperty.h"

namespace omm
{

std::string TransformationProperty::type() const
{
  return TYPE;
}

void TransformationProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_object_transformation(
    make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_object_transformation(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)) );
}

void TransformationProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> TransformationProperty::clone() const
{
  return std::make_unique<TransformationProperty>(*this);
}

}  // namespace omm
