#include <momuma/spdlog.h>

#include "Application.h"


namespace MomumaGtk
{
	extern Application *app; // declaration, required for proper linking
	Application *app;
}

int main(int argc, char **argv)
{
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	
	spdlog::flush_every(chrono::seconds(3));
	spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
	
	if (!Momuma::init()) {
		SPDLOG_CRITICAL("Failed to initiate Momuma");
	}
	Application app;
	
	// Gtk causes the UI to mirror whenever the locale (or `LANGUAGE` env
	// variable) is set to a RTL language.
	// Needs to be called after the Gtk::Application is created.
	Gtk::Widget::set_default_direction(Gtk::TEXT_DIR_LTR);
	
	MomumaGtk::app = &app;
	return app.run(argc, argv);
}
