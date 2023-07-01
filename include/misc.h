#ifndef UTILS__UTILITIES_H
#define UTILS__UTILITIES_H

#include <chrono>
#include <filesystem>
#include <gtkmm/widget.h>
#include <pangomm/attrlist.h>


namespace Utils
{

/* #Create a string for displaying the current time.
! The string format is equivalent to `"%02d:%0d2"` (minutes:seconds) when `time`
contains 0 hours, otherwise it's equivalent to `"%d:%02d:%0d2"` (hours:minutes:seconds).
! @param time: the time duration to convert into a string.
*/
[[nodiscard]]
std::string time_to_ui_string(std::chrono::seconds time);

/* #Convenience function for creating `Pango::AttrList` s.
! The attributes are inserted from first to last using `Pango::AttrList::insert()`.
! @param attrs: list of attributes to create an `AttrList` from.
*/
[[nodiscard]]
Pango::AttrList create_attr_list(std::initializer_list<Pango::Attribute> attrs);

/* Call 'slot' on the widget. If the widget is a container, the function will be called
on its children using 'Gtk::Container::foreach()'.
*/
void recursive_foreach(Gtk::Widget &widget, const sigc::slot<void, Gtk::Widget&> &slot);

std::filesystem::path get_appdata_folder(void);

[[nodiscard]] Glib::ustring app_menuLabel_to_actionName(const Glib::ustring &menuLabel);

}

#endif /* UTILS__UTILITIES_H */
