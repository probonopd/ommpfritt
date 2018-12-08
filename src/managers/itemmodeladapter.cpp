#include "managers/itemmodeladapter.h"
#include "scene/contextes.h"
#include "scene/propertyownermimedata.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "commands/movecommand.h"
#include "commands/copycommand.h"
#include <QAbstractItemModel>
#include <QAbstractListModel>

namespace
{

template<typename T, typename StructureT> bool
can_move_drop_items( StructureT& structure,
                     const std::vector<typename omm::Contextes<T>::Move>& contextes )
{
  const auto is_strictly_valid = [&structure](const typename omm::Contextes<T>::Move& context) {
    return context.is_strictly_valid(structure);
  };

  const auto is_valid = [](const typename omm::Contextes<T>::Move& context) {
    return context.is_valid();
  };

  // all contextes may be moved and at least one move is no noop
  return std::all_of(contextes.begin(), contextes.end(), is_valid)
      && std::any_of(contextes.begin(), contextes.end(), is_strictly_valid);
}

template<typename ContextT, typename ItemModelAdapterT>
typename std::enable_if<ContextT::is_tree_context, std::vector<ContextT>>::type
make_contextes( const ItemModelAdapterT& adapter,
                const QMimeData* data, int row, const QModelIndex& parent )
{
  std::vector<ContextT> contextes;
  using T = typename ContextT::item_type;

  auto property_owner_mime_data = qobject_cast<const omm::PropertyOwnerMimeData*>(data);
  auto items = property_owner_mime_data->items<T>();
  if (property_owner_mime_data == nullptr || items.size() == 0) {
    return contextes;
  }

  T& new_parent = adapter.item_at(parent);
  const size_t pos = row < 0 ? new_parent.n_children() : row;

  ContextT::remove_internal_children(items);
  contextes.reserve(items.size());
  const T* predecessor = (pos == 0) ? nullptr : &new_parent.child(pos - 1);
  for (T* subject : items) {
    contextes.emplace_back(*subject, new_parent, predecessor);
    predecessor = subject;
  }

  return contextes;
}

template<typename ContextT, typename ItemModelAdapterT>
typename std::enable_if<!ContextT::is_tree_context, std::vector<ContextT>>::type
make_contextes( const ItemModelAdapterT& adapter,
                const QMimeData* data, int row, const QModelIndex& parent )
{
  if (parent.isValid()) {
    return std::vector<ContextT>(); // it's a list, not a tree.
  }

  using T = typename ContextT::item_type;

  auto property_owner_mime_data = qobject_cast<const omm::PropertyOwnerMimeData*>(data);
  auto items = property_owner_mime_data->items<T>();
  if (property_owner_mime_data == nullptr || items.size() == 0) {
    return std::vector<ContextT>();
  }

  ContextT::remove_internal_children(items);

  std::vector<ContextT> contextes;
  contextes.reserve(items.size());
  const size_t pos = row < 0 ? adapter.rowCount() : row;
  const T* predecessor = (pos == 0) ? nullptr : &adapter.item_at(adapter.index(pos-1, 0, parent));
  for (T* subject : items) {
    contextes.emplace_back(*subject, predecessor);
    predecessor = subject;
  }
  return contextes;
}

}  // namespace

namespace omm
{

template<typename StructureT, typename ItemModel>
ItemModelAdapter<StructureT, ItemModel>::ItemModelAdapter(Scene& scene, StructureT& structure)
  : m_scene(scene)
  , m_structure(structure)
{
  m_structure.Observed<AbstractStructureObserver<StructureT>>::register_observer(*this);
}

template<typename StructureT, typename ItemModel>
ItemModelAdapter<StructureT, ItemModel>::~ItemModelAdapter()
{
  m_structure.Observed<AbstractStructureObserver<StructureT>>::unregister_observer(*this);
}

template<typename StructureT, typename ItemModel> Qt::DropActions
ItemModelAdapter<StructureT, ItemModel>::supportedDragActions() const
{
  return Qt::LinkAction | Qt::MoveAction | Qt::CopyAction;
}

template<typename StructureT, typename ItemModel> Qt::DropActions
ItemModelAdapter<StructureT, ItemModel>::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}

template<typename StructureT, typename ItemModel> bool ItemModelAdapter<StructureT, ItemModel>
::canDropMimeData( const QMimeData *data, Qt::DropAction action,
                   int row, int column, const QModelIndex &parent ) const
{
  using Context = typename Contextes<item_type>::Move;
  switch (action) {
  case Qt::MoveAction:
    return data->hasFormat(PropertyOwnerMimeData::MIME_TYPE)
        && qobject_cast<const PropertyOwnerMimeData*>(data) != nullptr
        && can_move_drop_items<item_type>( m_structure,
                                           make_contextes<Context>(*this, data, row, parent) );
  case Qt::CopyAction:
    return data->hasFormat(PropertyOwnerMimeData::MIME_TYPE)
        && qobject_cast<const PropertyOwnerMimeData*>(data) != nullptr;
  default:
    return false;
  }
}

template<typename StructureT, typename ItemModel> bool
ItemModelAdapter<StructureT, ItemModel>
::dropMimeData( const QMimeData *data, Qt::DropAction action,
                int row, int column, const QModelIndex &parent )
{
  using MoveContext = typename Contextes<item_type>::Move;
  using OwningContext = typename Contextes<item_type>::Owning;
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  } else {
    std::unique_ptr<Command> command;
    switch (action) {
    case Qt::MoveAction: {
      auto move_contextes = make_contextes<MoveContext>(*this, data, row, parent);
      m_scene.submit<MoveCommand<StructureT>>(m_structure, move_contextes);
      break;
    }
    case Qt::CopyAction: {
      auto copy_contextes = make_contextes<OwningContext>(*this, data, row, parent);
      m_scene.submit<CopyCommand<StructureT>>(m_structure, std::move(copy_contextes));
      break;
    }
    }
    return true;
  }
}

template<typename StructureT, typename ItemModel>
QStringList ItemModelAdapter<StructureT, ItemModel>::mimeTypes() const
{
  return { PropertyOwnerMimeData::MIME_TYPE };
}

template<typename StructureT, typename ItemModel>
QMimeData* ItemModelAdapter<StructureT, ItemModel>::mimeData(const QModelIndexList &indexes) const
{
  if (indexes.isEmpty()) {
    return nullptr;
  } else {
    const auto f = [this](const QModelIndex& index) {
      // TODO also return selected tags ans styles
      return &this->item_at(index);
    };
    const auto items = ::transform<AbstractPropertyOwner*, std::vector>(indexes, f);
    return std::make_unique<PropertyOwnerMimeData>(items).release();
  }
}

template<typename StructureT, typename ItemModel>
StructureT& ItemModelAdapter<StructureT, ItemModel>::structure() const
{
  return m_structure;
}

template<typename StructureT, typename ItemModel>
Scene& ItemModelAdapter<StructureT, ItemModel>::scene() const
{
  return m_scene;
}



template class ItemModelAdapter<Tree<Object>, QAbstractItemModel>;
template class ItemModelAdapter<List<Style>, QAbstractListModel>;

}  // namespace omm
