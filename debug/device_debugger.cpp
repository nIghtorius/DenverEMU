/*
	
	Device Debugger Class.

*/

#include "device_debugger.h"

device_debugger::device_debugger() {
	debuglines.clear();
}

void device_debugger::add_debug_var(const std::string description, const int max_value, const void* data, const dbg_datatypes datatypes) {
	debuglines.push_back(
		new dbg_dt{ datatypes, max_value, data, description }
	);
}
