#include <momuma/spdlog.h>

#include "Pages.h"


PageMap::PageMap(void) :
	m_playing { PageId::Null }
{
}

PageId PageMap::get_playing(void)
{
	return m_playing;
}

bool PageMap::has_playing_page(void)
{
	return m_playing != PageId::Null && this->contains(m_playing);
}

void PageMap::connect_page_destroyed(Gui::PlaylistNotebook &src)
{
	src.signal_page_destroyed().connect(
		[this, &src](PageId) -> void
		{
			m_playing = src.current_page_id();
		}
	);
}

void PageMap::connect_row_activated(Gui::PlaylistNotebook &src)
{
	src.signal_row_activated().connect(
		[this](const PageId newId, int, Gui::NotebookRowProxy)
		{
			spdlog::trace("2) Row activated");
			m_playing = newId;
		}
	);
}
