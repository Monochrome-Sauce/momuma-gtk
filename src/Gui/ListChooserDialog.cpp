#include <gtkmm/scrolledwindow.h>
#include <momuma/bitset.h>
#include <momuma/spdlog.h>

#include "Gui.h"


namespace Gui
{

void ListChooserDialog::reset_widget_text(void)
{
}

ListChooserDialog::ListChooserDialog(const Glib::ustring &title, Gtk::Window &parent) :
	TopWidget { title, parent,
		Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_USE_HEADER_BAR
	},
	_list { 1, false, Gtk::SELECTION_SINGLE }
{
	_list.set_headers_visible(false);
	
	auto &scrollWnd = *Gtk::make_managed<Gtk::ScrolledWindow>();
	scrollWnd.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	scrollWnd.add(_list);
	
	w_.get_content_area()->pack_start(scrollWnd, Gtk::PACK_EXPAND_WIDGET, 0);
	w_.set_default_size(300, 400);
	w_.signal_key_press_event().connect(
		sigc::bind_return(
			sigc::mem_fun(*this, &ListChooserDialog::cb__keypress),
			sigc::PROPAGATE
		), sigc::BEFORE
	);
	w_.show_all_children(true);
}

Gtk::ResponseType ListChooserDialog::run(void)
{
	return static_cast<Gtk::ResponseType>(w_.run());
}

unsigned int ListChooserDialog::append_value(const Glib::ustring &value)
{
	return _list.append(value);
}

Glib::ustring ListChooserDialog::get_selected_value(void)
{
	const int i = _list.get_selected().at(0);
	return _list.get_text(static_cast<guint>(i));
}

void ListChooserDialog::cb__keypress(const GdkEventKey *const event)
{
	constexpr auto GDK_MASKS_TO_IGNORE = GDK_CONTROL_MASK | GDK_SHIFT_MASK;
	if (Momuma::Bitset(event->state).contains(GDK_MASKS_TO_IGNORE)) { return; }
	
	switch (event->keyval)
	{
	case GDK_KEY_Return:
		if (!_list.get_selected().empty()) {
			w_.response(Gtk::RESPONSE_ACCEPT);
		}
		break;
	case GDK_KEY_Escape:
		w_.close();
		break;
	}
}

}
