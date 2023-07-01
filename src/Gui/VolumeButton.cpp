#include <cassert>
#include <momuma/sigc.h>
#include <momuma/spdlog.h>

#include "Gui/VolumeButton.h"


namespace Gui
{

static void set_volume_scale_properties(Gtk::Scale &volumeScale)
{
	volumeScale.set_draw_value(true);
	volumeScale.set_increments(0.01 * VolumeButton::VOL_SCALE, 0.1 * VolumeButton::VOL_SCALE);
	volumeScale.set_range(VolumeButton::VOL_MIN, VolumeButton::VOL_MAX);
	volumeScale.set_value(VolumeButton::VOL_MIN);
}

static Glib::ustring volume_to_string(const double volume)
{
	assert(volume <= VolumeButton::VOL_MAX);
	assert(volume >= VolumeButton::VOL_MIN);
	// scale `volume` to fit into the range [0,100] for clean formatting
	return Glib::ustring::sprintf("%.0f%%", volume * (100.0 / VolumeButton::VOL_SCALE));
}


void VolumeButton::reset_widget_text(void)
{
}

VolumeButton::VolumeButton(void)
{
	static_assert(VOL_MAX - VOL_MIN == VOL_SCALE);
	static_assert(VOL_MAX <= 1000.0);
	static_assert(VOL_MAX > VOL_MIN);
	static_assert(VOL_MIN == 0);
	
	Gtk::Scale &scale = this->get_widget_scale();
	scale.signal_format_value().connect(&volume_to_string);
	set_volume_scale_properties(scale);
}

Gtk::Box& VolumeButton::get_widget_box(void)
{
	return *dynamic_cast<Gtk::Box*>(this->get_widget_popup().get_child());
}

Gtk::Button& VolumeButton::get_widget_minus_button(void)
{
	return *dynamic_cast<Gtk::Button*>(this->get_minus_button());
}

Gtk::Button& VolumeButton::get_widget_plus_button(void)
{
	return *dynamic_cast<Gtk::Button*>(this->get_plus_button());
}

Gtk::Popover& VolumeButton::get_widget_popup(void)
{
	return *dynamic_cast<Gtk::Popover*>(this->get_popup());
}

Gtk::Scale& VolumeButton::get_widget_scale(void)
{
	return *dynamic_cast<Gtk::Scale*>(get_widget_box().get_children()[1]);
}

}
