#ifndef APPLICATION_INTERFACE_H
#define APPLICATION_INTERFACE_H

#include <momuma/momuma.h>

#include "glibmm/ustring.h"


/* #Used as a global API for the application itself.
*/
class Application
{
public:
	bool add_playlist_to_view(const Glib::ustring &playlistName);
	
	Momuma::Momuma &get_backend(void);
	
private:
	Application(void) = delete;
};

namespace MomumaGtk
{
	// only way the API should be accessed
	extern Application *const app;
}

#endif /* APPLICATION_INTERFACE_H */
