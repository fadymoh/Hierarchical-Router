#include "Parser.h"
#include <vector>
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
/*mypq_type Parser::order_the_nets()
{
auto it = nets.begin();
mypq_type ordered_nets;
while (it != nets.end()) {
float sum = 0;
std::pair <int, int> current, prev; //first is x-axis and second is y-axis
for (int x = 0; x < it->second.size(); ++x) {
int i;
str temp = it->second[x].first;
for (i = temp.length() - 1; i >= 0; --i)
if (temp[i] == '_') break;
str gate_name = temp.substr(0, i);

for (i = 0; i < components[gate_name].connected_gates[temp].pins_connections.size(); ++i)
if (components[gate_name].connected_gates[temp].pins_connections[i].first == it->second[x].second) break;
if (x == 0) {
current.first = prev.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
current.second = prev.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
}
else {
current.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
current.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
sum += sqrt(pow((current.first - prev.first), 2) + pow((current.second - prev.second), 2));
}
}
std::pair<str, float> temp;
temp = std::make_pair(it->first, sum);
ordered_nets.push(temp);
it++;
}
return ordered_nets;
}*/
mypq_type Parser::order_the_nets()
{
	auto it = nets.begin();
	int metal_1, metal_2;
	metal_1 = metal_2 = 1;
	mypq_type return_PQ;
	while (it != nets.end())
	{

		triplet current, prev; //first is x-axis and second is y-axis
		for (int x = 0; x < it->second.size(); ++x)
		{
			int i;
			str temp = it->second[x].first;
			for (i = temp.length() - 1; i >= 0; --i)
				if (temp[i] == '_') break;
			str gate_name = temp.substr(0, i);

			for (i = 0; i < components[gate_name].connected_gates[temp].pins_connections.size(); ++i)
				if (components[gate_name].connected_gates[temp].pins_connections[i].first == it->second[x].second) break;
			if (x == 0) {
				current.first = prev.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
				current.second = prev.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
				if (IsPrimary(it->first)) {
					pair<int, int> temp_pair = getPrimaryPinCoordinates(it->first, metal_1);
					triplet temp_trip;
					temp_trip.first = temp_pair.first;
					temp_trip.second = temp_pair.second;
					temp_trip.third = metal_1;
					current.third = metal_2;
					pair<triplet, triplet> mytriplet = make_pair(temp_trip, current);
					float temp = sqrt(pow((current.first - temp_trip.first), 2) + pow((current.second - temp_trip.second), 2));
					pair <float, string> second_pair = make_pair(temp, it->first);
					return_PQ.push(make_pair(mytriplet, second_pair));
					metal_1 = 1;
				}
			}
			else {

				current.first = components[gate_name].connected_gates[temp].pins_connections[i].second.x;
				current.second = components[gate_name].connected_gates[temp].pins_connections[i].second.y;
				current.third = metal_1;
				prev.third = metal_2;
				float temp = sqrt(pow((current.first - prev.first), 2) + pow((current.second - prev.second), 2));

				pair <triplet, triplet> mytriplet = make_pair(current, prev);
				pair <float, string> second_pair = make_pair(temp, it->first);
				return_PQ.push(make_pair(mytriplet, second_pair));
				prev = current;

			}
		}
		it++;
	}

	return return_PQ;
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
				int x = itz->second.x / track[layer_string].third;
				int y = itz->second.y / track[layer_string].third;
				for (int i = 0; i < it->second.pins_sizes.size(); ++i) {
					int x_axis = x + int(it->second.pins_sizes[i].second.x1) / track[layer_string].third;
					int y_axis = y + int(it->second.pins_sizes[i].second.y1) / track[layer_string].third;
					if (it->second.pins_sizes[i].first != "gnd" && it->second.pins_sizes[i].first != "vdd") {
						//if (grid[x_axis][y_axis] == -1)  std::cout << "overwriting\n";
						//else {
						grid[x_axis][y_axis] = -1;
						int j;
						for (j = 0; j < itz->second.pins_connections.size(); ++j)
							if (itz->second.pins_connections[j].first == it->second.pins_sizes[i].first) break;

						itz->second.pins_connections[j].second.x = x_axis;
						itz->second.pins_connections[j].second.y = y_axis;
						itz->second.pins_connections[j].second.metal_layer = 1;
						//}
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
				int x = it->second.x / track[layer_string].third;
				int y = it->second.y / track[layer_string].third;
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
						int x = stoi(myvector[6]) * 1;
						int y = stoi(myvector[7]) * 1;
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
							pins[primary_pin].x = stoi(myvector[3]) * 1 - xinit;
							pins[primary_pin].y = stoi(myvector[4]) * 1 - yinit;
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
								components[gate_name].size_x = stof(myvector[1]) * 100;
								components[gate_name].size_y = stof(myvector[3]) * 100;
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
	for (int i = 1; i < 5; ++i) {
		str layer_string = "metal" + std::to_string(i);
		y_dimension = yfinal / track[layer_string].third + 110;
		x_dimension = xfinal / track[layer_string].third + 110;
		x[i].resize(x_dimension);
		for (int j = 0; j < x_dimension; ++j)
			x[i][j].resize(y_dimension);
		x[i] = makegridlayer(i);
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


vector<pair<str, str>> Parser::getNetPairs(str net_name)
{
	return nets[net_name];
}

pair<int, int> Parser::getConnectedPinCoordinates(str gate_name, str pin_name, int & metal)
{
	pair<int, int> xy;
	unordered_map<std::string, gate_info> connected_gates_temp;
	gate_info gate_info_temp;
	vector<std::pair<std::string, pin_info>> pins_connections_temp;
	pin_info pin_info_temp;
	str temp;

	mystruct tempstruct;

	temp = gate_name;
	int found = temp.find("_");
	string gate_name_new = temp.erase(found, string::npos);

	tempstruct = components[gate_name_new];
	connected_gates_temp = tempstruct.connected_gates;
	gate_info_temp = connected_gates_temp[gate_name];
	pins_connections_temp = gate_info_temp.pins_connections;
	for (int i = 0; i < pins_connections_temp.size(); i++)
	{
		if (pins_connections_temp[i].first == pin_name)
		{
			pin_info_temp = pins_connections_temp[i].second;
			//connected_pin = pin_info_temp.connected_pin;
			xy.first = pin_info_temp.x;
			xy.second = pin_info_temp.y;
			//metal = pin_info_temp.metal_layer;
			metal = 1;
		}
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

pair<int, int> Parser::getPrimaryPinCoordinates(str pin_name, int& metal)
{
	pair<int, int> xy;
	xy.first = pins[pin_name].x / (get_track_step(pins[pin_name].metal_layer));
	xy.second = pins[pin_name].y / (get_track_step(pins[pin_name].metal_layer));
	metal = pins[pin_name].metal_layer;
	return xy;
}
int Parser::get_track_step(int metal_layer)
{
	string metal = "metal" + to_string(metal_layer);
	return track[metal].third;
}