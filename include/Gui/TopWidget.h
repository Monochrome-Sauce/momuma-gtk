#ifndef GUI__TOP_WIDGET_H
#define GUI__TOP_WIDGET_H

#include <gtkmm/enums.h>

namespace Gui
{

template<typename Widget>
struct TopWidget
{
public:
	[[nodiscard]] constexpr inline operator Widget&(void) { return w_; }
	[[nodiscard]] constexpr inline operator const Widget&(void) const { return w_; }
	
	[[nodiscard]] constexpr inline Widget& get_top_widget(void) { return w_; }
	[[nodiscard]] constexpr inline const Widget& get_top_widget(void) const { return w_; }
	
	template<typename ...Args>
	constexpr inline
	TopWidget(Args &&...args) : w_ { args... }
	{}
	
protected:
	Widget w_;
};

}

#endif /* GUI__TOP_WIDGET_H */
