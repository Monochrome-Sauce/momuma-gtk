#ifndef GUI__FUNCTIONS_H
#define GUI__FUNCTIONS_H

#include <gtkmm/window.h>

namespace Gui
{

/* #Display a Gtk error dialog box with the message `msg`.
*/
void display_msg_box(Gtk::Window &parent, const Glib::ustring &msg);

/* #Get the mouse-pointer coordinates relative to the position of a widget.
! The (0,0) point is the `src` widget's top-left, the positive X and Y axes go
east/right and south/down respectively.
! @param src: the relative source of the coordinates.
! @return: `Gdk::Point` containing the `x` and `y` coordinates.
*/
[[nodiscard]]
Gdk::Point get_relative_pointer_position(const Gtk::Widget &src);

/* #Translate a duplicate gdkkeysyms keyval into a single proper one.
! e.g `GDK_KEY_Tab`, `GDK_KEY_KP_Tab`, `GDK_KEY_ISO_Left_Tab` and `GDK_KEY_3270_BackTab`
will all translate into `GDK_KEY_Tab`.
*/
[[nodiscard]] guint translate_keysm(const guint keyval);

}

#endif /* GUI__FUNCTIONS_H */
