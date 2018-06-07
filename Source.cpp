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
#include "flute.c"
//using namespace std;
#define GLOBAL 1
#define DETAILED 0

const int UNBLOCKED = 0;
const int BLOCKED = -1;
const int SHARED = -2;
int GBOXsize = 5;
int congestion = 50;
int enable_output = 0;
int time_out_counter;
int save_time_min, save_time_hour;
int global_fail, detailed_fail;
int metal_temp;

bool global_detailed;
std::stack<str> actual_path;
std::stack<triplet>actual_coordinates;
std::vector<str> directions;

Parser myparser;
std::stack<triplet> tempstack2;
std::stack<triplet> Path;
std::unordered_map<int, std::pair<int, int>> pathcoordinates;
std::vector <std::pair<triplet, triplet>> failed_routing;
std::vector <str> failed_routing_name;

std::pair <int, int> prev_pins_coordinates_temp;

str net_name;
std::unordered_map<str, std::vector<str>> DEFRoute;
void save_time()
{
	time_t now = time(0);
	tm* gmtm = gmtime(&now);
	save_time_min = gmtm->tm_min;
	save_time_hour = gmtm->tm_hour;
}
void block_grid(ThreeDimensions &grid, std::vector <str> &directions, int &coordinatesi, int &coordinatesj, int &coordinatesk, bool block)
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
	str &str_route, str &part1, str &part2, str &viapart, bool mode)
{
	if (!counter)
	{
		endx = startx = p.second*myparser.get_track_step_x();
		endy = starty = p.third*myparser.get_track_step_y();
		currentlayer = p.first;
	}
	if ((currentlayer != p.first) || ((Path.empty()) && mode == true) || (actual_coordinates.empty() && mode == false))
	{
		part1 = " ( " + std::to_string(startx) + " " + std::to_string(starty) + " ) ";
		if ((startx == endx) && (starty == endy))
			part2 = part1;
		else if (startx == endx)
			part2 = " ( * " + std::to_string(endy) + " )";
		else if (starty == endy)
			part2 = " ( " + std::to_string(endx) + " * )";
		else
			part2 = " ( " + std::to_string(endx) + " " + std::to_string(endy) + " )";

		if (p.first == currentlayer)
			viapart = "";
		else
		{
			if (currentlayer > p.first)
				viapart = " M" + std::to_string(currentlayer) + "_M" + std::to_string(p.first);
			else
				viapart = " M" + std::to_string(p.first) + "_M" + std::to_string(currentlayer);
		}


		str_route = "metal" + std::to_string(currentlayer) + part1 + part2 + viapart;
		DEFRoute[net_name].push_back(str_route);
		currentlayer = p.first;
		startx = p.second*myparser.get_track_step_x();
		starty = p.third*myparser.get_track_step_y();
	}
}
void PrintDEF(str filename)
{
	std::ifstream readdef;
	std::ofstream writedef;
	str current;
	std::vector<str> temp;
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
double calculateHValue(triplet current, triplet destination, ThreeDimensionsCell cell)
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
bool isUnBlocked(ThreeDimensions& Grid, triplet p, ThreeDimensionsCell& celldetails, int mode)
{
	if ((Grid[p.first][p.second][p.third] != BLOCKED && mode == DETAILED) || (mode == GLOBAL && Grid[p.first][p.second][p.third] < congestion))
		return (true);
	else
		return (false);
}


// Check whether the cell to visit is inside the boundaries of my 3D Matrix
// PARAMETERS:
//   - (c) 3D POINT TO TEST
//   - (X) SIZE FOR FIRST DIMENSION FOR GRID
//   - (Y) SIZE FOR SECOND DIMENSION FOR GRID
//   - (Z) SIZE FOR THIRD DIMENSION FOR GRID
//

bool isValid(triplet c, ThreeDimensions &Grid)
{
	int xx = c.first, yy = c.second, zz = c.third;
	return (xx <= Grid.size() - 1 && xx >= 1) && (yy < Grid[xx].size() && yy >= 0) && (zz < Grid[xx][yy].size() && zz >= 0);
}


str find_place(int delta_i, int delta_j, int delta_k) {

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
void tracePath(std::vector<std::vector<std::vector<cell>>>& cellDetails, triplet dest, ThreeDimensions& Grid, bool mode)
{
	if (enable_output)
		printf("\nThe Path is \n");
	int i = dest.first;
	int j = dest.second;
	int k = dest.third;

	while (!(cellDetails[i][j][k].parent_i == i && cellDetails[i][j][k].parent_j == j && cellDetails[i][j][k].parent_k == k))
	{

		tempstack2.push(triplet(i, j, k));
		Path.push(triplet(i, j, k));

		int temp_i = cellDetails[i][j][k].parent_i;
		int temp_j = cellDetails[i][j][k].parent_j;
		int temp_k = cellDetails[i][j][k].parent_k;

		actual_path.push(find_place(temp_i - i, temp_j - j, temp_k - k));

		i = temp_i;
		j = temp_j;
		k = temp_k;

	}

	tempstack2.push(triplet(i, j, k));
	Path.push(triplet(i, j, k));

	int startx, starty;
	int endx, endy;
	int currentlayer, prevlayer;
	str str_route;
	str part1, part2;
	str viapart;

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
			std::cout << "Actual Final Path Coordinates: " << '\n';
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


bool generate_and_check_dest(triplet new_node, triplet parent_node, std::vector<std::vector<std::vector<cell>>>& cellDetails, triplet destination_node,
	std::vector<std::vector<std::vector<bool>>>& closedList, std::priority_queue <search_node, std::vector<search_node>, std::greater<search_node>>& pq,
	ThreeDimensions& Grid, triplet grid_size, bool up_down, bool mode) {

	double newG, newH, newF;

	// Only process this cell if this is a valid one
	if (isValid(new_node, Grid) == true)
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
			return true;
		}

		// If the successor is already on the closed
		// list or if it is blocked, then ignore it.
		// Else do the following
		else if (closedList[new_node.first][new_node.second][new_node.third] == false && isUnBlocked(Grid, new_node, cellDetails, mode) == true)
		{
			newG = cellDetails[parent_node.first][parent_node.second][parent_node.third].g + 1.0;
			newG = (up_down) ? (newG + 50) : (newG);


			newH = calculateHValue(new_node, destination_node, cellDetails);

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


	std::vector<std::vector<std::vector<cell>>> cellDetails;
	std::vector<std::vector<std::vector<bool>>> closedList;
	triplet gridSize;

	gridSize.first = Grid.size();
	cellDetails.resize(Grid.size());
	closedList.resize(Grid.size());


	for (int i = 1; i < Grid.size(); i++) {
		cellDetails[i].resize(Grid[i].size());
		closedList[i].resize(Grid[i].size());
		gridSize.second = Grid[i].size();
		for (int j = 0; j < Grid[i].size(); j++) {
			cellDetails[i][j].resize(Grid[i][j].size());
			closedList[i][j].resize(Grid[i][j].size());
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

	std::priority_queue <search_node, std::vector<search_node>, std::greater<search_node>> pq;
	pq.push(search_node(0.0, triplet(x, y, z)));

	while (!pq.empty()) {
		time_t now = time(0);
		tm* gmtm = gmtime(&now);
		int x = abs((save_time_hour * 60 + save_time_min) - (gmtm->tm_hour * 60 + gmtm->tm_min));
		if ((mode == DETAILED && x >= 3) && mode != GLOBAL)
		{
			std::cout << " exceeded my time limit \n";
			time_out_counter++;
			return;
		}
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

		if (x % 2 == 0 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;

		//----------- 2nd Successor (South) ------------
		newI = x;
		newJ = y + 1;
		newK = z;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 0 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;

		//----------- 3rd Successor (East) ------------
		newI = x;
		newJ = y;
		newK = z + 1;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 1 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
			return;

		//----------- 4th Successor (West) ------------
		newI = x;
		newJ = y;
		newK = z - 1;

		new_node.first = newI;
		new_node.second = newJ;
		new_node.third = newK;

		if (x % 2 == 1 && generate_and_check_dest(new_node, current.point, cellDetails, dest, closedList, pq, Grid, gridSize, false, mode))
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


void  GlobalRouting(ThreeDimensions GlobalGrid, std::vector <std::pair<std::pair<triplet, triplet>, std::pair<float, str>>> nets, ThreeDimensions grid)
{

	std::vector<std::pair<str, str>> net_pairs;
	p current_pins_coordinates;
	p prev_pins_coordinates;
	int metal1, metal2;
	int OutCount = 0;
	ThreeDimensions tempPath;

	for (int q = 0; q < nets.size(); ++q) {
		std::pair<int, int> new_prev_pins_coordinates, new_current_pins_coordinates;

		std::cout << ++OutCount << '\n';
		net_name = nets[q].second.second;

		current_pins_coordinates = std::make_pair(nets[q].first.first.first, nets[q].first.first.second);
		prev_pins_coordinates = std::make_pair(nets[q].first.second.first, nets[q].first.second.second);
		metal1 = nets[q].first.first.third;
		metal2 = nets[q].first.second.third;

		triplet globalsrc = triplet(metal1, (prev_pins_coordinates.first / GBOXsize), (prev_pins_coordinates.second / GBOXsize));
		triplet globaltarget = triplet(metal2, (current_pins_coordinates.first / GBOXsize), (current_pins_coordinates.second / GBOXsize));
		if (enable_output) {
			std::cout << "Src:  ( " << metal1 << ", " << globalsrc.second << ", " << globalsrc.third << ")\n";
			std::cout << "Dest:  ( " << metal2 << ", " << globaltarget.second << ", " << globaltarget.third << ")\n";
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
			save_time();
			ThreeDimensions my3dPath;

			triplet mysrc, mydest;
			mysrc = triplet(metal1, prev_pins_coordinates.first % GBOXsize, prev_pins_coordinates.second %GBOXsize);
			mydest = triplet(metal2, current_pins_coordinates.first % GBOXsize, current_pins_coordinates.second % GBOXsize);
			int start_box_x = prev_pins_coordinates.first / GBOXsize, start_box_y = prev_pins_coordinates.second / GBOXsize;
			int my_final_x_dimension = GBOXsize;
			int my_final_y_dimension = GBOXsize;
			if (start_box_x * GBOXsize + GBOXsize > grid[1].size())
				my_final_x_dimension = grid[1].size() - start_box_x * GBOXsize;
			if (start_box_y * GBOXsize + GBOXsize > grid[1][start_box_y*GBOXsize].size())
				my_final_y_dimension = grid[1][start_box_y * GBOXsize].size();
			my3dPath.resize(3);
			for (int i_dimension = 1; i_dimension < 3; ++i_dimension)
			{
				my3dPath[i_dimension].resize(my_final_x_dimension);
				for (int x_axis = 0; x_axis < my_final_x_dimension; ++x_axis)
					my3dPath[i_dimension][x_axis].resize(my_final_y_dimension);
			}

			for (int i_dimension = 1; i_dimension < 3; ++i_dimension)
			{
				for (int x_axis = 0; x_axis < my_final_x_dimension; ++x_axis)
				{
					for (int y_axis = 0; y_axis < my_final_y_dimension; ++y_axis)
					{
						my3dPath[i_dimension][x_axis][y_axis] = grid[i_dimension][start_box_x * GBOXsize + x_axis][start_box_y * GBOXsize + y_axis];
					}
				}
			}
			global_detailed = true;
			prev_pins_coordinates_temp = prev_pins_coordinates;
			metal_temp = metal1;
			aStarSearch(my3dPath, mysrc, mydest, DETAILED);
			int coordinatesi = metal1, coordinatesj = prev_pins_coordinates.first, coordinatesk = prev_pins_coordinates.second;
			block_grid(grid, directions, coordinatesi, coordinatesj, coordinatesk, true);
			global_detailed = false;
		}
		if (enable_output)
			std::cout << "Src:  ( " << metal1 << ", " << (prev_pins_coordinates.first / GBOXsize) << ", " << (prev_pins_coordinates.second / GBOXsize) << ")\n";

		if (global_fail == temp_global && flag_global)
		{
			std::cout << "global failure " << global_fail << "\tdetailed failure " << detailed_fail << "\ttime out failure " << time_out_counter << '\n';
			triplet  xsrc = triplet(0, 0, 0);
			xsrc = triplet(metal1, prev_pins_coordinates.first / GBOXsize, prev_pins_coordinates.second / GBOXsize);

			int xPathsize = 0, yPathsize = 0, zPathsize = 0;
			int xmin = 10, ymin = 1000, zmin = 1000;
			int xmax = 0, ymax = 0, zmax = 0;
			while (!tempstack2.empty())
			{
				triplet t = tempstack2.top();
				if (t.first < xmin)
					xmin = t.first;
				if (t.second < ymin)
					ymin = t.second;
				if (t.third < zmin)
					zmin = t.third;

				if (t.first > xmax)
					xmax = t.first;
				if (t.second > ymax)
					ymax = t.second;
				if (t.third > zmax)
					zmax = t.third;
				tempstack2.pop();

			}
			xPathsize = abs((xmax - xmin)) + 2;
			yPathsize = abs((ymax - ymin)) + 2;
			zPathsize = abs((zmax - zmin)) + 2;

			if (!directions.empty())
			{
				tempPath.resize(xPathsize);
				for (int i = 1; i < tempPath.size(); i++)
				{
					tempPath[i].resize(yPathsize * GBOXsize);
					for (int j = 0; j < tempPath[i].size(); j++)
						tempPath[i][j].resize(zPathsize * GBOXsize);
				}
			}
			else
			{
				tempPath.resize(xsrc.first + 1);
				for (int i = 1; i < tempPath.size(); i++)
				{
					tempPath[i].resize(GBOXsize);
					for (int j = 0; j < tempPath[i].size(); j++)
						tempPath[i][j].resize(GBOXsize);
				}
			}

			int pathi = xsrc.first, pathj = 0, pathk = 0;
			int coordinatesi = xsrc.first, coordinatesj = xsrc.second * GBOXsize, coordinatesk = xsrc.third * GBOXsize;
			for (int j = pathcoordinates[pathi].first; j < GBOXsize; j++)
			{
				for (int k = pathcoordinates[pathi].second; k < GBOXsize; k++)
				{
					if (coordinatesi == metal1 && (coordinatesj + j) == prev_pins_coordinates.first && (coordinatesk + k) == prev_pins_coordinates.second)
					{
						//cout << "FOUND THE SRC PIN!! " << endl;
						new_prev_pins_coordinates.first = j;
						new_prev_pins_coordinates.second = k;
					}
					if (coordinatesi == metal2 && (coordinatesj + j) == current_pins_coordinates.first && (coordinatesk + k) == current_pins_coordinates.second)
					{
						//cout << "FOUND THE DEST PIN!! " << endl;
						new_current_pins_coordinates.first = j;
						new_current_pins_coordinates.second = k;
					}
					if (coordinatesj + j < grid[coordinatesi].size() && coordinatesk + k < grid[coordinatesi][coordinatesj + j].size())
						tempPath[pathi][j][k] = grid[coordinatesi][coordinatesj + j][coordinatesk + k];
				}
			}
			pathcoordinates[pathi].first += GBOXsize;
			pathcoordinates[pathi].second += GBOXsize;

			//	cout << "Path in Moves: " << endl;
			for (int i = 0; i < directions.size(); i++)
			{
				if (directions[i] == "SOUTH")
				{
					int x = 0;
					for (int j = pathcoordinates[pathi].first; j < pathcoordinates[pathi].first + GBOXsize; j++)
					{
						for (int i = 0; i < GBOXsize; i++)
						{
							if (coordinatesi == metal2 && (coordinatesj + i) ==
								current_pins_coordinates.first && (coordinatesk + x) == current_pins_coordinates.second)
							{
								//		cout << "FOUND THE DEST PIN!! " << endl;
								new_current_pins_coordinates.first = pathcoordinates[pathi].first;
								new_current_pins_coordinates.second = j;
							}
						}
						if (j >= tempPath[1].size())
							tempPath.resize(j + GBOXsize);
						tempPath[pathi][j][pathcoordinates[pathi].second - 1] = grid[coordinatesi][coordinatesj][coordinatesk];
						coordinatesj++;
						x++;
					}
					pathcoordinates[pathi].first += GBOXsize;
				}
				if (directions[i] == "NORTH")
				{
					int x = 0;
					for (int j = pathcoordinates[pathi].first; j < pathcoordinates[pathi].first + GBOXsize; j++)
					{
						for (int i = 0; i < GBOXsize; i++)
						{
							if (coordinatesi == metal2 && (coordinatesj + i) == current_pins_coordinates.first && (coordinatesk + x) == current_pins_coordinates.second)
							{
								//		cout << "FOUND THE DEST PIN!! " << endl;
								new_current_pins_coordinates.first = pathcoordinates[pathi].first;
								new_current_pins_coordinates.second = j;
							}
						}
						if (j >= tempPath[1].size())
							tempPath.resize(j + GBOXsize);
						tempPath[pathi][j][pathcoordinates[pathi].second - 1] = grid[coordinatesi][coordinatesj][coordinatesk];
						coordinatesj--;
						x++;
					}
					pathcoordinates[pathi].first += GBOXsize;
				}
				if (directions[i] == "EAST")
				{

					int x = 0;
					coordinatesk += GBOXsize;
					for (int k = pathcoordinates[pathi].second; k < pathcoordinates[pathi].second + GBOXsize; k++)
					{

						for (int i = 0; i < GBOXsize; i++)
						{
							if (coordinatesi == metal2 && (coordinatesj + i) == current_pins_coordinates.first && (coordinatesk + x) == current_pins_coordinates.second)
							{
								//	cout << "FOUND THE DEST PIN!! " << endl;
								new_current_pins_coordinates.first = pathcoordinates[pathi].first;
								new_current_pins_coordinates.second = k;
							}
						}
						if (k >= tempPath[1][1].size())
							tempPath.resize(k + GBOXsize);
						tempPath[pathi][pathcoordinates[pathi].first - 1].push_back(grid[coordinatesi][coordinatesj][coordinatesk]);
						x++;
					}
					pathcoordinates[pathi].second += GBOXsize;
				}
				if (directions[i] == "WEST")
				{
					int x = 0;
					coordinatesk -= GBOXsize;
					for (int k = pathcoordinates[pathi].second; k < pathcoordinates[pathi].second + GBOXsize; k++)
					{
						for (int i = 0; i < GBOXsize; i++)
						{
							if (coordinatesi == metal2 && (coordinatesj + i) == current_pins_coordinates.first && (coordinatesk + x) == current_pins_coordinates.second)
							{
								//		cout << "FOUND THE DEST PIN!! " << endl;
								new_current_pins_coordinates.first = pathcoordinates[pathi].first;
								new_current_pins_coordinates.second = k;
							}
						}
						if (k >= tempPath[1][1].size())
							tempPath.resize(k + GBOXsize);
						tempPath[pathi][pathcoordinates[pathi].first - 1].push_back(grid[coordinatesi][coordinatesj][coordinatesk]);
						//	coordinatesk--;
						x++;
					}

					pathcoordinates[pathi].second += GBOXsize;

				}
				if (directions[i] == "UP")
				{
					coordinatesi++;
					pathi++;
					int x = 0;
					int y = 0;
					for (int j = pathcoordinates[pathi].first; j < pathcoordinates[pathi].first + GBOXsize; j++)
					{
						x = 0;
						for (int k = pathcoordinates[pathi].second; k < pathcoordinates[pathi].second + GBOXsize; k++)
						{
							if (coordinatesi == metal2 && (coordinatesj + y) == current_pins_coordinates.first && (coordinatesk + x) == current_pins_coordinates.second)
							{
								//		cout << "FOUND THE DEST PIN!! " << endl;
								new_current_pins_coordinates.first = j;
								new_current_pins_coordinates.second = k;
							}
							tempPath[pathi][j][k] = grid[coordinatesi][coordinatesj + y][coordinatesk + x];
							x++;
						}
						y++;
					}

					pathcoordinates[pathi].first += GBOXsize;
					pathcoordinates[pathi].second += GBOXsize;
				}
				if (directions[i] == "DOWN")
				{
					coordinatesi--;
					pathi--;
					int y = 0;
					int x = 0;

					for (int j = pathcoordinates[pathi].first; j < pathcoordinates[pathi].first + GBOXsize; j++)
					{
						x = 0;
						for (int k = pathcoordinates[pathi].second; k < pathcoordinates[pathi].second + GBOXsize; k++)
						{
							if (coordinatesi == metal2 && (coordinatesj + y) == current_pins_coordinates.first && (coordinatesk + x) == current_pins_coordinates.second)
							{
								//			cout << "FOUND THE DEST PIN!! " << endl;
								new_current_pins_coordinates.first = j;
								new_current_pins_coordinates.second = k;
							}
							if (coordinatesj + y < grid[coordinatesi].size() && coordinatesk + x < grid[coordinatesi][coordinatesj + y].size())
								tempPath[pathi][j][k] = grid[coordinatesi][coordinatesj + y][coordinatesk + x];
							x++;
						}
						y++;
					}
					pathcoordinates[pathi].first += GBOXsize;
					pathcoordinates[pathi].second += GBOXsize;
				}
			}
			directions.clear();
			//Start Detailed Routing
			if (enable_output) {
				std::cout << "DETAILED!!! \n";
				std::cout << "x src " << new_prev_pins_coordinates.first << " y src " << new_prev_pins_coordinates.second << "\n";
				std::cout << "x target" << new_current_pins_coordinates.first << " y target" << new_current_pins_coordinates.second << "\n";
			}

			int temp_fail = detailed_fail;

			global_detailed = true;
			prev_pins_coordinates_temp = prev_pins_coordinates;
			metal_temp = metal1;
			save_time();
			aStarSearch(tempPath, (triplet(metal1, new_prev_pins_coordinates.first, new_prev_pins_coordinates.second)),
				triplet(metal2, new_current_pins_coordinates.first, new_current_pins_coordinates.second), DETAILED);
			global_detailed = false;
			tempPath.clear();
			pathcoordinates.clear();
			while (!tempstack2.empty()) tempstack2.pop();
		}
		else if (flag_global) {
			failed_routing.push_back(std::make_pair(triplet(current_pins_coordinates.first, current_pins_coordinates.second, metal2),
				triplet(prev_pins_coordinates.first, prev_pins_coordinates.second, metal1)));
			failed_routing_name.push_back(net_name);

		}
		int coordinatesi = metal1, coordinatesj = prev_pins_coordinates.first, coordinatesk = prev_pins_coordinates.second;
		block_grid(grid, directions, coordinatesi, coordinatesj, coordinatesk, true);

		if (enable_output) std::cout << "Actual Final Path Coordinates: \n";

		while (!actual_coordinates.empty()) {
			triplet temp_final = actual_coordinates.top();
			if (enable_output) printf("(%d,%d,%d) \n", temp_final.first, temp_final.second, temp_final.third);

			actual_coordinates.pop();
		}

		prev_pins_coordinates = current_pins_coordinates;
		metal1 = metal2;
	}
}


//int main(int argc, char* argv[])
int main()
{
	int x, y;
/*	if (argc < 6)
	{
		std::cout << " Wrong number of input \n";
		return EXIT_FAILURE;
	}
	else {
		myparser.set_def(argv[1]);
		myparser.set_lef(argv[2]);
		GBOXsize = std::stoi(argv[3]);
		congestion = std::stoi(argv[4]);
		enable_output = std::stoi(argv[5]);*/
	int xx[10], yy[10];
	int d = 0;
	Tree flutetree;
	int flutewl;

for (int i = 0; i < 10; ++i){
	std::cin >> xx[i] >> yy[i];
	}
	readLUT();

	flutetree = flute(d, xx, yy, ACCURACY);
	printf("FLUTE wirelength = %d\n", flutetree.length);

	flutewl = flute_wl(d, xx, yy, ACCURACY);
	printf("FLUTE wirelength (without RSMT construction) = %d\n", flutewl);
	
			myparser.set_def("cpu_unroute.def");
			myparser.set_lef("osu035.lef");
		myparser.Parse_DEF();
		myparser.Parse_LEF();
		if (enable_output)
			myparser.print_output();
		ThreeDimensions grid;
		myparser.create_grid(grid);
		ThreeDimensions GlobalMatrix;
		mypq_type ordered_output = myparser.order_the_nets();

		GlobalMatrix = createGlobalGrid(grid);
		myparser.getGridDimensions(x, y);
		std::vector <std::pair<std::pair<triplet, triplet>, std::pair<float, str>>> myvector;
		int n = ordered_output.size();
		myvector.resize(n);
		//needs optimization 
		for (int i = 0; i < n; ++i) {
			std::pair<std::pair <triplet, triplet>, std::pair<float, str>> temp = ordered_output.top();
			ordered_output.pop();
			myvector[n - 1 - i] = temp;
		}
		GlobalRouting(GlobalMatrix, myvector, grid);
		myvector.clear();
		for (int i = 0; i < failed_routing.size(); ++i)
		{
			myvector.push_back(std::make_pair(std::make_pair(failed_routing[i].first, failed_routing[i].second), std::make_pair(0, failed_routing_name[i])));
		}
			std::cout << "take 2\n\n";
			GlobalRouting(GlobalMatrix, myvector, grid);
		std::cout << " this is global failure " << global_fail << "\n this is detailed failure " << detailed_fail << '\n';
		//	std::cout << " THIS IS THE SIZE OF FAILED_ROUTING " << failed_routing.size() << '\n';
		PrintDEF("cpu_unroute.def");
//	}
	system("pause");
	return 0;
}