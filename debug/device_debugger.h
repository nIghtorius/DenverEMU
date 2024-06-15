/*

	Denver Device Debugger Class.
	Debug Abstraction Class debugging devices in the global device debugger GUI interface.

*/

#pragma once

#include <string>
#include <vector>

#define SHOW_ONLY_VALUE_AS_TEXT -1

enum dbg_datatypes {
	t_int, t_byte, t_shortintarray, t_cstr, t_longintarray, t_beginblock, t_endblock, t_bool, t_addr, t_word, t_bytearray,
	t_byte_lonibble
};

struct dbg_dt {
	dbg_datatypes datatype;
	int	max = SHOW_ONLY_VALUE_AS_TEXT;		// -1 means no max (show as value), max set means bar.
	const void* data;
	std::string description;
};

class device_debugger {
public:
	bool		show_debugger = false;			// gets called by the GUI.
	std::vector<dbg_dt*> debuglines;
	device_debugger();
	void	add_debug_var(const std::string description, const int max_value, const void* data, const dbg_datatypes datatype);
};
