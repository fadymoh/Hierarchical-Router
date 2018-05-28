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
#include<algorithm>
#include <functional>
#include <cfloat>
using namespace std;
#define GLOBAL 1
#define DETAILED 0

const int UNBLOCKED = 0;
const int BLOCKED = -1;
const int SHARED = -2;
int GBOXsize = 10;
int congestion = 150;
int time_out_counter;
int save_time_min, save_time_hour;
int failed = 0;
int fail_counter;
int global_fail, detailed_fail;
int metal_temp;

bool global_detailed;
vector<pair<stack<triplet>, stack<string>>> all_paths;
stack<string> actual_path;
stack<triplet>actual_coordinates;
vector<string> directions;
float percentage = 0;

Parser myparser;
stack<triplet> tempstack;
stack<triplet> tempstack2;
stack<triplet> Path;
unordered_map<int, pair<int, int>> pathcoordinates;
vector <pair<triplet, triplet>> failed_routing;
vector <string> failed_routing_name;


pair <int, int> prev_pins_coordinates_temp;

typedef pair<int, triplet> pt;

str net_name;
std::unordered_map<string, vector<string>> DEFRoute;


void PrintDEF(string filename/*, unordered_map<string, vector<string>> DEFRoute*/)
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
			writedef << current << endl;

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
							writedef << ";" << endl;
						else
							writedef << endl;

					}
					writedef << current << endl;
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
							writedef << ";" << endl;
						else
							writedef << endl;
					}

					temp = DEFRoute[current.substr(2)];
					writedef << current << endl;
				}
				else if (current[0] == '-')
				{

					flag = true;
					temp = DEFRoute[current.substr(2)];
					writedef << current << endl;
				}
				else
				{
					if (current.find(";") < current.length())
						current.erase(current.find(";"));

					writedef << current << endl;
				}
			}
		}
		else
			writedef << current << endl;
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
	if ((Grid[p.first][p.second][p.third] != BLOCKED && mode == DETAILED) || (Grid[p.first][p.second][p.third] < congestion && mode == GLOBAL))
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

bool isValid(triplet c, int x, int y, int z)
{
	int xx = c.first, yy = c.second, zz = c.third;
	return (xx <= x - 1 && xx >= 1) && (yy < y && yy >= 0) && (zz < z && zz >= 0);
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
void tracePath(vector<vector<vector<cell>>>& cellDetails, triplet dest, ThreeDimensions& Grid, bool mode)
{


	printf("\nThe Path is \n");
	if (mode == DETAILED)
	cout << "THIS IS DETAILED ROUTING!!!!!!\n";
	int i = dest.first;
	int j = dest.second;
	int k = dest.third;



	while (!(cellDetails[i][j][k].parent_i == i && cellDetails[i][j][k].parent_j == j && cellDetails[i][j][k].parent_k == k))
	{

		tempstack.push(triplet(i, j, k));
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


	tempstack.push(triplet(i, j, k));
	tempstack2.push(triplet(i, j, k));
	Path.push(triplet(i, j, k));

	all_paths.push_back(make_pair(Path, actual_path));
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
		printf("(%d,%d,%d) \n", p.first, p.second*myparser.get_track_step(p.first), p.third*myparser.get_track_step(p.first));
		if (mode == DETAILED && !global_detailed)
		{
			if (!counter)
			{
				endx = startx = p.second*myparser.get_track_step(p.first);
				endy = starty = p.third*myparser.get_track_step(p.first);
				currentlayer = p.first;
			}
			if ((currentlayer != p.first) || (Path.empty()))
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
				//str_route = "metal" + to_string(currentlayer) + part1 + part2 + viapart;
				DEFRoute[net_name].push_back(str_route);
				currentlayer = p.first;
				startx = p.second*myparser.get_track_step(p.first);
				starty = p.third*myparser.get_track_step(p.first);
			}
		}
		// SHARED : SHOULD BE CHANGED TO BLOCKED AFTERWARDS

		if (counter != 0 && counter != total_size - 1)
		{
			if (mode == DETAILED)  // IF DETAILED AND BLOCK LATER
				Grid[p.first][p.second][p.third] = SHARED;
			else if (mode == GLOBAL) // IF GLOBAL ROUTING
				Grid[p.first][p.second][p.third]++;
		}

		endx = p.second*myparser.get_track_step(p.first);
		endy = p.third*myparser.get_track_step(p.first);
		counter++;
	}

	cout << "Path in Moves: " << endl;
	while (!actual_path.empty()) {
		cout << "MOVE " << actual_path.top() << endl;
		directions.push_back(actual_path.top());
		actual_path.pop();
	}
	if (global_detailed == true)
	{
		int counter = 0, total_size = actual_path.size();

		int coordinatesi = metal_temp, coordinatesj = prev_pins_coordinates_temp.first, coordinatesk = prev_pins_coordinates_temp.second;
		for (int i = 0; i < directions.size(); i++)
		{
			if (directions[i] == "SOUTH")
			{
				coordinatesj++;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "NORTH")
			{
				coordinatesj--;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "EAST")
			{
				coordinatesk++;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "WEST")
			{
				coordinatesk--;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "UP")
			{

				coordinatesi++;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "DOWN")
			{
				coordinatesi--;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
		}
		directions.clear();
		//	cout << "Actual Final Path Coordinates: " << endl;
		while (!actual_coordinates.empty()) {
			triplet p = actual_coordinates.top();
			//	printf("(%d,%d,%d) \n", temp_final.first, temp_final.second, temp_final.third);
			actual_coordinates.pop();
			if (!counter)
			{
				endx = startx = p.second*myparser.get_track_step(p.first);
				endy = starty = p.third*myparser.get_track_step(p.first);
				currentlayer = p.first;
			}
			if ((currentlayer != p.first) || (actual_coordinates.empty()))
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
				//str_route = "metal" + to_string(currentlayer) + part1 + part2 + viapart;
				DEFRoute[net_name].push_back(str_route);
				currentlayer = p.first;
				startx = p.second*myparser.get_track_step(p.first);
				starty = p.third*myparser.get_track_step(p.first);
			}
			endx = p.second*myparser.get_track_step(p.first);
			endy = p.third*myparser.get_track_step(p.first);
			counter++;
		}
	}
	//	cout << endl;
	return;
}


bool generate_and_check_dest(triplet new_node, triplet parent_node, vector<vector<vector<cell>>>& cellDetails, triplet destination_node,
	vector<vector<vector<bool>>>& closedList, priority_queue <search_node, vector<search_node>, greater<search_node>>& pq, ThreeDimensions& Grid,
	triplet grid_size, bool up_down, bool mode) {



	double newG, newH, newF;


	// Only process this cell if this is a valid one
	if (isValid(new_node, grid_size.first, grid_size.second, grid_size.third) == true)
	{
		// If the destination cell is the same as the
		// current successor
		if (isDestination(new_node, destination_node) == true)
		{
			// Set the Parent of the destination cell
			cellDetails[new_node.first][new_node.second][new_node.third].parent_i = parent_node.first;
			cellDetails[new_node.first][new_node.second][new_node.third].parent_j = parent_node.second;
			cellDetails[new_node.first][new_node.second][new_node.third].parent_k = parent_node.third;
			//		printf("The destination cell is found\n");
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

			// If it isn�t on the open list, add it to
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


	vector<vector<vector<cell>>> cellDetails;
	vector<vector<vector<bool>>> closedList;
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

	priority_queue <search_node, vector<search_node>, greater<search_node>> pq;
	pq.push(search_node(0.0, triplet(x, y, z)));

	while (!pq.empty()) {
		time_t now = time(0);
		tm* gmtm = gmtime(&now);
		int x = abs((save_time_hour * 60 + save_time_min) - (gmtm->tm_hour * 60 + gmtm->tm_min));
		if ((mode == DETAILED && x >= 3) && mode != GLOBAL)
		{
			cout << " exceeded my time limit " << endl;
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
	++fail_counter;
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


void  GlobalRouting(ThreeDimensions GlobalGrid, vector <pair<pair<triplet, triplet>, pair<float, string>>> nets, ThreeDimensions grid)
{
	str gate_name;
	str pin_name;
	vector<pair<str, str>> net_pairs;
	p current_pins_coordinates;
	p prev_pins_coordinates;
	int metal1, metal2;
	int OutCount = 0;
	ThreeDimensions tempPath;

	for (int q = 0; q < nets.size(); ++q)
	{

		all_paths.clear();
		pair<int, int> new_prev_pins_coordinates, new_current_pins_coordinates;
		cout << net_name << '\t' << gate_name << '\t' << pin_name << '\t';
		cout << ++OutCount << endl;
		net_name = nets[q].second.second;
		//++OutCount;
		current_pins_coordinates = make_pair(nets[q].first.first.first, nets[q].first.first.second);
		prev_pins_coordinates = make_pair(nets[q].first.second.first, nets[q].first.second.second);
		metal1 = nets[q].first.first.third;
		metal2 = nets[q].first.second.third;

		triplet globalsrc = triplet(metal1, (prev_pins_coordinates.first / GBOXsize), (prev_pins_coordinates.second / GBOXsize));
		triplet globaltarget = triplet(metal2, (current_pins_coordinates.first / GBOXsize), (current_pins_coordinates.second / GBOXsize));

		cout << "Src:  ( " << metal1 << ", " << (prev_pins_coordinates.first / GBOXsize) << ", " << (prev_pins_coordinates.second / GBOXsize) << ")\n";
		cout << "Dest:  ( " << metal2 << ", " << (current_pins_coordinates.first / GBOXsize) << ", " << (current_pins_coordinates.second / GBOXsize) << ")\n";
		int temp_global = global_fail;
		bool flag_global = false;
		if (globalsrc.first != globaltarget.first || globalsrc.second != globaltarget.second || globalsrc.third != globaltarget.third)
		{
			aStarSearch(GlobalGrid, (triplet(metal1, (prev_pins_coordinates.first / GBOXsize), (prev_pins_coordinates.second / GBOXsize))), triplet(metal2, (current_pins_coordinates.first / GBOXsize), (current_pins_coordinates.second / GBOXsize)), GLOBAL);
			flag_global = true;
		}
		else
		{
			failed_routing.push_back(make_pair(triplet(metal1, prev_pins_coordinates.first, prev_pins_coordinates.second),
				triplet(metal2, current_pins_coordinates.first, current_pins_coordinates.second)));

			failed_routing_name.push_back(net_name);
		}
		cout << "Src:  ( " << metal1 << ", " << (prev_pins_coordinates.first / GBOXsize) << ", " << (prev_pins_coordinates.second / GBOXsize) << ")\n";

		if (global_fail == temp_global && flag_global)
		{
			cout << "global failure " << global_fail << "\tdetailed failure " << detailed_fail << "\ttime out failure " << time_out_counter << endl;
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
						cout << "FOUND THE SRC PIN!! " << endl;
						new_prev_pins_coordinates.first = j;
						new_prev_pins_coordinates.second = k;
					}
					if (coordinatesi == metal2 && (coordinatesj + j) == current_pins_coordinates.first && (coordinatesk + k) == current_pins_coordinates.second)
					{
						cout << "FOUND THE DEST PIN!! " << endl;
						new_current_pins_coordinates.first = j;
						new_current_pins_coordinates.second = k;
					}
					if (coordinatesj + j < grid[coordinatesi].size())
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
								cout << "FOUND THE DEST PIN!! " << endl;
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
								cout << "FOUND THE DEST PIN!! " << endl;
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
								cout << "FOUND THE DEST PIN!! " << endl;
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
								cout << "FOUND THE DEST PIN!! " << endl;
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
								cout << "FOUND THE DEST PIN!! " << endl;
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
								cout << "FOUND THE DEST PIN!! " << endl;
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
			}
			directions.clear();
			//Start Detailed Routing
			cout << "DETAILED!!! " << endl;
			cout << "x src " << new_prev_pins_coordinates.first << " y src " << new_prev_pins_coordinates.second << "\n";
			cout << "x target" << new_current_pins_coordinates.first << " y target" << new_current_pins_coordinates.second << "\n";

			int temp_fail = detailed_fail;
			time_t now = time(0);
			tm* gmtm = gmtime(&now);
			save_time_min = gmtm->tm_min;
			save_time_hour = gmtm->tm_hour;
			global_detailed = true;
			prev_pins_coordinates_temp = prev_pins_coordinates;
			metal_temp = metal1;
			aStarSearch(tempPath, (triplet(metal1, new_prev_pins_coordinates.first, new_prev_pins_coordinates.second)),
				triplet(metal2, new_current_pins_coordinates.first, new_current_pins_coordinates.second), DETAILED);
			global_detailed = false;
			tempPath.clear();
			pathcoordinates.clear();
			while (!tempstack.empty())
			{
				tempstack.pop();
			}
			while (!tempstack2.empty())
			{
				tempstack2.pop();
			}
		}
		else {
			failed_routing.push_back(make_pair(triplet(metal1, prev_pins_coordinates.first, prev_pins_coordinates.second),
				triplet(metal2, current_pins_coordinates.first, current_pins_coordinates.second)));
			failed_routing_name.push_back(net_name);
		}
		int coordinatesi = metal1, coordinatesj = prev_pins_coordinates.first, coordinatesk = prev_pins_coordinates.second;
		for (int i = 0; i < directions.size(); i++)
		{
			if (directions[i] == "SOUTH")
			{
				if (coordinatesj + 1 != grid[coordinatesi].size())
					coordinatesj++;
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "NORTH")
			{
				coordinatesj--;
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "EAST")
			{
				if (coordinatesk + 1 != grid[coordinatesi][coordinatesj].size())
					coordinatesk++;
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "WEST")
			{
				coordinatesk--;
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "UP")
			{

				coordinatesi++;
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
			if (directions[i] == "DOWN")
			{
				coordinatesi--;
				grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
				actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
			}
		}
		directions.clear();
		cout << "Actual Final Path Coordinates: " << endl;
		while (!actual_coordinates.empty()) {
			triplet temp_final = actual_coordinates.top();

			printf("(%d,%d,%d) \n", temp_final.first, temp_final.second, temp_final.third);
			actual_coordinates.pop();
		}


		prev_pins_coordinates = current_pins_coordinates;
		metal1 = metal2;
	}
}


int main(int argc, char* argv[])
{
	int x, y;
	int metal;
	if (argc < 5)
	{
		cout << " Wrong number of input " << endl;
		return EXIT_FAILURE;
	}
	else
	{
		myparser.set_def(argv[1]);
		myparser.set_lef(argv[2]);
		GBOXsize = stoi(argv[3]);
		congestion = stoi(argv[4]);

		myparser.Parse_DEF();
		myparser.Parse_LEF();
		myparser.print_output();
		ThreeDimensions grid;
		myparser.create_grid(grid);
		TwoDimensions matrix;
		ThreeDimensions GlobalMatrix;
		mypq_type ordered_output = myparser.order_the_nets();

		GlobalMatrix = createGlobalGrid(grid);
		myparser.getGridDimensions(x, y);
		vector <pair<pair<triplet, triplet>, pair<float, string>>> myvector;
		int n = ordered_output.size();
		myvector.resize(n);

		for (int i = 0; i < n; ++i) {
			std::pair<pair <triplet, triplet>, pair<float, string>> temp = ordered_output.top();
			ordered_output.pop();
			myvector[n - 1 - i] = temp;
		}
		int OutCount = 0;
		GlobalRouting(GlobalMatrix, myvector, grid);
		cout << " this is global failure " << global_fail << "\n this is detailed failure " << detailed_fail << endl;
		int nn = failed_routing.size();
		for (int q = 0; q < failed_routing.size(); ++q) {
			//Routing the pins in the priority queue
			cout << q << endl;
			net_name = failed_routing_name[q];
			all_paths.clear();
			time_t now = time(0);
			tm* gmtm = gmtime(&now);
			save_time_min = gmtm->tm_min;
			save_time_hour = gmtm->tm_hour;
			aStarSearch(grid, failed_routing[q].first, failed_routing[q].second, DETAILED);
			int coordinatesi = failed_routing[q].first.first, coordinatesj = failed_routing[q].first.second, coordinatesk = failed_routing[q].first.third;
			for (int i = 0; i < directions.size(); i++)
			{
				if (directions[i] == "SOUTH")
				{
					coordinatesj++;
					grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
					actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
				}
				if (directions[i] == "NORTH")
				{
					coordinatesj--;
					grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
					actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
				}
				if (directions[i] == "EAST")
				{
					coordinatesk++;
					grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
					actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
				}
				if (directions[i] == "WEST")
				{
					coordinatesk--;
					grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
					actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
				}
				if (directions[i] == "UP")
				{
					coordinatesi++;
					grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
					actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
				}
				if (directions[i] == "DOWN")
				{
					coordinatesi--;
					grid[coordinatesi][coordinatesj][coordinatesk] = BLOCKED;
					actual_coordinates.push(triplet(coordinatesi, coordinatesj, coordinatesk));
				}
			}
			directions.clear();
		}
		PrintDEF(argv[1]);
	}
	system("pause");
	return 0;
}
