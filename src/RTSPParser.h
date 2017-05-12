#ifndef _RTSPPARSER_H_
#define _RTSPPARSER_H_

#include <string>
using namespace std;
class RTSPParser{
public:
	static int find_end_pos(const char* data,int len);
	static string parseSession(const string& rtsp);
	static string parseCommond(const string& rtsp);
	static void setSession(string& rtsp, const string& session);
};

#endif