#include "denver_config.h"
#include <iostream>
#include <fstream>

inifile::inifile() {
	items.clear();
}

inifile::~inifile() {
	items.clear();
}


int		inifile::find_index_of_item(std::string section, std::string item) {
	for (int i = 0; i < items.size(); i++) {
		if ((items[i].property == item) &&
			(items[i].section == section)) return i;
	}
	return -1;
}

void	inifile::setvalue(std::string section, std::string item, char* value) {
	int q = find_index_of_item(section, item);
	if (q != -1) {
		items[q].str_value = value;
		items[q].boolint_value = 0;
		items[q].datatype = dt_string;
	}
	else {
		configitem _item;
		_item.boolint_value = 0;
		_item.str_value = value;
		_item.property = item;
		_item.datatype = dt_string;
		_item.section = section;
		items.push_back(_item);
	}
}

void	inifile::setvalue(std::string section, std::string item, int  value) {
	int q = find_index_of_item(section, item);
	if (q != -1) {
		items[q].str_value = "";
		items[q].boolint_value = value;
		items[q].datatype = dt_int;
	}
	else {
		configitem _item;
		_item.section = section;
		_item.boolint_value = value;
		_item.str_value = "";
		_item.property = item;
		_item.datatype = dt_int;
		items.push_back(_item);
	}
}

void	inifile::setvalue(std::string section, std::string item, bool value) {
	setvalue(section, item, (int)value);		// just use the int variant.
	items[find_index_of_item(section, item)].datatype = dt_bool;
}


void	inifile::save(std::string filename) {
	// first get all the sections.
	std::vector<std::string> sections;
	for (int i = 0; i < items.size(); i++) {
		std::string sectionName = items[i].section;
		bool	known = false;
		for (std::string section : sections) {
			if (sectionName == section) known = true;
		}
		std::cout << sectionName << " " << (int)known << "\n";
		if (!known) sections.push_back(sectionName);	// not known, add the sections.
	}

	// setup the file.
	std::ofstream file(filename);

	// enumerate the sections and get write their values(properties)		
	for (std::string section : sections) {
		// write section name.
		file << "[" << section << "]" << std::endl;
		// collect members of that section.
		for (configitem item : items) {
			if (item.section == section) {
				// value is in section.. write it.
				file << item.property << "=";
				std::cout << item.datatype << "\n";
				switch (item.datatype) {
				case dt_int:
					file << std::dec << (int)item.boolint_value << std::endl;
					break;
				case dt_bool:
					if (item.boolint_value) {
						file << "true" << std::endl;
					}
					else {
						file << "false" << std::endl;
					}
					break;
				case dt_string:
					file << item.str_value << std::endl;
					break;
				}
			}
		}
		// extra empty line between sections.
		file << std::endl;
	}

	file.close(); // we are done.
}
