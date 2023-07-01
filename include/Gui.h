#ifndef GUI_H
#define GUI_H

#include <glib/gi18n.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/togglebutton.h>
#include <momuma/sigc.h>
#include <tuple>

#include "Gui/PlaylistNotebook.h"
#include "Gui/TopWidget.h"
#include "Gui/VolumeButton.h"
#include "Gui/functions.h"


namespace Gui
{

struct ListChooserDialog final : public TopWidget<Gtk::Dialog>
{
public:
	void reset_widget_text(void);
	
	ListChooserDialog(const Glib::ustring &title, Gtk::Window &parent);
	
	/* #Calls a Gtk::Dialog's 'run()' function.
	! @return: Gtk::RESPONSE_ACCEPT when an item is chosen. Any other return
	value should be treated as a leave without selection.
	*/
	Gtk::ResponseType run(void);
	
	/* #Add a new row at the end of the list.
	! @param value: the new text for the new row.
	! @return: the number of the row added.
	*/
	unsigned int append_value(const Glib::ustring &value);
	
	[[nodiscard]] Glib::ustring get_selected_value(void);
	
	Gtk::ListViewText _list;
	
private:
	void cb__keypress(const GdkEventKey *event);
};



struct Slider final : public TopWidget<Gtk::HBox>
{
	void reset_widget_text(void);
	
	Slider(void);
	
	// set the slider's current time
	void set_time(std::chrono::milliseconds time);
	
	// set the slider's upper time limit (bottom limit is always 0)
	void set_time_limit(std::chrono::milliseconds time);
	
	// get the slider's current time (unaffected by 'Slider::enable()')
	[[nodiscard]] std::chrono::milliseconds get_time(void) const;
	
	// get the slider's upper time limit (bottom limit is always 0)
	[[nodiscard]] std::chrono::milliseconds get_time_limit(void) const;
	
	// make the slider [non-]interactive and zero the *displayed* time
	void set_sensitive(bool enable);
	
	//[[nodiscard]] sigc::signal<void()> signal_press(void);
	//[[nodiscard]] sigc::signal<void()> signal_release(void);
	
	enum class DragPhase { BEGIN, END };
	[[nodiscard]] sigc::signal<void(DragPhase)> signal_drag(void);
	
	Gtk::Scale _scale;
	Gtk::Label _timeDisplay;
	Gtk::Popover _mouseHover;
	
private:
	void sync_display_to_scale(void);
	
	void connect_notify_event_signals(void);
	
	void connect_scale_button_signals(void);
	
	//sigc::signal<void()> m_signal_press;
	//sigc::signal<void()> m_signal_release;
	sigc::signal<void(DragPhase)> m_signal_drag;
	
	// use to handle multi-clicks (which send only 1 release signal)
	DragPhase m_lastDragPhase;
};



struct PlayerControls final : public TopWidget<Gtk::HBox>
{
	void reset_widget_text(void);
	
	PlayerControls(void);
	
	// set the w_playPause button to the PLAY state
	void switch_to_play_button(void);
	
	// set the w_playPause button to the PAUSE state
	void switch_to_pause_button(void);
	
	Gtk::HBox _leftBatch;
	Gtk::HBox _rightBatch;
	Gtk::Button _next;
	Gtk::Button _playPause;
	Gtk::Button _prev;
	Gtk::Button _stop;
	Gtk::ToggleButton _loop;
	Slider _slider;
	VolumeButton _volume;
	
	
	[[nodiscard]]
	Glib::SignalProxy<void, double> signal_volume_value_changed(void);
	
	[[nodiscard]]
	Glib::SignalProxy<void> signal_clicked_stop(void);
	
	[[nodiscard]]
	Glib::SignalProxy<void> signal_clicked_next_song(void);
	
	[[nodiscard]]
	Glib::SignalProxy<void> signal_clicked_prev_song(void);
	
	[[nodiscard]]
	sigc::signal<void()> signal_clicked_play(void);
	
	[[nodiscard]]
	sigc::signal<void()> signal_clicked_pause(void);
	
	
private:
	sigc::connection m_conn_playPause_clicked;
	sigc::signal<void()> m_signal_clickedPlay, m_signal_clickedPause;
};



struct MasterWindow final : public Gtk::ApplicationWindow, TopWidget<Gtk::VBox>
{
	void reset_widget_text(void);
	
	MasterWindow(void);
	
	PlayerControls _controls;
	PlaylistNotebook _notebook;
};

}

#endif /* GUI_H */
