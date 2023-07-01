momuma_sources = files(
	'Application-public-API.cpp',
	'Application.cpp',
	'Gui/AudioPlayerControls.cpp',
	'Gui/ListChooserDialog.cpp',
	'Gui/MasterWindow.cpp',
	'Gui/PlaylistNotebook.cpp',
	'Gui/Slider.cpp',
	'Gui/VolumeButton.cpp',
	'Gui/functions.cpp',
	'Pages.cpp',
	'main.cpp',
	'misc.cpp',
)

if not is_debug_build
	extra_flags += [
		'-DG_DISABLE_ASSERT=1',
		'-DG_DISABLE_CHECKS=1',
		'-DNDEBUG',
	]
endif
