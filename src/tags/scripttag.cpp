#include "tags/scripttag.h"
#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include <pybind11/embed.h>

#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "properties/triggerproperty.h"

#include "python/tagwrapper.h"
#include "python/scenewrapper.h"
#include "python/pythonengine.h"
#include "common.h"

namespace py = pybind11;

namespace omm
{

ScriptTag::ScriptTag(Object& owner)
  : Tag(owner)
{
  add_property<StringProperty>(CODE_PROPERTY_KEY)
    .set_is_multi_line(true)
    .set_label("code").set_category("script");
  add_property<OptionsProperty>(UPDATE_MODE_PROPERTY_KEY)
    .set_options({ "per frame", "on request" })
    .set_label("update").set_category("script");
  add_property<TriggerProperty>(TRIGGER_UPDATE_PROPERTY_KEY)
    .set_label("evaluate").set_category("script");
}

ScriptTag::~ScriptTag()
{
}

std::string ScriptTag::type() const
{
  return TYPE;
}

QIcon ScriptTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_FileDialogListView);
}

std::unique_ptr<Tag> ScriptTag::clone() const
{
  return std::make_unique<ScriptTag>(*this);
}

void ScriptTag::on_property_value_changed(Property& property)
{
  if (&property == &this->property(TRIGGER_UPDATE_PROPERTY_KEY)) {
    run();
  }
}

void ScriptTag::run()
{
  Scene* scene = owner.scene();
  assert(scene != nullptr);
  using namespace py::literals;
  const auto code = property(ScriptTag::CODE_PROPERTY_KEY).value<std::string>();
  auto locals = py::dict("this"_a=TagWrapper::make(*this), "scene"_a=SceneWrapper(*scene));
  scene->python_engine.run(code, locals);
}

bool ScriptTag::update_on_frame() const
{
  return property(UPDATE_MODE_PROPERTY_KEY).value<std::size_t>() == 0;
}

}  // namespace omm
