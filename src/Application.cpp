#include <glibmm/miscutils.h>
#include <glibmm/main.h>
#include <momuma/bitset.h>
#include <momuma/spdlog.h>

#include "Application.h"
#include "misc.h"

#include "build-config.h"


constexpr char APP_ACTION_PREFIX[] = "app.";


static void change_slider_times(Gui::Slider &slider,
	chrono::milliseconds position, chrono::milliseconds duration
) {
	slider.set_time(position);
	slider.set_time_limit(duration);
}

static void update_controls_state(Gui::PlayerControls &ctrls, const PlayerState state)
{
	switch (state)
	{
	case PlayerState::STOP:
		change_slider_times(ctrls._slider, chrono::seconds(0), chrono::seconds(0));
		/* fallthrough */
	case PlayerState::PAUSE:
		ctrls.switch_to_play_button();
		break;
	case PlayerState::PLAY:
		ctrls.switch_to_pause_button();
		break;
	default:
		throw std::logic_error("Invalid enum value");
	}
	
	const bool b = (state != PlayerState::STOP);
	ctrls._playPause.set_sensitive(b);
	ctrls._slider.set_sensitive(b);
	ctrls._stop.set_sensitive(b);
}

static void update_slider_using_player(Gui::Slider &slider, Momuma::MpvPlayer &player)
{
	//SPDLOG_CRITICAL("blockPlayerUpdate: {}", m_blockPlayerUpdate);
	const PlayerState state = player.get_state();
	if (state != PlayerState::PLAY) { return; }
	
	mpv_error errPos, errDur;
	const chrono::microseconds position = player.get_position(errPos);
	const chrono::microseconds duration = player.get_duration(errDur);
	
	if (errPos == MPV_ERROR_PROPERTY_UNAVAILABLE && errDur == MPV_ERROR_PROPERTY_UNAVAILABLE) {
		return;
	}
	else if (errPos != MPV_ERROR_SUCCESS || errDur != MPV_ERROR_SUCCESS) {
		SPDLOG_ERROR("Position error: {} | Duration error: {}", errPos, errDur);
		return;
	}
	
	change_slider_times(slider,
		chrono::duration_cast<chrono::milliseconds>(position),
		chrono::duration_cast<chrono::milliseconds>(duration)
	);
}

static void connect_timeout_signals(Momuma::MpvPlayer &player,
	Gui::Slider &playerSlider, const volatile bool &letSliderUpdate
) {
	Glib::signal_timeout().connect(
		[&letSliderUpdate, &playerSlider, &player](void) -> bool
		{
			if (letSliderUpdate) { update_slider_using_player(playerSlider, player); }
			return true;
		}, chrono::milliseconds(150).count()
	);
	
	Glib::signal_timeout().connect(
		[&player](void) -> bool
		{
			player.wait_event(chrono::seconds(0));
			return true;
		}, chrono::milliseconds(20).count(), Glib::PRIORITY_DEFAULT_IDLE
	);
}

[[nodiscard]] static inline
std::optional<Glib::ustring> get_playlist_choice_from_user(
	Gtk::Window &parent, Momuma::Database::Sqlite3 &database
) {
	Gui::ListChooserDialog dialog(_("Choose playlist"), parent);
	const auto cb = [&dialog](Glib::ustring playlist) -> Momuma::Database::IterFlag
	{
		dialog.append_value(std::move(playlist));
		return Momuma::Database::IterFlag::NEXT;
	};
	
	if (database.get_playlists(cb) < 0) {
		Gui::display_msg_box(parent, _("Failed to retrieve playlists"));
		return std::optional<Glib::ustring>();
	}
	return dialog.run() == Gtk::RESPONSE_ACCEPT ?
		dialog.get_selected_value() : std::optional<Glib::ustring>();
}

// Called when the PLAY button is clicked
static void cb__play(Momuma::MpvPlayer &player)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
	player.set_play(true);
}

// Called when the PAUSE button is clicked
static void cb__pause(Momuma::MpvPlayer &player)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
	player.set_play(false);
}

// Called when the STOP button is clicked
static void cb__stop(Momuma::MpvPlayer &player)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
	player.stop_playback();
}


// public
// ==================================================

Application::Application(void) :
	Gtk::Application { APPLICATION_ID },
	_title { APPLICATION_TITLE },
	m_backend { Utils::get_appdata_folder() / MOMUMA_GTK__NAME },
	m_menubar { Gio::Menu::create() }, m_window { },
	m_pages { }, m_letSliderUpdate { false }
{
	if (!m_backend) {
		throw std::runtime_error("Failed to initialize Momuma backend");
	}
	
	Glib::set_application_name(_title);
	
	auto &player = m_backend.get_player();
	player.signal_streamStarted.connect(
		sigc::mem_fun(*this, &Application::cb__audioStreamStarted)
	);
	player.signal_streamEnded.connect(
		sigc::mem_fun(*this, &Application::cb__audioStreamEnded)
	);
	player.signal_stateChanged.connect(
		sigc::mem_fun(*this, &Application::cb__audioStateChanged)
	);
	
	Gui::PlayerControls &ctrls = m_window._controls;
	update_controls_state(ctrls, PlayerState::STOP);
	ctrls.signal_clicked_play().connect(sigc::bind(&cb__play, sigc::ref(player)));
	ctrls.signal_clicked_pause().connect(sigc::bind(&cb__pause, sigc::ref(player)));
	ctrls.signal_clicked_stop().connect(sigc::bind(&cb__stop, sigc::ref(player)));
	ctrls.signal_volume_value_changed().connect(
		sigc::mem_fun(player, &Momuma::MpvPlayer::set_volume)
	);
	ctrls._slider.signal_drag().connect(
		sigc::mem_fun(*this, &Application::cb__slider_update)
	);
	ctrls._volume.set_value(Gui::VolumeButton::VOL_MAX);
	
	m_window.signal_key_press_event().connect(
		sigc::mem_fun(*this, &Application::cb__window_keypress), sigc::BEFORE
	);
	
	// the order matters when connecting slots to the `row_activated()` signal.
	m_window._notebook.signal_row_activated().connect(
		sigc::mem_fun(*this, &Application::cb__row_activated)
	);
	m_pages.connect_row_activated(m_window._notebook);
	m_pages.connect_page_destroyed(m_window._notebook);
	
	connect_timeout_signals(player, ctrls._slider, m_letSliderUpdate);
	m_window.show_all_children(true);
}



// private
// ==================================================

void Application::set_accel_for_action(const Glib::ustring &actionName, const Glib::ustring &accel)
{
	Gtk::Application::set_accel_for_action(APP_ACTION_PREFIX + actionName, accel);
}

void Application::on_startup(void)
{
	Gtk::Application::on_startup(); // mandatory - do NOT remove
	
	this->create_keyboard_only_shortcuts();
	
	m_menubar->append_submenu(_("File"), this->create_menu_File());
	this->set_menubar(m_menubar);
}

void Application::on_activate(void)
{
	try {
		this->add_window(m_window);
		m_window.present();
	}
	catch (const Glib::Error &ex) {
		SPDLOG_CRITICAL(ex.what());
	}
	catch (const std::exception &ex) {
		SPDLOG_CRITICAL(ex.what());
	}
}

void Application::create_keyboard_only_shortcuts(void)
{
	/*this->add_action("new-tab", "<Primary>N", [this](void) -> void {
		m_window._view.page_create();
	});
	this->add_action("close-tab", "<Primary>W", [this](void) -> void {
		m_window._view.current_page_remove();
	});
	this->add_action("shift-right", "<Primary>T", [this](void) -> void {
		m_window._view.page_focus_right();
	});
	this->add_action("shift-left", "<Primary><Shift>T", [this](void) -> void {
		m_window._view.page_focus_left();
	});*/
}

Glib::RefPtr<Gio::Menu> Application::create_menu_File(void)
{
	Glib::RefPtr menu = Gio::Menu::create();
	
	add_menu_item(menu, _("Quit"), "<Primary>Q",
		sigc::mem_fun(*this, &Application::on_action_quit)
	);
	add_menu_item(menu, _("Open Playlist"), "<Primary>O",
		sigc::mem_fun(*this, &Application::on_action_openPlaylist)
	);
	add_menu_item(menu, _("Import Song"), "<Primary>I",
		sigc::mem_fun(*this, &Application::on_action_importSong)
	);
	
	return menu;
}

void Application::add_menu_item(
	Glib::RefPtr<Gio::Menu> menu,
	const Glib::ustring &label,
	const Glib::ustring &accel,
	const Gtk::Application::ActivateSlot &slot
) {
	const Glib::ustring actionName = Utils::app_menuLabel_to_actionName(label);
	
	this->add_action(actionName, accel, slot);
	Glib::RefPtr item = Gio::MenuItem::create(label, APP_ACTION_PREFIX + actionName);
	menu->append_item(item);
}

Glib::RefPtr<Gio::SimpleAction> Application::add_action(
	const Glib::ustring &name,
	const Glib::ustring &accel,
	const Gtk::Application::ActivateSlot &slot
) {
	Glib::RefPtr action = Gtk::Application::add_action(name, slot);
	this->set_accel_for_action(name, accel);
	return action;
}

void Application::reset_title(void)
{
	Glib::set_application_name(_title);
}

void Application::prefix_title(Glib::ustring prefix)
{
	Glib::set_application_name(prefix + " - " + _title);
}



// actions
// ==================================================

void Application::on_action_quit(void)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
	SPDLOG_INFO("Hiding all window forms.");
	for (Gtk::Window *window : this->get_windows()) {
		window->hide();
	}
	this->quit();
}

void Application::on_action_openPlaylist(void)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
}

void Application::on_action_closePlaylist(void)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
}

void Application::on_action_importSong(void)
{
	SPDLOG_TRACE("Triggered {:s}()", SPDLOG_FUNCTION);
}


// callbacks
// ==================================================

bool Application::cb__window_keypress(const GdkEventKey *const event)
{
	const Momuma::Bitset state(event->state);
	if (!state.contains(GDK_CONTROL_MASK)) { return false; }
	
	const bool keyShift = state.contains(GDK_SHIFT_MASK);
	const guint keyval = Gui::translate_keysm(event->keyval);
	
	SPDLOG_TRACE("Key pressed = 0x{:X} | 0x{:X}", keyval, event->keyval);
	
	Gui::PlaylistNotebook &notebook = m_window._notebook;
	if (keyval == GDK_KEY_Tab) {
		if (keyShift) {
			notebook.page_focus_left();
		}
		else {
			notebook.page_focus_right();
		}
		//return !sigc::PROPAGATE;
	}
	else if (!keyShift) {
		switch (keyval)
		{
		case GDK_KEY_n:
		case GDK_KEY_N:
		{
			Pango::AttrList attrList = Utils::create_attr_list({
				Pango::Attribute::create_attr_style(Pango::STYLE_ITALIC)
			});
			notebook.page_create(_("untitled"), attrList);
			break;
		}
		case GDK_KEY_p:
		case GDK_KEY_P:
		{
			auto &db = m_backend.get_database();
			std::optional<Glib::ustring> playlistName = get_playlist_choice_from_user(m_window, db);
			
			if (playlistName.has_value()) {
				this->add_playlist_to_view(playlistName.value());
			}
			break;
		}
		case GDK_KEY_w:
		case GDK_KEY_W:
		{
			if (notebook.size() == 0) { break; }
			
			const PageId current = notebook.current_page_id();
			if (current == m_pages.get_playing()) {
				m_backend.get_player().stop_playback();
			}
			notebook.page_remove(current);
			break;
		}
		case GDK_KEY_v:
		case GDK_KEY_V:
			m_window._controls._volume.get_widget_popup().popup();
			break;
		}
	}
	return sigc::PROPAGATE;
}

void Application::cb__row_activated(const PageId id, const int rowIndex, Gui::NotebookRowProxy row)
{
	SPDLOG_INFO("Clicked {:d}: '{:s}' [{}]", rowIndex, row.get_name(), row.get_duration());
	
	// unmark all the rows in the page containing the previously played song
	if (m_pages.has_playing_page()) {
		m_window._notebook
			.get_page(m_pages.get_playing())
			.mark_rows(Gui::NotebookColBit::None);
	}
	
	auto &player = m_backend.get_player();
	if (id != m_pages.get_playing() || player.playlist_empty()) {
		using IterFlag = Momuma::Database::IterFlag;
		spdlog::trace("1) Row activated");
		
		player.stop_playback();
		m_backend.get_database().get_media_paths(m_pages[id].name,
			[&player](fs::path p) -> IterFlag
			{
				mpv_error err = player.append_media(std::move(p));
				if (err == MPV_ERROR_SUCCESS) { return IterFlag::NEXT; }
				
				SPDLOG_ERROR("Failed to append mpv media: ({:d}) {:s}",
					err, mpv_error_string(err)
				);
				return IterFlag::STOP;
			}
		);
	}
	
	player.set_index(rowIndex);
	player.set_play(true);
	row.set_marked(Gui::NotebookColBit::NAME);
}

void Application::cb__slider_update(const Gui::Slider::DragPhase phase)
{
	auto &player = this->m_backend.get_player();
	const PlayerState state = player.get_state();
	if (state == PlayerState::STOP) {
		SPDLOG_WARN("The slider shouldn't be updated when the player is stopped");
		return;
	}
	
	static PlayerState oldState;
	
	if (phase == Gui::Slider::DragPhase::BEGIN) {
		(void)player.set_play(false);
	}
	else {
		auto &slider = m_window._controls._slider;
		(void)player.set_position(slider.get_time());
		
		if (oldState == PlayerState::PLAY) {
			(void)player.set_play(true);
		}
	}
	
	oldState = state;
}

void Application::cb__audioStreamStarted(Momuma::MpvPlayer &src)
{
	mpv_error e;
	m_window._controls._slider.set_time_limit(
		chrono::duration_cast<chrono::milliseconds>(src.get_duration(e))
	);
	SPDLOG_TRACE("{:s}: ({:d}) {:s}", SPDLOG_FUNCTION, e, mpv_error_string(e));
}

void Application::cb__audioStreamEnded(Momuma::MpvPlayer &src)
{
	SPDLOG_TRACE(SPDLOG_FUNCTION);
	if (m_pages.get_playing() == PageId::Null) { return; }
	
	std::vector proxies = m_window._notebook.get_page(m_pages.get_playing()).get_rows();
	if (proxies.size() == 0) { return; }
	
	const int64_t index = src.get_index();
	const auto songs = static_cast<size_t>(src.playlist_size());
	if (index < 0) {
		proxies.back().set_marked(Gui::NotebookColBit::None);
		return;
	}
	
	const auto uIndex = static_cast<size_t>(index);
	proxies.at(uIndex).set_marked(Gui::NotebookColBit::NAME);
	if (proxies.size() > 1) {
		proxies.at((uIndex - 1) % songs).set_marked(Gui::NotebookColBit::None);
	}
}

void Application::cb__audioStateChanged(Momuma::MpvPlayer&,
	[[maybe_unused]] const PlayerState prevState,
	const PlayerState newState
) {
	SPDLOG_TRACE("{:s}: {:d} => {:d}", SPDLOG_FUNCTION,
		static_cast<int>(prevState), static_cast<int>(newState)
	);
	update_controls_state(m_window._controls, newState);
	m_letSliderUpdate = (newState == PlayerState::PLAY);
}
