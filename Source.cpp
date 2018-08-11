#define _CRT_SECURE_NO_WARNINGS

#include "Parser.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <queue>
#include <stack>
#include <limits>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <functional>
#include <cfloat>
#include <cmath>
#include <chrono>

using namespace std::chrono;
using namespace std;
#define GLOBAL 1
#define DETAILED 0

const int UNBLOCKED = 0;
const int BLOCKED = -1;
const int SHARED = -2;
int GBOXsize = 10;
int congestion = 40;
int enable_output = 0;
int time_out_counter;
int save_time_min, save_time_hour, save_time_seconds;
int global_fail, detailed_fail;
int metal_temp;

bool global_detailed;
stack<string> actual_path;
stack<triplet>actual_coordinates;
vector<string> directions;

Parser myparser;
stack<triplet> Path;
unordered_map<int, pair<int, int>> pathcoordinates;
vector <pair<triplet, triplet>> failed_routing;
vector <string> failed_routing_name;
vector<triplet> checkVector;

pair <int, int> prev_pins_coordinates_temp;

string net_name;
unordered_map<string, vector<string>> DEFRoute;
void save_time()
{
	time_t now = time(0);
	tm* gmtm = gmtime(&now);
	save_time_min = gmtm->tm_min;
	save_time_hour = gmtm->tm_hour;
	save_time_seconds = gmtm->tm_sec;
}
void block_grid(ThreeDimensions &grid, vector <string> &directions, int &coordinatesi, int &coordinatesj, int &coordinatesk, bool block)
{
	for (int i = 0; i < directions.size(); i++)
	{
		if (directions[i] == "SOUTH")
		{
			//	if (coordinatesj + 1 != grid[coordinatesi].size())
			coordinatesj++;
			if (block)
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
			actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
		}
		if (directions[i] == "NORTH")
		{
			coordinatesj--;
			if (block)
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
			actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
		}
		if (directions[i] == "EAST")
		{
			//	if (coordinatesk + 1 != grid[coordinatesi][coordinatesj].size())
			coordinatesk++;
			if (block)
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
			actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
		}
		if (directions[i] == "WEST")
		{
			coordinatesk--;
			if (block)
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
			actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
		}
		if (directions[i] == "UP")
		{

			coordinatesi++;
			if (block)
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
			actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
		}
		if (directions[i] == "DOWN")
		{
			coordinatesi--;
			if (block)
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
			actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
		}
	}
	directions.clear();
}
void create_output(triplet &p, int &counter, int &startx, int &starty, int &endx, int &endy, int &currentlayer, int &prevlayer,
	string &str_route, string &part1, string &part2, string &viapart, bool mode)
{
	if (!counter)
	{
		endx = startx = p.second*myparser.get_track_step_x();
		endy = starty = p.third*myparser.get_track_step_y();
		currentlayer = p.first;
	}
	if ((currentlayer != p.first) || ((Path.empty()) && mode == true) || (actual_coordinates.empty() && mode == false))
	{
		part1 = " ( " + to_string(startx) + " " + to_string(starty) + " ) ";
		if ((startx == endx) && (starty == endy))
			part2 = part1;
		else if (startx == endx)
			part2 = " ( * " + to_string(endy) + " )";
		else if (starty == endy)
			part2 = " ( " + to_string(endx) + " * )";
		else
			part2 = " ( " + to_string(endx) + " " + to_string(endy) + " )";

		if (p.first == currentlayer)
			viapart = "";
		else
		{
			if (currentlayer > p.first)
				viapart = " M" + to_string(currentlayer) + "_M" + to_string(p.first);
			else
				viapart = " M" + to_string(p.first) + "_M" + to_string(currentlayer);
		}


		str_route = "metal" + to_string(currentlayer) + part1 + part2 + viapart;
		DEFRoute[net_name].push_back(str_route);
		currentlayer = p.first;
		startx = p.second*myparser.get_track_step_x();
		starty = p.third*myparser.get_track_step_y();
	}
}
void PrintDEF(string filename)
{
	ifstream readdef;
	ofstream writedef;
	string current;
	vector<string> temp;
	bool flag = false;

	readdef.open(filename.c_str());
	writedef.open(filename.substr(0, filename.find(".")) + "_routed.def");

	while (getline(readdef, current))
	{
		if (current.find("NETS") == 0)
		{
			writedef << current << '\n';

			while (getline(readdef, current))
			{
				if (current == "END NETS")
				{
					for (int i = 0; i < temp.size(); ++i)
					{
						if (!i)
							writedef << "+ ROUTED ";
						else
							writedef << "NEW ";
						writedef << temp[i];

						if (i == temp.size() - 1)
							writedef << ";" << '\n';
						else
							writedef << '\n';

					}
					writedef << current << '\n';
					break;
				}

				if (current[0] == '-' && flag)
				{
					for (int i = 0; i < temp.size(); ++i)
					{
						if (!i)
							writedef << "+ ROUTED ";
						else
							writedef << "NEW ";

						writedef << temp[i];

						if (i == temp.size() - 1)
							writedef << ";" << '\n';
						else
							writedef << '\n';
					}

					temp = DEFRoute[current.substr(2)];
					writedef << current << '\n';
				}
				else if (current[0] == '-')
				{

					flag = true;
					temp = DEFRoute[current.substr(2)];
					writedef << current << '\n';
				}
				else
				{
					if (current.find(";") < current.length())
						current.erase(current.find(";"));

					writedef << current << '\n';
				}
			}
		}
		else
			writedef << current << '\n';
	}
}

// Function to calculate the H Value for the current search node in 3D space
double calculateHValue(triplet current, triplet destination)
{
	return abs(current.first - destination.first) + abs(current.second - destination.second) + abs(current.third - destination.third);
}

// Function that checks whether the destination has been reached
bool isDestination(triplet source, triplet destination)
{
	return (source == destination);
}


// FUNCTION TO CHECK WHETHER A POINT IN THE 3D MATRIX IS BLOCKED
// PARAMETERS:
//   - (GRID) 3D GRID
//   - (P) 3D POINT
bool isUnBlocked(ThreeDimensions& Grid, triplet p, cell*** celldetails, int mode)
{
	if ((Grid[p.first][p.second][p.third] != BLOCKED && mode == DETAILED) || (mode == GLOBAL && Grid[p.first][p.second][p.third] < congestion))
		return (true);
	else
		return (false);
}
bool validate_point_checkVector(triplet checkingUpon)
{
	for (int i = 0; i < checkVector.size(); i++)
	{
		if (checkingUpon == checkVector[i])
			return true;
	}
	return false;
}

// Check whether the cell to visit is inside the boundaries of my 3D Matrix
// PARAMETERS:
//   - (c) 3D POINT TO TEST
//   - (X) SIZE FOR FIRST DIMENSION FOR GRID
//   - (Y) SIZE FOR SECOND DIMENSION FOR GRID
//   - (Z) SIZE FOR THIRD DIMENSION FOR GRID
//

bool isValid(triplet c, ThreeDimensions &Grid, int mode)
{
	int xx = c.first, yy = c.second, zz = c.third;
	if ((xx <= Grid.size() - 1 && xx >= 1) && (yy < Grid[xx].size() && yy >= 0) && (zz < Grid[xx][yy].size() && zz >= 0)) {
		if (mode == DETAILED)
		{
			return (validate_point_checkVector(triplet(xx, yy / GBOXsize, zz / GBOXsize)));
		}
		return true;
	}
	return false;
}


string find_place(int delta_i, int delta_j, int delta_k) {

	if (delta_i < 0)
		return "UP";
	else if (delta_i > 0)
		return "DOWN";
	else if (delta_j < 0)
		return "SOUTH";
	else if (delta_j > 0)
		return "NORTH";
	else if (delta_k < 0)
		return "EAST";
	else if (delta_k > 0)
		return "WEST";


	return "N/A";


}


// A Utility Function to trace the path from the source
// to destination
void tracePath(cell*** cellDetails, triplet dest, ThreeDimensions& Grid, bool mode)
{
	if (enable_output)
		printf("\nThe Path is \n");
	int i = dest.first;
	int j = dest.second;
	int k = dest.third;

	while (!(cellDetails[i][j][k].parent_i == i && cellDetails[i][j][k].parent_j == j && cellDetails[i][j][k].parent_k == k))
	{

		Path.push(triplet(i, j, k));

		int temp_i = cellDetails[i][j][k].parent_i;
		int temp_j = cellDetails[i][j][k].parent_j;
		int temp_k = cellDetails[i][j][k].parent_k;

		actual_path.push(find_place(temp_i - i, temp_j - j, temp_k - k));

		i = temp_i;
		j = temp_j;
		k = temp_k;

	}

	Path.push(triplet(i, j, k));

	int startx, starty;
	int endx, endy;
	int currentlayer, prevlayer;
	string str_route;
	string part1, part2;
	string viapart;

	int counter = 0, total_size = Path.size();
	while (!Path.empty())
	{
		triplet p = Path.top();
		Path.pop();
		if (enable_output)
			printf("(%d,%d,%d) \n", p.first, p.second*myparser.get_track_step_x(), p.third*myparser.get_track_step_y());
		if (mode == DETAILED && !global_detailed)
		{
			create_output(p, counter, startx, starty, endx, endy, currentlayer, prevlayer, str_route, part1, part2, viapart, true);
			endx = p.second*myparser.get_track_step_x();
			endy = p.third*myparser.get_track_step_y();
			counter++;
		}
		// SHARED : SHOULD BE CHANGED TO BLOCKED AFTERWARDS

		if (counter != 0 && counter != total_size - 1)
		{
			if (mode == DETAILED)  // IF DETAILED AND BLOCK LATER
				Grid[p.first][p.second][p.third] = BLOCKED;
			else if (mode == GLOBAL) // IF GLOBAL ROUTING
				Grid[p.first][p.second][p.third]++;
		}
		if (mode == GLOBAL)
			counter++;
	}

	//cout << "Path in Moves: " << endl;
	while (!actual_path.empty()) {
		//cout << "MOVE " << actual_path.top() << endl;
		directions.push_back(actual_path.top());
		actual_path.pop();
	}
	if (global_detailed == true)
	{
		int counter = 0, total_size = actual_path.size();

		int coordinatesi = metal_temp, coordinatesj = prev_pins_coordinates_temp.first, coordinatesk = prev_pins_coordinates_temp.second;
		block_grid(Grid, directions, coordinatesi, coordinatesj, coordinatesk, false);
		if (enable_output)
			cout << "Actual Final Path Coordinates: " << '\n';
		while (!actual_coordinates.empty()) {
			triplet p = actual_coordinates.top();
			if (enable_output)
				printf("(%d,%d,%d) \n", p.first, p.second, p.third);
			actual_coordinates.pop();
			create_output(p, counter, startx, starty, endx, endy, currentlayer, prevlayer, str_route, part1, part2, viapart, false);
			endx = p.second*myparser.get_track_step_x();
			endy = p.third*myparser.get_track_step_y();
			counter++;
		}
	}
	return;
}


bool generate_and_check_dest(triplet new_node, triplet parent_node, cell*** cellDetails, triplet destination_node,
	bool ***closedList, priority_queue <search_node, vector<search_node>, greater<search_node>>& pq,
	ThreeDimensions& Grid, triplet grid_size, bool up_down, bool mode) {

	double newG, newH, newF;

	// Only process this cell if this is a valid one
	if (isValid(new_node, Grid,mode) == true)
	{
		// If the destination cell is the same as the
		// current successor

		if (isDestination(new_node, destination_node) == true)
		{
			// Set the Parent of the destination cell
			cellDetails[new_node.first][new_node.second][new_node.third].parent_i = parent_node.first;
			cellDetails[new_node.first][new_node.second][new_node.third].parent_j = parent_node.second;
			cellDetails[new_node.first][new_node.second][new_node.third].parent_k = parent_node.third;
			if (enable_output)
				printf("The destination cell is found\n");
			tracePath(cellDetails, destination_node, Grid, mode);
			for (int i = 1; i<Grid.size(); i++) {
				for (int j = 0; j<Grid[i].size(); j++) {
					delete[] cellDetails[i][j];
				}
				delete[] cellDetails[i];
			}
			delete[] cellDetails;
			for (int i = 1; i<Grid.size(); i++) {
				for (int j = 0; j<Grid[i].size(); j++) {
					delete[] closedList[i][j];
				}
				delete[] closedList[i];
			}
			delete[] closedList;
			return true;
		}

		// If the successor is already on the closed
		// list or if it is blocked, then ignore it.
		// Else do the following
		else if (closedList[new_node.first][new_node.second][new_node.third] == false && isUnBlocked(Grid, new_node, cellDetails, mode) == true)
		{
			newG = cellDetails[parent_node.first][parent_node.second][parent_node.third].g + 1.0;
			newG = (up_down) ? (newG + 50) : (newG);


			newH = calculateHValue(new_node, destination_node);

			if (Grid[new_node.first][new_node.second][new_node.third] == SHARED)
			{
				newG -= 2;
				newH = newH * 0.5;
			}
			else {
				newH += Grid[new_node.first][new_node.second][new_node.third];
			}

			newF = newG + newH;

			// If it isnt on the open list, add it to
			// the open list. Make the current square
			// the parent of this square. Record the
			// f, g, and h costs of the square cell
			//                OR
			// If it is on the open list already, check
			// to see if this path to that square is better,
			// using 'f' cost as the measure.
			if (cellDetails[new_node.first][new_node.second][new_node.third].f == DBL_MAX ||
				cellDetails[new_node.first][new_node.second][new_node.third].f > newF)
			{
				pq.push(search_node(newF, new_node));

				// Update the details of this cell
				cellDetails[new_node.first][new_node.second][new_node.third].f = newF;
				cellDetails[new_node.first][new_node.second][new_node.third].g = newG;
				cellDetails[new_node.first][new_node.second][new_node.third].h = newH;
				cellDetails[new_node.first][new_node.second][new_node.third].parent_i = parent_node.first;
				cellDetails[new_node.first][new_node.second][new_node.third].parent_j = parent_node.second;
				cellDetails[new_node.first][new_node.second][new_node.third].parent_k = parent_node.third;

			}
		}
	}

	return false;
}


void aStarSearch(ThreeDimensions& Grid, triplet src, triplet dest, bool mode) {

	//vector<vector<vector<cell>>> cellDetails;
	//vector<vector<vector<bool>>> closedList;
	bool ***closedList = new bool**[Grid.size()];
	cell *** cellDetails = new cell**[Grid.size()];

	triplet gridSize;

	gridSize.first = Grid.size();

	for (int i = 1; i < Grid.size(); i++) {
		cellDetails[i] = new cell*[Grid[i].size()];
		closedList[i] = new bool*[Grid[i].size()];
		gridSize.second = Grid[i].size();
		for (int j = 0; j < Grid[i].size(); j++) {
			cellDetails[i][j] = new cell[Grid[i][j].size()];
			closedList[i][j] = new bool[Grid[i][j].size()];
			gridSize.third = Grid[i][j].size();
			for (int k = 0; k < Grid[i][j].size(); k++)
			{
				cellDetails[i][j][k].f = DBL_MAX;
				cellDetails[i][j][k].g = DBL_MAX;
				cellDetails[i][j][k].h = DBL_MAX;
				cellDetails[i][j][k].parent_i = -1;
				cellDetails[i][j][k].parent_j = -1;
				cellDetails[i][j][k].parent_k = -1;

				closedList[i][j][k] = false;
			}
		}
	}

	int x, y, z;
	// Initialising the parameters of the starting node
	x = src.first; y = src.second; z = src.third;
	cellDetails[x][y][z].f = 0.0;
	cellDetails[x][y][z].g = 0.0;
	cellDetails[x][y][z].h = 0.0;
	cellDetails[x][y][z].parent_i = x;
	cellDetails[x][y][z].parent_j = y;
	cellDetails[x][y][z].parent_k = z;

	priority_queue <search_node, vector<search_node>, greater<search_node>> pq;
	pq.push(search_node(0.0, triplet(x, y, z)));

	while (!pq.empty()) {
		time_t now = time(0);
		tm* gmtm = gmtime(&now);
		int x = abs((save_time_hour * 60 + save_time_min) - (gmtm->tm_hour * 60 + gmtm->tm_min));
		/*if ((mode == DETAILED && x >= 3) && mode != GLOBAL)
		{
			cout << " exceeded my time limit \n";
			time_out_counter++;
			return;
		}*/
		search_node current = pq.top();
		pq.pop();

		// Add this vertex to the open list
		x = current.point.first;
		y = current.point.second;
		z = current.point.third;
		closedList[x][y][z] = true;

		int newI, newJ, newK;
		triplet new_node;


		//----------- 1st Successor (North) ------------
		newI = x;
		newJ = y - 1;
		newK = z;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 1 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;
		
		//----------- 2nd Successor (South) ------------
		newI = x;
		newJ = y + 1;
		newK = z;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 1 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;
		//----------- 3rd Successor (East) ------------
		newI = x;
		newJ = y;
		newK = z + 1;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 0 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;
		
		//----------- 4th Successor (West) ------------
		newI = x;
		newJ = y;
		newK = z - 1;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 0 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;

		
		//----------- 5th Successor (Up) ------------
		newI = x - 1;
		newJ = y;
		newK = z;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, true, mode))
			return;

		//----------- 6th Successor (Down) ------------

		newI = x + 1;
		newJ = y;
		newK = z;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, true, mode))
			return;
	}
	//cout << "FAILED TO ROUTE!!\n";

	for (int i = 1; i<Grid.size(); i++) {
		for (int j = 0; j<Grid[i].size(); j++) {
			delete[] cellDetails[i][j];
		}
		delete[] cellDetails[i];
	}
	delete[] cellDetails;
	for (int i = 1; i<Grid.size(); i++) {
		for (int j = 0; j<Grid[i].size(); j++) {
			delete[] closedList[i][j];
		}
		delete[] closedList[i];
	}
	delete[] closedList;
	if (mode == GLOBAL)
		++global_fail;
	else
		++detailed_fail;
}


ThreeDimensions createGlobalGrid(ThreeDimensions OriginalGrid)
{
	ThreeDimensions GlobalGrid;
	int newX, newY;

	GlobalGrid.resize(OriginalGrid.size());

	for (int i = 1; i < OriginalGrid.size(); i++)
	{
		newX = ceil((OriginalGrid[i].size() / float(GBOXsize))) + 1;
		GlobalGrid[i].resize(newX);
		for (int j = 0; j < newX; j++)
		{
			newY = ceil((OriginalGrid[i][j].size() / float(GBOXsize))) + 1;
			GlobalGrid[i][j].resize(newY);
		}
	}

	return GlobalGrid;
}


void  GlobalRouting(ThreeDimensions GlobalGrid, ThreeDimensions grid)
{
	p current_pins_coordinates;
	p prev_pins_coordinates;
	int metal1, metal2;
	int OutCount = 0;
	ThreeDimensions tempPath;
	vector<string> nets;
	nets = myparser.get_net_names();
	for (int q = 0; q < nets.size(); ++q) {
		vector<pair<triplet, triplet>> routing_coordinates;
		routing_coordinates = myparser.get_net_pairs(nets[q]);
		for (int L = 0; L < routing_coordinates.size(); ++L)
		{
			pair<int, int> new_prev_pins_coordinates, new_current_pins_coordinates;

			cout << ++OutCount << '\n';
			net_name = nets[q];
		
			current_pins_coordinates = make_pair(routing_coordinates[L].first.first, routing_coordinates[L].first.second);
			prev_pins_coordinates = make_pair(routing_coordinates[L].second.first, routing_coordinates[L].second.second);
			metal1 = routing_coordinates[L].first.third;
			metal2 = routing_coordinates[L].second.third;

			triplet globalsrc = triplet(metal1, (prev_pins_coordinates.first / GBOXsize), (prev_pins_coordinates.second / GBOXsize));
			triplet globaltarget = triplet(metal2, (current_pins_coordinates.first / GBOXsize), (current_pins_coordinates.second / GBOXsize));
			if (enable_output) {
				cout << "Src:  ( " << metal1 << ", " << globalsrc.second << ", " << globalsrc.third << ")\n";
				cout << "Dest:  ( " << metal2 << ", " << globaltarget.second << ", " << globaltarget.third << ")\n";
			}
			int temp_global = global_fail;
			bool flag_global = false;
			if (globalsrc.first != globaltarget.first || globalsrc.second != globaltarget.second || globalsrc.third != globaltarget.third)
			{

				aStarSearch(GlobalGrid, globalsrc, globaltarget, GLOBAL);
				flag_global = true;
			}
			else
			{
				//save_time();

				int start_box_x = prev_pins_coordinates.first / GBOXsize, start_box_y = prev_pins_coordinates.second / GBOXsize;

				global_detailed = true;
				prev_pins_coordinates_temp = prev_pins_coordinates;
				metal_temp = metal1;
				for (int z_dimension = 1; z_dimension < 5; ++z_dimension)
					checkVector.push_back(triplet(z_dimension, start_box_x, start_box_y));
				aStarSearch(grid, (triplet(metal1, prev_pins_coordinates.first, prev_pins_coordinates.second)),
					triplet(metal2, current_pins_coordinates.first, current_pins_coordinates.second), DETAILED);		
				int coordinatesi = metal1, coordinatesj = prev_pins_coordinates.first, coordinatesk = prev_pins_coordinates.second;
				block_grid(grid, directions, coordinatesi, coordinatesj, coordinatesk, true);
				checkVector.clear();
				global_detailed = false;
			}
			if (enable_output)
				cout << "Src:  ( " << metal1 << ", " << (prev_pins_coordinates.first / GBOXsize) << ", " << (prev_pins_coordinates.second / GBOXsize) << ")\n";

			if (global_fail == temp_global && flag_global)
			{
				//cout << "global failure " << global_fail << "\tdetailed failure " << detailed_fail << "\ttime out failure " << time_out_counter << '\n';
			int start_box_x = prev_pins_coordinates.first / GBOXsize, start_box_y = prev_pins_coordinates.second / GBOXsize;
			for (int z_dimension = 1; z_dimension < 5; ++z_dimension)
				checkVector.push_back(triplet(z_dimension, start_box_x, start_box_y));
			//should i make an initial box?
			for (int i = 0; i < directions.size(); ++i)
			{
				string temp = directions[i];
				if (temp == "NORTH")
				{
					--start_box_x;
					for (int z_dimension = 1; z_dimension < 5; ++z_dimension)
						checkVector.push_back(triplet(z_dimension, start_box_x, start_box_y));
				}
				else if (temp == "SOUTH")
				{
					++start_box_x;
					for (int z_dimension = 1; z_dimension < 5; ++z_dimension)
						checkVector.push_back(triplet(z_dimension, start_box_x, start_box_y));
				}
				else if (temp == "EAST")
				{
					++start_box_y;
					for (int z_dimension = 1; z_dimension < 5; ++z_dimension)
						checkVector.push_back(triplet(z_dimension, start_box_x, start_box_y));
				}
				else if (temp == "WEST")
				{
					--start_box_y;
					for (int z_dimension = 1; z_dimension < 5; ++z_dimension)
						checkVector.push_back(triplet(z_dimension, start_box_x, start_box_y));
				}
			}
				//Start Detailed Routing
				if (enable_output) {
					cout << "DETAILED!!! \n";
					cout << "x src " << new_prev_pins_coordinates.first << " y src " << new_prev_pins_coordinates.second << "\n";
					cout << "x target" << new_current_pins_coordinates.first << " y target" << new_current_pins_coordinates.second << "\n";
				}

				int temp_fail = detailed_fail;

				global_detailed = true;
				prev_pins_coordinates_temp = prev_pins_coordinates;
				metal_temp = metal1;
				//save_time();
				directions.clear();
				aStarSearch(grid, (triplet(metal1, prev_pins_coordinates.first, prev_pins_coordinates.second)),
					triplet(metal2, current_pins_coordinates.first, current_pins_coordinates.second), DETAILED);
				global_detailed = false;
				tempPath.clear();
				checkVector.clear();
				pathcoordinates.clear();
			}
			else if (flag_global) {
				failed_routing.push_back(make_pair(triplet(current_pins_coordinates.first, current_pins_coordinates.second, metal2),
					triplet(prev_pins_coordinates.first, prev_pins_coordinates.second, metal1)));
				failed_routing_name.push_back(net_name);
			}
			int coordinatesi = metal1, coordinatesj = prev_pins_coordinates.first, coordinatesk = prev_pins_coordinates.second;
			block_grid(grid, directions, coordinatesi, coordinatesj, coordinatesk, true);

			if (enable_output) cout << "Actual Final Path Coordinates: \n";

			while (!actual_coordinates.empty()) {
				triplet temp_final = actual_coordinates.top();
				if (enable_output) printf("(%d,%d,%d) \n", temp_final.first, temp_final.second, temp_final.third);

				actual_coordinates.pop();
			}

			prev_pins_coordinates = current_pins_coordinates;
			metal1 = metal2;
		}
	}
}


int main(int argc, char* argv[])
//int main()
{
	int x, y;
	if (argc < 6)
	{
		cout << " Wrong number of input \n";
		return EXIT_FAILURE;
	}
	else {
		myparser.set_def(argv[1]);
		myparser.set_lef(argv[2]);
		GBOXsize = stoi(argv[3]);
		congestion = stoi(argv[4]);
		enable_output = stoi(argv[5]);
		

		//myparser.set_def("alu_divider_unroute.def");
		//myparser.set_lef("osu035.lef");
		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		
		myparser.Parse_DEF();
		myparser.Parse_LEF();
		if (enable_output)
			myparser.print_output();
		ThreeDimensions grid;
		myparser.create_grid(grid);
		ThreeDimensions GlobalMatrix;
		myparser.order_the_nets();

		GlobalMatrix = createGlobalGrid(grid);
		myparser.getGridDimensions(x, y);
		save_time();
	
		GlobalRouting(GlobalMatrix, grid);
		for (int i = 0; i < failed_routing.size(); i++)
		{
			net_name = failed_routing_name[i];

			prev_pins_coordinates_temp = make_pair(failed_routing[i].second.first, failed_routing[i].second.second);
			metal_temp = failed_routing[i].first.third;
			aStarSearch(grid, (triplet(failed_routing[i].second.third, failed_routing[i].second.first, failed_routing[i].second.second)),
				triplet(failed_routing[i].first.third, failed_routing[i].first.first, failed_routing[i].first.second), DETAILED);
			int coordinatesi = failed_routing[i].second.third, coordinatesj = failed_routing[i].second.first,
				coordinatesk = failed_routing[i].second.second;
			block_grid(grid, directions, coordinatesi, coordinatesj, coordinatesk, true);
		}
		cout << " this is global failure " << global_fail << "\n this is detailed failure " << detailed_fail << '\n';
		PrintDEF(argv[1]);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();

		auto duration = duration_cast<microseconds>(t2 - t1).count();
		int min = (duration / 1000000) / 60;
		int seconds = int(duration / 1000000) % 60;
		cout << "it took " << min << "mins, and " << seconds << "seconds\n";
		//PrintDEF("alu_divider_unroute.def");
	}
	system("pause");
	return 0;
}