#include <cassert>
#include <momuma/spdlog.h>

#include "Gui.h"
#include "misc.h"


namespace Gui
{

static Glib::ustring time_position_to_string(chrono::seconds current, chrono::seconds finalPos)
{
	return Utils::time_to_ui_string(current) + "/" + Utils::time_to_ui_string(finalPos);
}

static double calc_scale_upper_limit(const double limit)
{
	return std::max(limit, 0.01);
}

static void set_scale_properties(Gtk::Scale &scale)
{
	scale.set_draw_value(false);
	scale.set_increments(1, 10.0);
	scale.set_range(0, calc_scale_upper_limit(0)); // set it to a minimum value so it's always shown
	scale.set_valign(Gtk::ALIGN_CENTER);
	scale.set_value_pos(Gtk::POS_TOP);
}


void Slider::reset_widget_text(void)
{
}

Slider::Slider(void) :
	TopWidget { false, 5 },
	_scale { Gtk::ORIENTATION_HORIZONTAL },
	_mouseHover { _scale },
	m_lastDragPhase { DragPhase::END }
{
	set_scale_properties(_scale);
	this->sync_display_to_scale();
	
	this->connect_notify_event_signals();
	this->connect_scale_button_signals();
	_scale.signal_value_changed().connect(
		sigc::mem_fun(*this, &Slider::sync_display_to_scale)
	);
	
	_mouseHover.set_modal(false);
	_mouseHover.set_relative_to(w_);
	_mouseHover.add_label("");
	{
		auto *const label = dynamic_cast<Gtk::Label*>(_mouseHover.get_child());
		assert(label != nullptr);
		label->set_justify(Gtk::JUSTIFY_CENTER);
	}
	
	w_.pack_start(_scale, Gtk::PACK_EXPAND_WIDGET);
	w_.pack_start(_timeDisplay, Gtk::PACK_SHRINK);
}

void Slider::set_time(chrono::milliseconds time)
{
	const chrono::duration<double> val(time);
	_scale.set_value(val.count());
}

void Slider::set_time_limit(chrono::milliseconds time)
{
	const chrono::duration<double> val(time);
	_scale.get_adjustment()->set_upper(calc_scale_upper_limit(val.count()));
}

chrono::milliseconds Slider::get_time(void) const
{
	const chrono::duration<double> val(_scale.get_value());
	return chrono::duration_cast<chrono::milliseconds>(val);
}

chrono::milliseconds Slider::get_time_limit(void) const
{
	const chrono::duration<double> val(_scale.get_adjustment()->get_upper());
	return chrono::duration_cast<chrono::milliseconds>(val);
}

void Slider::set_sensitive(bool sensitive)
{
	_scale.set_sensitive(sensitive);
	if (sensitive) {
		this->sync_display_to_scale();
	}
	else {
		using sec = chrono::seconds;
		_timeDisplay.set_text(time_position_to_string(sec(0), sec(0)));
	}
}

void Slider::sync_display_to_scale(void)
{
	_timeDisplay.set_text(time_position_to_string(
		chrono::duration_cast<chrono::seconds>(this->get_time()),
		chrono::duration_cast<chrono::seconds>(this->get_time_limit())
	));
}

void Slider::connect_notify_event_signals(void)
{
	_scale.signal_enter_notify_event().connect_notify(
		[this](GdkEventCrossing*) -> void { _mouseHover.show(); }
	);
	_scale.signal_leave_notify_event().connect_notify(
		[this](GdkEventCrossing*) -> void { _mouseHover.hide(); }
	);
	_scale.signal_motion_notify_event().connect_notify(
		[this](GdkEventMotion*) -> void
		{
			// move the popover to where the mouse-cursor is
			const int x = Gui::get_relative_pointer_position(_scale).get_x();
			_mouseHover.set_pointing_to(Gdk::Rectangle(x, 0, 0, 0));
			
			// approximate the duration based on the popover's location
			const auto value = chrono::duration_cast<chrono::seconds>(
				(this->get_time_limit() * x) / _scale.get_width()
			);
			auto &label = dynamic_cast<Gtk::Label&>(*_mouseHover.get_child());
			label.set_text(Utils::time_to_ui_string(value));
		}
	);
}

void Slider::connect_scale_button_signals(void)
{
	_scale.signal_button_press_event().connect_notify(
		[this](GdkEventButton*) -> void
		{
			if (m_lastDragPhase == DragPhase::BEGIN) { return; }
			SPDLOG_TRACE("Slider pressed");
			m_lastDragPhase = DragPhase::BEGIN;
			m_signal_drag.emit(m_lastDragPhase);
		}
	);
	
	_scale.signal_button_release_event().connect_notify(
		[this](GdkEventButton*) -> void
		{
			if (m_lastDragPhase == DragPhase::END) { return; }
			SPDLOG_TRACE("Slider released");
			m_lastDragPhase = DragPhase::END;
			m_signal_drag.emit(m_lastDragPhase);
		}
	);
}

sigc::signal<void(Slider::DragPhase)> Slider::signal_drag(void)
{
	return m_signal_drag;
}

}
