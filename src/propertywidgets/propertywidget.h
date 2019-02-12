#pragma once

#include <QWidget>
#include "properties/typedproperty.h"
#include "common.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"

namespace omm
{

class AbstractPropertyWidget
  : public QWidget
  , public AbstractFactory< std::string, AbstractPropertyWidget,
                            Scene&, const std::string&, const std::set<Property*>& >
  , public AbstractPropertyObserver
{
public:
  explicit AbstractPropertyWidget( Scene& scene, const std::string& key,
                                   const std::set<Property*>& properties );
  virtual ~AbstractPropertyWidget();
  void on_property_value_changed(Property& property) override;

protected:
  std::string label() const;
  std::string tool_tip() const;

  void set_default_layout(std::unique_ptr<QWidget> other);
  std::unique_ptr<QWidget> make_label_widget() const;
  Scene& scene;
  virtual void update_edit() = 0;

private:
  const std::string m_label;
  const std::string m_key;
  std::set<Property*> m_properties;
  template<typename PropertyT> friend class PropertyWidget;
};

template<typename PropertyT>
class PropertyWidget : public AbstractPropertyWidget
{
public:
  using AbstractPropertyWidget::AbstractPropertyWidget;
  using property_type = PropertyT;
  using value_type = typename property_type::value_type;
  static const std::string TYPE;

protected:
  virtual void set_properties_value(const value_type& value)
  {
    const bool wrap = std::any_of(m_properties.begin(), m_properties.end(), [](const Property* p) {
      return p->wrap_with_macro;
    });

    const auto is_noop = [&value](const Property* p) {
      return p->value<value_type>() == value;
    };
    if (!std::all_of(m_properties.begin(), m_properties.end(), is_noop)) {
      auto command = std::make_unique<PropertiesCommand<property_type>>(m_properties, value);
      if (wrap) {
        scene.undo_stack.beginMacro(QString::fromStdString(command->label()));
        for (auto* property : m_properties) { property->pre_submit(*property); }
      }
      scene.submit(std::move(command));
      if (wrap) {
        for (auto* property : m_properties) { property->post_submit(*property); }
        scene.undo_stack.endMacro();
      }
    }
  }

  auto get_properties_values() const
  {
    return ::transform<value_type>(m_properties, [](const auto* property) {
      return property->template value<value_type>();
    });
  }

  const std::set<Property*>& properties() const { return m_properties; }
};

template<typename PropertyT> const std::string
PropertyWidget<PropertyT>::TYPE = std::string(PropertyT::TYPE) + "Widget";

void register_propertywidgets();

}  // namespace omm
