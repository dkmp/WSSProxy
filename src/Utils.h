#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>

using namespace std;

class Utils{
public:
	static string trim(const string& str);
	static int split(const string& str, vector<string>& ret_, string sep = ",");
};
#endif