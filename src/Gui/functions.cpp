#include <gdk/gdkkeysyms.h>
#include <gtkmm/messagedialog.h>
#include <gdkmm/seat.h>

#include "Gui.h"


namespace Gui
{

void display_msg_box(Gtk::Window &parent, const Glib::ustring &msg)
{
	Gtk::MessageDialog msgBox(parent, msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
	msgBox.run();
}

Gdk::Point get_relative_pointer_position(const Gtk::Widget &src)
{
	const Glib::RefPtr<const Gdk::Window> wnd = src.get_window();
	const Glib::RefPtr<const Gdk::Seat> seat = src.get_display()->get_default_seat();
	
	Gdk::ModifierType mask = { };
	int x = 0, y = 0;
	wnd->get_device_position(seat->get_pointer(), x, y, mask);
	x -= src.get_allocation().get_x();
	y -= src.get_allocation().get_y();
	
	return { x, y };
}


guint translate_keysm(const guint keyval)
{
	switch (keyval)
	{
		case GDK_KEY_Tab:
		case GDK_KEY_KP_Tab:
		case GDK_KEY_ISO_Left_Tab:
		case GDK_KEY_3270_BackTab:
			return GDK_KEY_Tab;
	}
	return keyval;
}

}
