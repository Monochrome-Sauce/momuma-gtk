#ifndef APPLICATION_H
#define APPLICATION_H

#include <gtkmm/application.h>
#include <momuma/momuma.h>

#include "Gui.h"
#include "Pages.h"


using PlayerState = Momuma::MpvPlayer::State;


class Application final : public Gtk::Application
{
public:
	const Glib::ustring _title;
	
	bool add_playlist_to_view(const Glib::ustring &playlistName);
	
	[[nodiscard]] Momuma::Momuma &get_backend(void);
	
	
	Application(void);
	
private:
	Momuma::Momuma m_backend;
	
	Glib::RefPtr<Gio::Menu> m_menubar;
	Gui::MasterWindow m_window;
	
	PageMap m_pages;
	bool m_letSliderUpdate;
	
	
	void set_accel_for_action(const Glib::ustring& actionName, const Glib::ustring& accel);
	
	void on_startup(void) override;
	void on_activate(void) override;
	
	void create_keyboard_only_shortcuts(void);
	
	[[nodiscard]]
	Glib::RefPtr<Gio::Menu> create_menu_File(void);
	
	void add_menu_item(
		Glib::RefPtr<Gio::Menu> menu,
		const Glib::ustring &label, const Glib::ustring &accel,
		const Gtk::Application::ActivateSlot& slot
	);
	
	Glib::RefPtr<Gio::SimpleAction> add_action(
		const Glib::ustring &name, const Glib::ustring &accel,
		const Gtk::Application::ActivateSlot& slot
	);
	
	void reset_title(void);
	
	void prefix_title(Glib::ustring prefix);
	
	void on_action_quit(void);
	void on_action_openPlaylist(void);
	void on_action_closePlaylist(void);
	void on_action_importSong(void);
	
	
	bool cb__window_keypress(const GdkEventKey *const event);
	
	void cb__row_activated(const PageId id, const int rowIndex, Gui::NotebookRowProxy row);
	
	// Called when the user presses/releases the `MasterWindow`'s slider
	void cb__slider_update(Gui::Slider::DragPhase phase);
	
	// Called when the `Player` starts the stream
	void cb__audioStreamStarted(Momuma::MpvPlayer &src);
	
	// Called when the `Player` reaches the end-of-stream
	void cb__audioStreamEnded(Momuma::MpvPlayer &src);
	
	// Called when the `Player` changes states
	void cb__audioStateChanged(Momuma::MpvPlayer &src,
		PlayerState prevState, PlayerState newState
	);
};

#endif /* APPLICATION_H */
