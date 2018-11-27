#include "propertywidgets/referencelineedit.h"

#include <QDragEnterEvent>
#include <QMimeData>

#include "properties/referenceproperty.h"
#include "scene/propertyownermimedata.h"
#include "objects/object.h"

namespace omm
{

ReferenceLineEdit::ReferenceLineEdit(AbstractPropertyOwner::Kind allowed_kinds)
  : m_allowed_kinds(allowed_kinds)
{
  connect(this, &QLineEdit::textChanged, [this]() { setPlaceholderText(""); });
  setReadOnly(true);
}

void ReferenceLineEdit::set_value(const value_type& value)
{
  bool value_has_changed = m_value != value;
  m_value = value;
  if (value == nullptr) {
    setText(tr("< no reference >"));
  } else {
    setText(QString::fromStdString(value->name()));
  }
  if (value_has_changed) {
    Q_EMIT reference_changed(value);
  }
}

void ReferenceLineEdit::set_inconsistent_value()
{
  setPlaceholderText(tr("<multiple values>"));
  clear();
}

void ReferenceLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
  if (can_drop(*event->mimeData())) {
    event->accept();
  } else {
    event->ignore();
  }
}

void ReferenceLineEdit::dropEvent(QDropEvent* event)
{
  const auto& mime_data = *event->mimeData();
  if (!can_drop(mime_data)) {
    event->ignore();
  } else {
    const auto& property_owner_mime_data = *qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    AbstractPropertyOwner* reference = property_owner_mime_data.items(m_allowed_kinds).front();
    set_value(reference);
  }
}

ReferenceLineEdit::value_type ReferenceLineEdit::value() const
{
  return m_value;
}

bool ReferenceLineEdit::can_drop(const QMimeData& mime_data) const
{
  if (mime_data.hasFormat(PropertyOwnerMimeData::MIME_TYPE))
  {
    const auto property_owner_mime_data = qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    if (property_owner_mime_data != nullptr) {
      if (property_owner_mime_data->items(m_allowed_kinds).size() == 1) {
        return true;
      }
    }
  }
  return false;
}

void ReferenceLineEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
  set_value(nullptr);
}

}  // namespace omm
