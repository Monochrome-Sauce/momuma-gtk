#include <fmt/format.h>
#include <glibmm/miscutils.h>
#include <gtkmm/container.h>
#include <momuma/spdlog.h>

#include "misc.h"


namespace Utils
{

std::string time_to_ui_string(const std::chrono::seconds time)
{
	namespace chrono = std::chrono;
	const auto hours = chrono::duration_cast<chrono::hours>(time);
	const auto minutes = chrono::duration_cast<chrono::minutes>(time) - hours;
	const auto seconds = chrono::duration_cast<chrono::seconds>(time) - hours - minutes;
	
	std::string s;
	if (hours > chrono::hours(0)) {
		s += fmt::format("{:d}:", hours.count());
	}
	return s + fmt::format("{:02d}:{:02d}", minutes.count(), seconds.count());
}

Pango::AttrList create_attr_list(std::initializer_list<Pango::Attribute> attrs)
{
	Pango::AttrList list;
	for (auto attr : attrs) {
		list.insert(attr);
	}
	return list;
}

void recursive_foreach(Gtk::Widget &widget, const sigc::slot<void, Gtk::Widget&> &slot)
{
	slot(widget);
	auto *const container = dynamic_cast<Gtk::Container*>(&widget);
	if (container != nullptr) {
		container->foreach(sigc::bind(&recursive_foreach, sigc::ref(slot)));
	}
}

fs::path get_appdata_folder(void)
{
	// Windows and MacOS config folders:
	//     https://softwareengineering.stackexchange.com/a/299872
#if OS_LINUX
	const fs::path folder = fs::path(Glib::get_home_dir()) / ".config";
#else
	#warning "Only linux is supported"
#endif
	if (!fs::exists(folder)) {
		throw std::runtime_error("Failed to locate " + folder.string());
	}
	return folder;
}

Glib::ustring app_menuLabel_to_actionName(const Glib::ustring &menuLabel)
{
	Glib::ustring actionName;
	actionName.reserve(menuLabel.size() + 8 /* for ensurance */);
	
	for (Glib::ustring::value_type c : menuLabel.lowercase()) {
		actionName.push_back((c == ' ' ? '-' : c));
	}
	return actionName;
}

}
