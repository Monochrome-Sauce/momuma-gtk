#ifndef PAGES_H
#define PAGES_H

#include "Gui/PlaylistNotebook.h"


using PageId = Gui::PlaylistNotebook::PageId;

struct PageData
{
	Glib::ustring name;
	bool unsaved;
};

class PageMap final : public std::unordered_map<PageId, PageData>
{
public:
	PageMap(void);
	
	[[nodiscard]] PageId get_playing(void);
	
	[[nodiscard]] bool has_playing_page(void);
	
	void connect_page_destroyed(Gui::PlaylistNotebook &src);
	void connect_row_activated(Gui::PlaylistNotebook &src);
	
private:
	PageId m_playing;
};

#endif /* PAGES_H */
