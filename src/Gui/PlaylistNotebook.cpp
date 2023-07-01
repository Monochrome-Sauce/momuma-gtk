#include <gtkmm/liststore.h>
#include <momuma/bitset.h>
#include <momuma/momuma.h>
#include <momuma/spdlog.h>

#include "Gui/PlaylistNotebook.h"
#include "misc.h"


// Type of the container used inside a Gtk::Notebook page
using Container = Gtk::ScrolledWindow;

namespace CreateManaged
{

/* #Creates an abstract `Gtk::Container` (e.g `Gtk::ScrolledWindow`).
! The container has a PlaylistTreeView added directly to it.
! @return: pointer to a dynamically allocated `Gtk::Container`, marked as managed.
*/
[[nodiscard]] static
Gtk::Container* treeViewContainer(void);

/* #Creates a managed `Gtk::Label`.
! Use as a convenience function for creating a `Gtk::Label` that can use Pango attributes.
! Markup is disabled for the created label.
! @param text: text contents of the label.
! @param attributes: Pango::AttrList to add to the new label.
! @return: pointer to a dynamically allocated `Gtk::Label`, marked as managed.
*/
[[nodiscard]] static
Gtk::Label* pangoLabel(const Glib::ustring &text, Pango::AttrList &attributes)
{
	auto *label = Gtk::make_managed<Gtk::Label>();
	label->set_text(text);
	label->set_use_markup(false);
	label->set_attributes(attributes);
	return label;
}

}

[[nodiscard]] static inline
std::vector<int> path_to_indices(Gtk::TreePath path)
{
	gint len = 0;
	const gint *indices = gtk_tree_path_get_indices_with_depth(path.gobj(), &len);
	assert(indices != nullptr);
	return std::vector<int>(&indices[0], &indices[len]);
}

[[nodiscard]] static inline
int path_to_line_index(Gtk::TreePath path)
{
	const std::vector<int> indices = path_to_indices(std::move(path));
	assert(indices.size() == 1);
	return indices.at(0);
}

[[nodiscard]] static
Gtk::TreeRow row_from_ref(Gtk::TreeRowReference &ref)
{
	return *ref.get_model()->get_iter(ref.get_path());
}



class ColumnRecord : public Gtk::TreeModel::ColumnRecord
{
public:
	[[nodiscard]] static inline
	const ColumnRecord& get(void)
	{
		static const ColumnRecord instance;
		return instance;
	}
	
	Gtk::TreeModelColumn<Gui::NotebookColBit> markedColumns;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<chrono::seconds> duration;
	
private:
	ColumnRecord(void)
	{
		this->add(markedColumns);
		this->add(name);
		this->add(duration);
	}
};

class PlaylistTreeView : public Gtk::TreeView
{
public:
	PlaylistTreeView(void) :
		Gtk::TreeView { Gtk::ListStore::create(ColumnRecord::get()) }
	{
		this->set_headers_visible(false);
		this->set_grid_lines(Gtk::TREE_VIEW_GRID_LINES_NONE);
		Gtk::TreeViewColumn *column = nullptr;
		Gtk::CellRendererText *cell = nullptr;
		
		column = Gtk::make_managed<Gtk::TreeViewColumn>();
		cell = Gtk::make_managed<Gtk::CellRendererText>();
		this->column_pack_start(*column, *cell, &this->cb__render_line);
		this->append_column(*column);
		
		column = Gtk::make_managed<Gtk::TreeViewColumn>();
		cell = Gtk::make_managed<Gtk::CellRendererText>();
		this->column_pack_start(*column, *cell, &this->cb__render_name);
		this->append_column(*column);
		
		column = Gtk::make_managed<Gtk::TreeViewColumn>();
		cell = Gtk::make_managed<Gtk::CellRendererText>();
		this->column_pack_start(*column, *cell, &this->cb__render_duration);
		this->append_column(*column);
	}
	
private:
	using ColumnBit = Gui::NotebookColBit;
	
	[[nodiscard]] static inline
	Pango::AttrList attributes_for_cell(const Gtk::TreeIter &iter, ColumnBit column)
	{
		const ColumnBit marked = iter->get_value(ColumnRecord::get().markedColumns);
		
		Pango::AttrList attrs;
		if (Momuma::Bitset(marked).contains(column)) {
			Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_HEAVY);
			attrs.insert(attr);
		}
		return attrs;
	}
	
	static inline void column_pack_start(
		Gtk::TreeViewColumn &column, Gtk::CellRenderer &renderer,
		Gtk::TreeViewColumn::SlotTreeCellData dataFunc
	) {
		column.pack_start(renderer, true);
		column.set_cell_data_func(renderer, dataFunc);
	}
	
	static void cb__render_line(
		Gtk::CellRenderer *const cellRenderer, const Gtk::TreeIter &iter
	) {
		if (!iter) { SPDLOG_CRITICAL("{} iter is not valid!", SPDLOG_FUNCTION); return; }
		auto &renderer = *dynamic_cast<Gtk::CellRendererText*>(cellRenderer);
		
		const Pango::AttrList attrs = attributes_for_cell(iter, ColumnBit::LINE);
		renderer.property_attributes().set_value(attrs);
		
		const int line = path_to_line_index(Gtk::TreePath(iter)) + 1;
		renderer.property_text().set_value(std::to_string(line));
	}
	
	static void cb__render_name(
		Gtk::CellRenderer *const cellRenderer, const Gtk::TreeIter &iter
	) {
		if (!iter) { SPDLOG_CRITICAL("{} iter is not valid!", SPDLOG_FUNCTION); return; }
		auto &renderer = *dynamic_cast<Gtk::CellRendererText*>(cellRenderer);
		
		const Pango::AttrList attrs = attributes_for_cell(iter, ColumnBit::NAME);
		renderer.property_attributes().set_value(attrs);
		
		renderer.property_text().set_value(iter->get_value(ColumnRecord::get().name));
	}
	
	static void cb__render_duration(
		Gtk::CellRenderer *const cellRenderer, const Gtk::TreeIter &iter
	) {
		if (!iter) { SPDLOG_CRITICAL("{} iter is not valid!", SPDLOG_FUNCTION); return; }
		auto &renderer = *dynamic_cast<Gtk::CellRendererText*>(cellRenderer);
		
		const Pango::AttrList attrs = attributes_for_cell(iter, ColumnBit::DURATION);
		renderer.property_attributes().set_value(attrs);
		
		// get the value from the model and show it aligned in the view
		const chrono::seconds val = iter->get_value(ColumnRecord::get().duration);
		renderer.property_text().set_value("    " + Utils::time_to_ui_string(val));
	}
};



namespace Gui
{

using PageId = PlaylistNotebook::PageId;

[[nodiscard]] static
Container* _container_from_page_id(const PageId id)
{
	static_assert(sizeof(Container*) >= sizeof(PageId));
	return reinterpret_cast<Container*>(id);
}

[[nodiscard]] static
PageId _container_to_page_id(const Container *container)
{
	static_assert(sizeof(PageId) >= sizeof(Container*));
	return static_cast<PageId>(reinterpret_cast<std::uintptr_t>(container));
}

[[nodiscard]] static
Gtk::TreeView& _container_to_tree_view(Container &container)
{
	auto *x = dynamic_cast<Gtk::TreeView*>(container.get_child());
	assert(x != nullptr);
	return *x;
}



// PlaylistNotebook - public
// ==================================================

void PlaylistNotebook::reset_widget_text(void)
{
}

PlaylistNotebook::PlaylistNotebook(void)
{
	this->initialize_gui();
}

int PlaylistNotebook::size(void)
{
	return w_.get_n_pages();
}

NotebookPageProxy PlaylistNotebook::get_page(const PageId page)
{
	return NotebookPageProxy(page, *this);
}

NotebookPageProxy PlaylistNotebook::get_current_page(void)
{
	return NotebookPageProxy(this->current_page_id(), *this);
}

PageId PlaylistNotebook::current_page_id(void) const
{
	return _container_to_page_id(this->current_page_get_container());
}

void PlaylistNotebook::page_focus_right(void)
{
	const int i = w_.get_current_page() + 1;
	if (i == w_.get_n_pages()) {
		w_.set_current_page(0);
	}
	else {
		w_.next_page();
	}
}

void PlaylistNotebook::page_focus_left(void)
{
	const int i = w_.get_current_page();
	if (i >= 0) {
		// `set_current_page(-1)` is equivalent to `set_current_page(get_n_pages()-1)`
		w_.set_current_page(i - 1);;
	}
}

PageId PlaylistNotebook::page_create(const Glib::ustring &title, Pango::AttrList &titleAttributes)
{
	auto *container = dynamic_cast<Container*>(CreateManaged::treeViewContainer());
	assert(container != nullptr);
	
	PageId id = _container_to_page_id(container);
	{
		const int pos = w_.get_current_page() + 1;
		auto &label = *CreateManaged::pangoLabel(title, titleAttributes);
		w_.insert_page(*container, label, pos);
		w_.set_tab_reorderable(*container, true);
		w_.next_page();
		
		Gtk::TreeView &treeView = _container_to_tree_view(*container);
		treeView.set_activate_on_single_click(false);
		treeView.signal_row_activated().connect_notify(
			sigc::bind(sigc::mem_fun(*this, &PlaylistNotebook::cb__row_activated), id)
		);
	}
	
	m_signal_pageCreated.emit(id);
	return id;
}

void PlaylistNotebook::page_remove(const PageId page)
{
	Container *const container = _container_from_page_id(page);
	assert(container != nullptr);
	
	m_signal_pageRemove.emit(page);
	w_.remove_page(*container);
	m_signal_pageDestroyed.emit(page);
}

void PlaylistNotebook::page_rename(const PageId page,
	const Glib::ustring &newTitle, Pango::AttrList &titleAttributes
) {
	auto *const container = _container_from_page_id(page);
	assert(container != nullptr);
	
	auto &label = *CreateManaged::pangoLabel(newTitle, titleAttributes);
	w_.set_tab_label(*container, label);
}

auto PlaylistNotebook::signal_page_created(void) -> sigc::signal<void(PageId)>
{
	return m_signal_pageCreated;
}

auto PlaylistNotebook::signal_page_remove(void) -> sigc::signal<void(PageId)>
{
	return m_signal_pageRemove;
}

auto PlaylistNotebook::signal_page_destroyed(void) -> sigc::signal<void(PageId)>
{
	return m_signal_pageDestroyed;
}

auto PlaylistNotebook::signal_row_activated(void) -> sigc::signal<void(PageId, int, NotebookRowProxy)>
{
	return m_signal_rowActivated;
}



// PlaylistNotebook - private
// ==================================================

void PlaylistNotebook::initialize_gui(void)
{
	w_.set_scrollable(true);
	w_.show_all_children(true);
}

void PlaylistNotebook::cb__row_activated(
	const Gtk::TreePath &pathToRow, Gtk::TreeView::Column *tvc, PageId id
) {
	m_signal_rowActivated.emit(id,
		path_to_line_index(pathToRow),
		NotebookRowProxy(tvc->get_tree_view()->get_model(), pathToRow)
	);
}

const Container* PlaylistNotebook::current_page_get_container(void) const
{
	const int pos = w_.get_current_page();
	if (pos < 0) {
		return nullptr;
	}
	return dynamic_cast<const Container*>(w_.get_nth_page(pos));
}

Container* PlaylistNotebook::current_page_get_container(void)
{
	return const_cast<Container*>(
		const_cast<const PlaylistNotebook*>(this)->current_page_get_container()
	);
}




// PageProxy
// ==================================================

bool NotebookPageProxy::is_valid(void) const
{
	return _container_from_page_id(_id) != nullptr;
}

size_t NotebookPageProxy::size(void) const
{
	auto *const container = _container_from_page_id(_id);
	assert(container != nullptr);
	
	Gtk::TreeView &treeView = _container_to_tree_view(*container);
	Glib::RefPtr listStore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(treeView.get_model());
	return listStore->children().size();
}

bool NotebookPageProxy::empty(void) const
{
	auto *const container = _container_from_page_id(_id);
	assert(container != nullptr);
	
	Gtk::TreeView &treeView = _container_to_tree_view(*container);
	Glib::RefPtr listStore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(treeView.get_model());
	return listStore->children().empty();
}

Glib::ustring NotebookPageProxy::get_name(void) const
{
	auto *const container = _container_from_page_id(_id);
	assert(container != nullptr);
	
	const Gtk::Notebook *const parent = dynamic_cast<Gtk::Notebook*>(container->get_parent());
	assert(parent != nullptr);
	return parent->get_tab_label_text(*container);
}

void NotebookPageProxy::rename(
	const Glib::ustring &newTitle, Pango::AttrList &titleAttributes
) {
	_parent.page_rename(_id, newTitle, titleAttributes);
}

void NotebookPageProxy::append_row(const NotebookRowData &data) const
{
	auto *const container = _container_from_page_id(_id);
	assert(container != nullptr);
	
	Gtk::TreeView &treeView = _container_to_tree_view(*container);
	Glib::RefPtr listStore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(treeView.get_model());
	Gtk::TreeRow row = *listStore->append();
	
	row[ColumnRecord::get().name] = data.mediaName;
	row[ColumnRecord::get().duration] = data.mediaDuration;
}

void NotebookPageProxy::foreach_row(
	sigc::slot<IterFlag(long row, NotebookRowProxy rowProxy)> callback
) const {
	auto *const container = _container_from_page_id(_id);
	assert(container != nullptr);
	Gtk::TreeView &treeView = _container_to_tree_view(*container);
	
	const Glib::RefPtr<Gtk::TreeModel> model = treeView.get_model();
	const Gtk::TreeModel::Children rows = model->children();
	
	int lineIndex = 0;
	Gtk::TreeIter iter = rows.begin();
	for (const auto end = rows.end(); iter != end; ++iter, ++lineIndex) {
		const IterFlag res = callback(
			lineIndex, NotebookRowProxy(model, Gtk::TreePath(iter))
		);
		if (res == IterFlag::STOP) { break; }
	}
}

std::vector<NotebookRowProxy> NotebookPageProxy::get_rows(long firstLine, long lineCount) const
{
	assert(firstLine >= 0);
	if (lineCount <= 0) { return {}; }
	
	std::vector<NotebookRowProxy> list;
	list.reserve(std::min(this->size(), static_cast<decltype(this->size())>(lineCount)));
	this->foreach_row(
		[&](const long lineIndex, NotebookRowProxy row) -> IterFlag
		{
			if (list.size() == static_cast<size_t>(lineCount)) {
				return IterFlag::STOP;
			}
			
			if (lineIndex >= firstLine) {
				list.push_back(std::move(row));
			}
			return IterFlag::NEXT;
		}
	);
	return list;
}

void NotebookPageProxy::mark_rows(NotebookColBit column) const
{
	this->foreach_row(
		[column](const long /*line*/, NotebookRowProxy row) -> IterFlag
		{
			row.set_marked(column);
			return IterFlag::NEXT;
		}
	);
}



// NotebookRowProxy
// ==================================================

void NotebookRowProxy::toggle_marked(const NotebookColBit column)
{
	const Gtk::TreeRow row = row_from_ref(_ref);
	Gtk::TreeValueProxy proxy = row[ColumnRecord::get().markedColumns];
	proxy = proxy | column;
}
NotebookColBit NotebookRowProxy::get_marked(void)
{
	return row_from_ref(_ref).get_value(ColumnRecord::get().markedColumns);
}
void NotebookRowProxy::set_marked(const NotebookColBit column)
{
	row_from_ref(_ref).set_value(ColumnRecord::get().markedColumns, column);
}

Glib::ustring NotebookRowProxy::get_name(void)
{
	return row_from_ref(_ref).get_value(ColumnRecord::get().name);
}
void NotebookRowProxy::set_name(const Glib::ustring &name)
{
	row_from_ref(_ref).set_value(ColumnRecord::get().name, name);
}

chrono::seconds NotebookRowProxy::get_duration(void)
{
	return row_from_ref(_ref).get_value(ColumnRecord::get().duration);
}
void NotebookRowProxy::set_duration(const chrono::seconds duration)
{
	row_from_ref(_ref).set_value(ColumnRecord::get().duration, duration);
}

}



namespace CreateManaged
{

Gtk::Container* treeViewContainer(void)
{
	auto *wnd = Gtk::make_managed<Gtk::ScrolledWindow>();
	wnd->add(*Gtk::make_managed<PlaylistTreeView>());
	wnd->show_all();
	return wnd;
}

}
