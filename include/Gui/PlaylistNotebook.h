#ifndef GUI__PLAYLIST_NOTEBOOK_H
#define GUI__PLAYLIST_NOTEBOOK_H

#include <gtkmm/notebook.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treerowreference.h>
#include <gtkmm/treeview.h>
#include <momuma/enum_operators.h>

#include "Gui/TopWidget.h"


namespace Gui
{

class NotebookPageProxy;
class NotebookRowProxy;

struct NotebookRowData
{
	Glib::ustring mediaName;
	std::chrono::seconds mediaDuration;
};

enum class NotebookColBit : uint32_t
{
	None = 0, All = (1ULL << (sizeof (NotebookColBit) * CHAR_BIT)) - 1,
	LINE = (1 << 0),
	NAME = (1 << 1),
	DURATION = (1 << 2),
};
ENUM_DEFINE_BITWISE_OPS(NotebookColBit)


class PlaylistNotebook : public TopWidget<Gtk::Notebook>
{
public:
	using PageProxy = NotebookPageProxy;
	using RowProxy = NotebookRowProxy;
	using RowData = NotebookRowProxy;
	
	// uid type to interact with notebook pages
	enum class PageId : std::uintptr_t { Null = NULL };
	
	
	void reset_widget_text(void);
	
	PlaylistNotebook(void);
	
	// Get the number of pages in a notebook.
	[[nodiscard]] int size(void);
	
	[[nodiscard]] PageProxy get_page(PageId page);
	
	[[nodiscard]] PageProxy get_current_page(void);
	
	[[nodiscard]] PageId current_page_id(void) const;
	
	/* #Focuses on the page right of the focused one.
	! Does nothing when there are no pages.
	! Wraps to the left-most page when the right-most page is focused.
	*/
	void page_focus_right(void);
	
	/* #Focuses on the page left of the focused one.
	! Does nothing when there are no pages.
	! Wraps to the right-most page when the left-most page is focused.
	*/
	void page_focus_left(void);
	
	/* #Creates a new page.
	! The new page is added to the right of the currently focused page, and steals its focus.
	!
	! @param title: a title for the page.
	! @param titleAttributes: Pango markup attributes for `title`.
	! @return: id of the newly created page.
	*/
	PageId page_create(const Glib::ustring &title, Pango::AttrList &titleAttributes);
	
	/* #Remove the given page.
	! @param page: the page to remove.
	*/
	void page_remove(PageId page);
	
	/* #Rename the given page.
	! @param page: the page to rename.
	! @param newTitle: a new title for the page.
	! @param titleAttributes: Pango markup attributes for `newTitle`.
	*/
	void page_rename(PageId page,
		const Glib::ustring &newTitle, Pango::AttrList &titleAttributes
	);
	
	// #Convenience wrapper for `create_page()`.
	[[nodiscard]] inline
	PageId page_create(const Glib::ustring &title)
	{
		Pango::AttrList a;
		return this->page_create(title, a);
	}
	
	
	// Emitted after the page is created and added
	[[nodiscard]] sigc::signal<void(PageId)> signal_page_created(void);
	
	// Emitted before the page is destroyed
	[[nodiscard]] sigc::signal<void(PageId)> signal_page_remove(void);
	
	// Emitted after the page is destroyed
	[[nodiscard]] sigc::signal<void(PageId)> signal_page_destroyed(void);
	
	[[nodiscard]]
	sigc::signal<void(PageId, int rowIndex, NotebookRowProxy)> signal_row_activated(void);
	
private:
	sigc::signal<void(PageId)> m_signal_pageCreated, m_signal_pageRemove, m_signal_pageDestroyed;
	sigc::signal<void(PageId, int rowIndex, RowProxy)> m_signal_rowActivated;
	
	void initialize_gui(void);
	
	void cb__row_activated(
		const Gtk::TreeModel::Path &rowPath, Gtk::TreeView::Column *tvc, PageId id
	);
	
	// Returns `nullptr` when `notebook` has no pages
	[[nodiscard]]
	const Gtk::ScrolledWindow* current_page_get_container(void) const;
	
	// Returns `nullptr` when `notebook` has no pages
	[[nodiscard]]
	Gtk::ScrolledWindow* current_page_get_container(void);
};


class NotebookPageProxy
{
public:
	PlaylistNotebook::PageId _id;
	PlaylistNotebook &_parent;
	
	NotebookPageProxy(decltype(_id) id, decltype(_parent) &parent) :
		_id { id }, _parent { parent }
	{}
	
	[[nodiscard]] bool is_valid(void) const;
	
	// Number of rows in the page.
	[[nodiscard]] size_t size(void) const;
	
	// Equivalent to `PageProxy::size() == 0`.
	[[nodiscard]] bool empty(void) const;
	
	[[nodiscard]]
	Glib::ustring get_name() const;
	
	/* #Rename the given page.
	! @param page: the page to rename.
	! @param newTitle: a new title for the page.
	! @param titleAttributes: Pango markup attributes for `newTitle`.
	*/
	void rename(const Glib::ustring &newTitle, Pango::AttrList &titleAttributes);
	
	void append_row(const NotebookRowData &data) const;
	
	enum class IterFlag : bool { STOP = false, NEXT = !STOP };
	/* #Execute a callback for each row in the page.
	*/
	void foreach_row(sigc::slot<IterFlag(long row, NotebookRowProxy rowProxy)> callback) const;
	
	/* #Get a proxy for each row.
	! @param firstLine: the line *index* to start getting rows from. Must NOT be negative.
	! @param lineCount: the number of lines to get. 0 or a negative number are a no-op.
	! @return: a vector of proxies to the lines. A proxy is invalidated if the row is removed.
	*/
	[[nodiscard]]
	std::vector<NotebookRowProxy> get_rows(long firstLine = 0, long lineCount = INT_MAX) const;
	
	void mark_rows(NotebookColBit column) const;
	
	
	// #Convenience wrapper for `rename()`.
	inline void rename(const Glib::ustring &title)
	{
		Pango::AttrList a;
		this->rename(title, a);
	}
};


class NotebookRowProxy
{
public:
	Gtk::TreeRowReference _ref;
	
	NotebookRowProxy(const Glib::RefPtr<Gtk::TreeModel> &model, const Gtk::TreePath &path) :
		_ref { model, path }
	{}
	
	NotebookRowProxy(NotebookRowProxy &&other) : _ref { std::move(other._ref) } {}
	
	NotebookRowProxy(const NotebookRowProxy &other) : _ref { other._ref } {}
	
	
	void toggle_marked(NotebookColBit column);
	void set_marked(NotebookColBit column);
	[[nodiscard]] NotebookColBit get_marked(void);
	
	void set_name(const Glib::ustring&);
	[[nodiscard]] Glib::ustring get_name(void);
	
	void set_duration(std::chrono::seconds);
	[[nodiscard]] std::chrono::seconds get_duration(void);
};

}

#endif /* GUI__PLAYLIST_NOTEBOOK_H */
