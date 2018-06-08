#include "Parser.h"
#include <vector>
unsigned int gcd(unsigned int n1, unsigned int n2) {
	return (n2 == 0) ? n1 : gcd(n2, n1 % n2);
}
std::vector<std::string> Parser::split(const char *str, char c = ' ')
{
	std::vector<std::string> result;
	do {
		const char *begin = str;
		while (*str != c && *str && *str != '\t') str++;
		result.push_back(std::string(begin, str));
	} while (0 != *str++);
	return result;
}

std::vector <std::string> Parser::end_vector(std::ifstream &x)
{
	x.getline(arr, 400);
	std::vector <std::string> myvector = split(arr, ' ');
	myvector.erase(remove_if(myvector.begin(), myvector.end(), EmptyStr()), myvector.end());
	return myvector;
}
Parser::Parser()
{

}
void Parser::set_def(str input)
{
	input_file = input;
}
void Parser::set_lef(str input)
{
	lef_file = input;
}
void Parser::print_output()
{
	auto it = components.begin();
	while (it != components.end()) {
		std::cout << it->first << ' ' << it->second.size_x / site_width << " sites\n\n";
		auto itz = it->second.connected_gates.begin();
		while (itz != it->second.connected_gates.end()) {
			std::cout << itz->first << ' ' << itz->second.x << ' ' << itz->second.y << ' ' << itz->second.orientation << '\t';
			for (size_t i = 0; i < itz->second.pins_connections.size(); ++i)
				std::cout << itz->second.pins_connections[i].first << " = " << itz->second.pins_connections[i].second.connected_pin << '\t';
			std::cout << '\n';
			itz++;
		}
		it++;
		std::cout << '\n';
	}
}
void Parser::order_the_nets()
{
	auto it = nets.begin();
	triplet prev, current;
	int metal_1, metal_2;
	metal_1 = metal_2 = 1;

	int d = 0;
	Tree flutetree;
	int flutewl;
	readLUT();

	while (it != nets.end())
	{
		int *xx, *yy;
		 xx = new int[it->second.size()];
		 yy = new int[it->second.size()];
		d = 0;

		bool flag = true;
		if (pins.find(it->first) != pins.end()) // it is a primary pin and has coordinates
		{
			prev.first = pins[it->first].x;
			prev.second = pins[it->first].y;
			prev.third = pins[it->first].metal_layer;
			xx[d] = prev.first;
			yy[d++] = prev.second;
			flag = false;
		}
		
		for (int x = 0; x < it->second.size(); ++x)
		{
			int i;
			str temp = it->second[x].first;
			for (i = temp.length() - 1; i >= 0; --i)
				if (temp[i] == '_') break;
			str gate_name = temp.substr(0, i);

			for (i = 0; i < components[gate_name].connected_gates[temp].pins_connections.size(); ++i)
				if (components[gate_name].connected_gates[temp].pins_connections[i].first == it->second[x].second) break;
			if (flag == true) //means it is not a primary pin
			{
				if (x == 0)
				{
					current.first = prev.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
					current.second = prev.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
					current.third = prev.third = 1;
					xx[d] = prev.first;
					yy[d++] = prev.second;
				}
				else
				{
					current.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
					current.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
					current.third = 1;
					xx[d] = current.first;
					yy[d++] = current.second;
					my_ordered_nets[it->first].push_back(std::make_pair(prev, current));
				}
			}
			else // it is a primary pin
			{
				current.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
				current.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
				current.third = 1;
				xx[d] = current.first;
				yy[d++] = current.second;
				my_ordered_nets[it->first].push_back(std::make_pair(prev, current));
			}
			prev = current;
		}
		flutetree = flute(d, xx, yy, ACCURACY);
		return_PQ.push(std::make_pair(it->first, flutetree.length));
		printf("FLUTE wirelength = %d\n", flutetree.length);
		it++;
	}

	auto my_iterator = my_ordered_nets.begin();
	int size = 0;
	while (my_iterator != my_ordered_nets.end())
	{
		size += my_iterator->second.size();
		my_iterator++;
	}
	std::cout << size;
}
std::vector<std::pair<triplet, triplet>> Parser::get_net_pairs(str My_net_name)
{
	return my_ordered_nets[My_net_name];
}
std::vector<str> Parser::get_net_names()
{
	std::vector<str> temp;
	while (!return_PQ.empty())
	{
		std::pair<str, int> temp_pair = return_PQ.top();
		return_PQ.pop();
		temp.push_back(temp_pair.first);
	}
	return temp;
}
TwoDimensions Parser::makegridlayer(int layer)
{
	str layer_string = "metal" + std::to_string(layer);
	TwoDimensions grid;
	grid.resize(x_dimension);
	for (int i = 0; i < x_dimension; ++i)
		grid[i].resize(y_dimension);
	auto it = components.begin();
	for (int i = 0; i < x_dimension; ++i)
		for (int j = 0; j < y_dimension; ++j)
			grid[i][j] = 0;
	if (layer == 1)
		while (it != components.end()) {
			auto itz = it->second.connected_gates.begin();
			while (itz != it->second.connected_gates.end()) {
				int x = itz->second.x / final_step_x;
				int y = itz->second.y / final_step_y;
				for (int i = 0; i < it->second.pins_sizes.size(); ++i) {
					int x_axis = x + int(it->second.pins_sizes[i].second.x1) / final_step_x;
					int y_axis = y + int(it->second.pins_sizes[i].second.y1) / final_step_y;
					if (it->second.pins_sizes[i].first != "gnd" && it->second.pins_sizes[i].first != "vdd") {

						grid[x_axis][y_axis] = -1;
						int j;
						for (j = 0; j < itz->second.pins_connections.size(); ++j)
							if (itz->second.pins_connections[j].first == it->second.pins_sizes[i].first) break;

						itz->second.pins_connections[j].second.x = x_axis;
						itz->second.pins_connections[j].second.y = y_axis;
						itz->second.pins_connections[j].second.metal_layer = 1;
					}
				}
				itz++;
			}
			it++;
		}
	else {
		auto it = pins.begin();
		while (it != pins.end()) {
			if (it->second.metal_layer == layer) {
				int x = it->second.x / final_step_x;
				int y = it->second.y / final_step_y;
				if (grid[x][y] == -1) std::cout << "overwriting\n";
				else grid[x][y] = -1;
			}
			it++;
		}
	}
	return grid;
}
void Parser::Parse_DEF()
{
	file.open(input_file.c_str());
	if (file.fail())	std::cout << " couldnt open the file \n";
	else //parsing the def
		while (!file.eof()) {
			std::vector <str> myvector = end_vector(file);
			if (myvector.size() >= 1)
				if (myvector[0] == "UNITS")
					Units_distance = stoi(myvector[3]);
				else if (myvector[0] == "DIEAREA") {
					xinit = stoi(myvector[2]);
					yinit = stoi(myvector[3]);
					xfinal = (stoi(myvector[6]) - xinit);
					yfinal = (stoi(myvector[7]) - xinit);
				}
				else if (myvector[0] == "TRACKS") {
					tracks temp;
					temp.orientation = myvector[1][0];
					temp.first = stoi(myvector[2]);
					temp.second = stoi(myvector[4]);
					temp.third = stoi(myvector[6]);
					track[myvector[8]] = temp;
				}
				else if (myvector[0] == "COMPONENTS") {
					while (myvector[0] != "END") {
						myvector = end_vector(file);
						if (myvector[0] == "END") break;
						int x = stoi(myvector[6]);
						int y = stoi(myvector[7]);
						gate_info temp;
						temp.x = x;
						temp.y = y;
						temp.orientation = myvector[9];
						components[myvector[2]].connected_gates[myvector[1]] = temp;
					}
				}
				else if (myvector[0] == "NETS") {
					while (!(myvector[0] == "END")) {
						myvector = end_vector(file);
						if (myvector[0] == "END") break;
						if (myvector[0] == "-") pin_name = myvector[1];
						else if (myvector[0] != "+" && myvector[0] != "NEW") {
							if (myvector[1] != "PIN") {
								int i;
								for (i = myvector[1].length() - 1; i >= 0; --i)
									if (myvector[1][i] == '_') break;
								str gate_name = myvector[1].substr(0, i);
								std::pair <str, pin_info> temp;
								pin_info temp_pin;
								temp_pin.connected_pin = pin_name;
								temp = make_pair(myvector[2], temp_pin);
								components[gate_name].connected_gates[myvector[1]].pins_connections.push_back(temp);
								std::pair <str, str> temp2;
								temp2 = make_pair(myvector[1], myvector[2]); //gate name is first and pin name is second
								nets[pin_name].push_back(temp2);
							}
						}
					}
				}
				else if (myvector[0] == "PINS") {
					bool first = false;
					str primary_pin;
					while (!(myvector[0] == "END") || !first) {
						first = true;
						myvector = end_vector(file);
						if (myvector[0] == "-")  primary_pin = myvector[1];
						else if (myvector[1] == "LAYER")
							pins[primary_pin].metal_layer = stoi(myvector[2].substr(5));
						else if (myvector[1] == "PLACED") {
							pins[primary_pin].x = stoi(myvector[3]) - xinit;
							pins[primary_pin].y = stoi(myvector[4]) - yinit;
						}

					}
				}
		}
	file.close();
}
void Parser::Parse_LEF()
{
	bool first = true;
	lef.open(lef_file.c_str());
	if (lef.fail())		std::cout << " couldnt open the file \n";
	else
		while (!lef.eof()) {
			std::vector <str> myvector = end_vector(lef);
			if (myvector.size() >= 1) {
				if (myvector[0] == "SITE" && myvector[1] == "core" && first) {
					while (myvector[0] != "SIZE")	myvector = end_vector(lef);
					site_width = stof(myvector[1]);
					row_height = stof(myvector[3]);
					first = false;
				}
				else if (myvector[0] == "MACRO") {
					str gate_name = myvector[1], pin;
					bool flag = false, start = true;
					auto it = components.find(myvector[1]);
					if (it != components.end())
						while (myvector.size() == 1 || !(myvector[0] == "END" && myvector[1] == gate_name) || start) {
							start = false;
							myvector = end_vector(lef);
							if (myvector[0] == "SIZE") {
								components[gate_name].size_x = stof(myvector[1]);
								components[gate_name].size_y = stof(myvector[3]);
							}
							else if (myvector[0] == "PIN") {
								pin = myvector[1];
								flag = false;
							}
							else if (myvector[0] == "RECT" && !flag) {
								std::pair<str, rect> temp;
								rect temp_rect;
								temp_rect.x1 = stof(myvector[1]) * 100;
								temp_rect.x2 = stof(myvector[3]) * 100;
								temp_rect.y1 = stof(myvector[2]) * 100;
								temp_rect.y2 = stof(myvector[4]) * 100;
								temp = make_pair(pin, temp_rect);
								components[gate_name].pins_sizes.push_back(temp);
								flag = true;
							}
						}
				}
				else if (myvector[0] == "LAYER") {
					str layer_name = myvector[1];
					bool start = true;
					auto it = track.find(layer_name);
					if (it != track.end() && !it->second.found_before)
						while (myvector.size() == 1 || !(myvector[0] == "END" && myvector[1] == layer_name) || start) {
							start = false;
							myvector = end_vector(lef);
							if (myvector[0] == "PITCH")
								track[layer_name].pitch = stof(myvector[1]);
							else if (myvector[0] == "OFFSET")
								track[layer_name].offset = stof(myvector[1]);
							else if (myvector[0] == "WIDTH")
								track[layer_name].width = stof(myvector[1]);
							else if (myvector[0] == "SPACING") {
								track[layer_name].spacing = stof(myvector[1]);
								track[layer_name].found_before = true;
								break;
							}
						}
					else {
						str via_name = myvector[1];
						int pos = via_name.find("via");
						if (pos != str::npos) {
							auto itz = track.find("metal" + via_name.substr(3));
							if (itz != track.end() && !itz->second.found_via)
								while (!(myvector[0] == "END" && myvector[1] == via_name)) {
									myvector = end_vector(lef);
									if (myvector[0] == "SPACING") {
										track[itz->first].via_spacing = stof(myvector[1]);
										track[itz->first].found_via = true;
									}
								}
						}
					}
				}
			}
		}
	lef.close();
}
void Parser::create_grid(ThreeDimensions &x)
{
	x.resize(5);
	int current_x = -1 , current_y = -1;
	for (int i = 1; i < track.size(); ++i)
	{
		str layer_string = "metal" + std::to_string(i);
		if (track[layer_string].orientation == 'X')
		{
			if (current_x == -1)
				current_x = track[layer_string].third;
			else
				current_x = gcd(current_x, track[layer_string].third);
		}
		else if (track[layer_string].orientation == 'Y')
		{
			if (current_y == -1)
				current_y = track[layer_string].third;
			else
				current_y = gcd(current_y, track[layer_string].third);
		}
	}

	final_step_x = current_x;
	final_step_y = current_y;
	for (int i = 1; i < track.size() + 1; ++i) {
		str layer_string = "metal" + std::to_string(i);
		y_dimension = yfinal / current_y +1;
		x_dimension = xfinal / current_x +1;
		x[i].resize(x_dimension);
		for (int j = 0; j < x_dimension; ++j)
			x[i][j].resize(y_dimension);
		x[i] = makegridlayer(i);
	}
	for (int i = 1; i < track.size() + 1; ++i)
	{
		str layer_string = "metal" + std::to_string(i);
		if (track[layer_string].orientation == 'X')
		{
			if (track[layer_string].third != final_step_x)
			{
				int skip = track[layer_string].third / final_step_x;
				for (int x_axis = 0; x_axis < x_dimension; x_axis += skip)
				{
					for (int y_axis = 0; y_axis < x[i][x_axis].size(); ++y_axis)
						x[i][x_axis][y_axis] = -1;
				}
			}
		}
		else if (track[layer_string].orientation == 'Y')
		{
			if (track[layer_string].third != final_step_y)
			{
				int skip = track[layer_string].third / final_step_y;
				for (int y_axis = 0; y_axis < y_dimension; y_axis += skip)
				{
					for (int x_axis = 0; x_axis < x[i].size(); ++x_axis)
						x[i][x_axis][y_axis] = -1;
				}
			}
		}
	}

}
void Parser::getGridDimensions(int& x, int &y) {
	x = x_dimension;
	y = y_dimension;
}
Parser::~Parser()
{

}

std::unordered_map<str, mystruct> Parser::getComponents()
{
	return components;
}
std::unordered_map<str, pin_info> Parser::getPins()
{
	return pins;
}
std::unordered_map<str, std::vector<std::pair<str, str>>> Parser::getNets()
{
	return nets;
}


std::vector<std::pair<str, str>> Parser::getNetPairs(str net_name)
{
	return nets[net_name];
}

std::pair<int, int> Parser::getConnectedPinCoordinates(str gate_name, str pin_name, int & metal)
{
	std::pair<int, int> xy;
	str temp = gate_name;
	int i;
	for (i = temp.length() - 1; i >= 0; --i)
		if (temp[i] == '_') break;
	temp = temp.substr(0, i);
	auto it = components[temp].connected_gates[gate_name].pins_connections.begin();
	while (it != components[temp].connected_gates[gate_name].pins_connections.end()) {
		if (it->first == pin_name)
		{
			xy.first = it->second.x;
			xy.second = it->second.y;
			metal = 1;
		}
		it++;
	}
	return xy;
}

bool Parser::IsPrimary(str pin_name)
{

	auto found = pins.find(pin_name);

	if (found == pins.end())
		return false;
	else
		return true;

}

std::pair<int, int> Parser::getPrimaryPinCoordinates(str pin_name, int& metal)
{
	std::pair<int, int> xy;
	xy.first = pins[pin_name].x / (final_step_x);
	xy.second = pins[pin_name].y / (final_step_y);
	metal = pins[pin_name].metal_layer;
	return xy;
}
int Parser::get_track_step_x()
{
	return final_step_x;
}
int Parser::get_track_step_y()
{
	return final_step_y;
}