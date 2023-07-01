#ifndef GUI__VOLUME_BUTTON_H
#define GUI__VOLUME_BUTTON_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/popover.h>
#include <gtkmm/scale.h>
#include <gtkmm/volumebutton.h>


namespace Gui
{

struct VolumeButton final : public Gtk::VolumeButton
{
	// arbitrary value to use to change the intensity of the volume at compile-time
	constexpr static double VOL_SCALE = 75.0;
	constexpr static double VOL_MAX = 1.0 * VOL_SCALE;
	constexpr static double VOL_MIN = 0.0;
	
	void reset_widget_text(void);
	
	VolumeButton(void);
	
	Gtk::Box& get_widget_box(void);
	Gtk::Button& get_widget_minus_button(void);
	Gtk::Button& get_widget_plus_button(void);
	Gtk::Popover& get_widget_popup(void);
	Gtk::Scale& get_widget_scale(void);
};

}

#endif /* GUI__VOLUME_BUTTON_H */
