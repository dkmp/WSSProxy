#ifndef _WSPPARSER_H_
#define _WSPPARSER_H_

#include <string>
#include <vector>

using namespace std;

class WSPParser{
public:
	static int parseInit(const string& wsp, string& host, int& port, string& seq);
	static int parseWrap(const string& wsp, const string& host, string& seq, string& rtsp);
	static int parseJoin(const string& wsp, int& channel, string& seq);
	static string parseCommond(const string& wsp);
	static string responseChannel(int channel_id, const string& seq);
	static string responseRtsp(const string& rtsp, const string& seq);
	static string responseVoid(const string& seq);
};
#endif