#ifndef MYHEADERS_H
#define MYHEADERS_H
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <functional>
#include <utility>

//using namespace std;
typedef std::string str;
typedef std::pair<int, int> p;
typedef std::pair<int, std::pair<int, int>> pp;

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
	mycomparison(const bool& revparam = true)
	{
		reverse = revparam;
	}
	bool operator() (const std::pair<str,int> & lhs, const std::pair<str,int> &rhs) const
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

typedef std::priority_queue<std::pair<str,int>, std::vector<std::pair<str,int>>, mycomparison> mypq_type;

typedef std::vector<int> OneDimension;
typedef std::vector<OneDimension> TwoDimensions;
typedef std::vector<TwoDimensions> ThreeDimensions;

typedef std::vector<bool> OneDimensionBool;
typedef std::vector<OneDimensionBool> TwoDimensionsBool;
typedef std::vector<TwoDimensionsBool> ThreeDimensionsBool;

typedef std::vector<cell> OneDimensionCell;
typedef std::vector<OneDimensionCell> TwoDimensionsCell;
typedef std::vector<TwoDimensionsCell> ThreeDimensionsCell;


struct rect {
	std::string pin_name;
	float x1, x2, y1, y2;
};
struct pin_info {
	int metal_layer;
	int x, y;
	std::string connected_pin;
};
struct gate_info {
	int x, y;
	std::string orientation;
	std::vector<std::pair<std::string, pin_info>> pins_connections; //first is the pin, second is the connected wire to that pin
};
struct mystruct {
	std::unordered_map<std::string, gate_info> connected_gates;
	std::vector <std::pair<std::string, rect>> pins_sizes;
	bool i_o_pin = false;
	float size_x, size_y;
};

struct StringNames {
	str netname;
	str gatename;
	str pinname;
};

struct tracks {
	int first, second, third;
	char orientation;
	float pitch, offset, width, spacing, via_spacing;
	bool found_before = false, found_via = false;
};
struct EmptyStr {
	bool operator()(const std::string& s) {
		return (s == "");
	}
};
struct my_cmp {
	bool operator()(StringNames a, StringNames b) const {
		return (a.netname < b.netname);
	}
};

#endif