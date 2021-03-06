#ifndef PARSER_H
#define PARSER_H
#include <fstream>
#include <algorithm>
#include <queue>
#include <utility>
#include <functional>
#include <iostream>
#include "flute.h"
#include "MyHeaders.h"
class Parser {

private:
	std::unordered_map <string, std::vector<std::pair<triplet, triplet>>> my_ordered_nets;
	std::unordered_map<string, mystruct> components; //Contains all the components and their sizes locations and their pins sizes and location
	std::unordered_map <string, tracks> track; //contains the information about the tracks
	std::unordered_map<string, pin_info> pins; //contains the primary pins locations
	std::unordered_map<string, std::vector<std::pair<string, string>>> nets; //contains the the net and all of its connected components in a vector of pairs where the
																	//first element is the gate name and the second is the connected pin
	float site_width, row_height;
	string pin_name, input_file, output_file, lef_file;
	int x_dimension, y_dimension, Units_distance, xinit, xfinal, yinit, yfinal, final_step_x, final_step_y;
	std::ifstream file, lef;
	char arr[400];
	std::vector<std::string> split(const char *, char);
	std::vector <std::string> end_vector(std::ifstream &x);
	mypq_type return_PQ;
public:
	Parser();
	TwoDimensions makegridlayer(int);
	void set_lef(string);
	void set_def(string);
	void print_output();
	void getGridDimensions(int&, int&);
	std::unordered_map<string, mystruct> getComponents();
	std::unordered_map<string, pin_info> getPins();
	std::unordered_map<string, std::vector<std::pair<string, string>>> getNets();
	
	std::vector<string> get_net_names();
	//mypq_type order_the_nets();
	void Parse_DEF();
	void Parse_LEF();
	void create_grid(ThreeDimensions &);
	std::vector<std::pair<string, string>> getNetPairs(string);
	std::pair<int, int> getConnectedPinCoordinates(string, string, int&);
	std::pair<int, int> getPrimaryPinCoordinates(string, int&);
	bool IsPrimary(string);
	int get_track_step_x();
	int get_track_step_y();
	std::vector<std::pair<triplet, triplet>> get_net_pairs(string);
	void order_the_nets();

	~Parser();
};

#endif