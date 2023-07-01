#include "Gui.h"


namespace Gui
{

static void set_buttons_image(PlayerControls &self)
{
	self._loop.set_image_from_icon_name("view-refresh", Gtk::ICON_SIZE_SMALL_TOOLBAR);
	self._next.set_image_from_icon_name("media-skip-forward", Gtk::ICON_SIZE_SMALL_TOOLBAR);
	self._prev.set_image_from_icon_name("media-skip-backward", Gtk::ICON_SIZE_SMALL_TOOLBAR);
	self._stop.set_image_from_icon_name("media-playback-stop", Gtk::ICON_SIZE_SMALL_TOOLBAR);
}

static void set_buttons_tooltip(PlayerControls &self)
{
	self._loop.set_tooltip_text(_("Loop playlist"));
	self._next.set_tooltip_text(_("Next song"));
	self._prev.set_tooltip_text(_("Previous song"));
	self._stop.set_tooltip_text(_("Stop player"));
}

static void set_buttons_relief(PlayerControls &self)
{
	self._loop.set_relief(Gtk::RELIEF_NONE);
	self._next.set_relief(Gtk::RELIEF_NONE);
	self._playPause.set_relief(Gtk::RELIEF_NONE);
	self._prev.set_relief(Gtk::RELIEF_NONE);
	self._stop.set_relief(Gtk::RELIEF_NONE);
	self._volume.set_relief(Gtk::RELIEF_NONE);
}


void PlayerControls::reset_widget_text(void)
{
}

PlayerControls::PlayerControls(void) :
	TopWidget { false, 3 },
	_leftBatch { true, 1 }, _rightBatch { true, 1 }
{
	set_buttons_tooltip(*this);
	set_buttons_image(*this);
	set_buttons_relief(*this);
	
	_leftBatch.pack_start(_prev, Gtk::PACK_EXPAND_WIDGET);
	_leftBatch.pack_start(_playPause, Gtk::PACK_EXPAND_WIDGET);
	_leftBatch.pack_start(_stop, Gtk::PACK_EXPAND_WIDGET);
	_leftBatch.pack_start(_next, Gtk::PACK_EXPAND_WIDGET);
	
	_rightBatch.pack_start(_loop, Gtk::PACK_EXPAND_WIDGET);
	_rightBatch.pack_start(_volume, Gtk::PACK_EXPAND_WIDGET);
	
	this->switch_to_play_button();
	w_.pack_start(_leftBatch, Gtk::PACK_SHRINK);
	w_.pack_start(_slider, Gtk::PACK_EXPAND_WIDGET);
	w_.pack_start(_rightBatch, Gtk::PACK_SHRINK);
}


void PlayerControls::switch_to_play_button(void)
{
	_playPause.set_tooltip_text(_("Play song"));
	_playPause.set_image_from_icon_name(
		"media-playback-start",
		Gtk::ICON_SIZE_SMALL_TOOLBAR
	);
	
	m_conn_playPause_clicked.disconnect();
	m_conn_playPause_clicked = _playPause.signal_clicked().connect(
		[this](void) -> void { m_signal_clickedPlay.emit(); }
	);
}

void PlayerControls::switch_to_pause_button(void)
{
	_playPause.set_tooltip_text(_("Pause song"));
	_playPause.set_image_from_icon_name("media-playback-pause", Gtk::ICON_SIZE_SMALL_TOOLBAR);
	
	m_conn_playPause_clicked.disconnect();
	m_conn_playPause_clicked = _playPause.signal_clicked().connect(
		[this](void) -> void { m_signal_clickedPause.emit(); }
	);
}



Glib::SignalProxy<void, double> PlayerControls::signal_volume_value_changed(void)
{
	return _volume.signal_value_changed();
}

Glib::SignalProxy<void> PlayerControls::signal_clicked_stop(void)
{
	return _stop.signal_clicked();
}

Glib::SignalProxy<void> PlayerControls::signal_clicked_next_song(void)
{
	return _next.signal_clicked();
}

Glib::SignalProxy<void> PlayerControls::signal_clicked_prev_song(void)
{
	return _prev.signal_clicked();
}

sigc::signal<void()> PlayerControls::signal_clicked_play(void)
{
	return m_signal_clickedPlay;
}

sigc::signal<void()> PlayerControls::signal_clicked_pause(void)
{
	return m_signal_clickedPause;
}

}
