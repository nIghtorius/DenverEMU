/*

	Configuration handler.

*/

#pragma once

#include <string>
#include <vector>

enum ini_datatypes {
	dt_int, dt_bool, dt_string
};

struct configitem {
	std::string section;
	std::string property;
	ini_datatypes datatype;
	std::string str_value;
	int			boolint_value;
};

class inifile {
private:
	std::vector<configitem> items;
	int		find_index_of_item(std::string section, std::string item);
public:
	inifile();
	~inifile();
	
	void	setvalue(std::string section, std::string item, char * value);
	void	setvalue(std::string section, std::string item, int value);
	void	setvalue(std::string section, std::string item, bool value);

	void	save(std::string filename);
};