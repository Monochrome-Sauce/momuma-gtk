#include <momuma/spdlog.h>

#include "Application.h"


bool Application::add_playlist_to_view(const Glib::ustring &playlistName)
{
	Gui::PlaylistNotebook &notebook = m_window._notebook;
	const PageId id = notebook.page_create(playlistName);
	m_pages[id] = PageData { playlistName, false };
	
	const int items = m_backend.get_database().get_media_paths(playlistName,
		[&notebook, id](fs::path p) -> Momuma::Database::IterFlag
		{
			auto duration = chrono::duration_cast<chrono::seconds>(
				Momuma::MpvPlayer::query_duration(p)
			);
			notebook.get_page(id).append_row({ p.filename().string(), duration });
			return Momuma::Database::IterFlag::NEXT;
		}
	);
	
	if (items < 0) {
		SPDLOG_ERROR("Failed to get all media paths");
	}
	return !(items < 0);
}

Momuma::Momuma &Application::get_backend(void)
{
	return m_backend;
}
