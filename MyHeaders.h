#ifndef MYHEADERS_H
#define MYHEADERS_H
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <functional>
#include <utility>
using namespace std;
typedef pair<int, int> p;
typedef pair<int, pair<int, int>> pp;

struct cell {
	int parent_i, parent_j, parent_k;
	double f, g, h;
	int capacity;
};

struct triplet {
	int first, second, third;

	triplet() {
		first = 0;
		second = 0;
		third = 0;
	}
	triplet(int x, int y, int z) {
		first = x;
		second = y;
		third = z;
	}

	bool operator == (triplet& z) {
		return (first == z.first && second == z.second && third == z.third);
	}
};

class mycomparison
{
	bool reverse;
public:
	mycomparison(const bool& revparam = false)
	{
		reverse = revparam;
	}
	bool operator() (const pair<string,int> & lhs, const pair<string,int> &rhs) const
	{
		if (reverse) return (lhs.second>rhs.second);
		else return (lhs.second<rhs.second);
	}
};

struct search_node {
	double f;
	triplet point;

	search_node(double ff, triplet p) {
		f = ff;
		point = p;
	}

	bool operator > (const search_node& other) const {
		return f > other.f;
	}

	bool operator < (const search_node& other)const {
		return f < other.f;
	}

};

typedef priority_queue<pair<string,int>, vector<pair<string,int>>, mycomparison> mypq_type;

typedef vector<int> OneDimension;
typedef vector<OneDimension> TwoDimensions;
typedef vector<TwoDimensions> ThreeDimensions;

typedef vector<bool> OneDimensionBool;
typedef vector<OneDimensionBool> TwoDimensionsBool;
typedef vector<TwoDimensionsBool> ThreeDimensionsBool;

typedef vector<cell> OneDimensionCell;
typedef vector<OneDimensionCell> TwoDimensionsCell;
typedef vector<TwoDimensionsCell> ThreeDimensionsCell;


struct rect {
	string pin_name;
	float x1, x2, y1, y2;
};
struct pin_info {
	int metal_layer;
	int x, y;
	string connected_pin;
};
struct gate_info {
	int x, y;
	string orientation;
	vector<pair<string, pin_info>> pins_connections; //first is the pin, second is the connected wire to that pin
};
struct mystruct {
	unordered_map<string, gate_info> connected_gates;
	vector <pair<string, rect>> pins_sizes;
	bool i_o_pin = false;
	float size_x, size_y;
};

struct StringNames {
	string netname;
	string gatename;
	string pinname;
};

struct tracks {
	int first, second, third;
	char orientation;
	float pitch, offset, width, spacing, via_spacing;
	bool found_before = false, found_via = false;
};
struct EmptyStr {
	bool operator()(const string& s) {
		return (s == "");
	}
};
struct my_cmp {
	bool operator()(StringNames a, StringNames b) const {
		return (a.netname < b.netname);
	}
};

#endif