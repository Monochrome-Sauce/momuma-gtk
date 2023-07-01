#include "Gui.h"


namespace Gui
{

void MasterWindow::reset_widget_text(void)
{
}

MasterWindow::MasterWindow(void) :
	TopWidget { false, 0 }
{
	w_.pack_start(_notebook, Gtk::PACK_EXPAND_WIDGET, 0);
	w_.pack_start(_controls, Gtk::PACK_SHRINK, 0);
	this->add(w_);
	this->set_default_size(800, 600);
}

}
