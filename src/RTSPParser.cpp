#include "RTSPParser.h"
#include "Utils.h"

int RTSPParser::find_end_pos(const char* data, int len){
	int rn = 1;
	int contentLength = 0;
	bool bfind = false;
	int i;
	for (i = 13; i < len; i++){
		if (!bfind
			&&data[i - 13] == 'C'
			&&data[i - 12] == 'o'
			&&data[i - 11] == 'n'
			&&data[i - 10] == 't'
			&&data[i - 9] == 'e'
			&&data[i - 8] == 'n'
			&&data[i - 7] == 't'
			&&data[i - 6] == '-'
			&&data[i - 5] == 'L'
			&&data[i - 4] == 'e'
			&&data[i - 3] == 'n'
			&&data[i - 2] == 'g'
			&&data[i-1] == 't'
			&&data[i] == 'h'){
			rn = 2;
			int num;
			for (int j = i+2; j < len; j++){
				if (data[j - 1] == '\r'&&data[j] == '\n'){
					if (sscanf(&data[i + 2], "%u", &num) == 1) {
						contentLength = num;
						contentLength += j - i;
						bfind = true;
						break;
					}
				}
			}
		}
		if (data[i - 3] == '\r'&&data[i - 2] == '\n'&&data[i - 1] == '\r'&&data[i] == '\n'){
			rn--;
			if (rn == 0){
				return i+1;
			}
		}
		if (bfind){
			if (contentLength == 0){
				return i+3;
			}
			contentLength--;
		}
	}
	return -1;
}

string RTSPParser::parseSession(const string& rtsp)
{
	vector<string> lines;
	vector<string> sections;
	vector<string> items;
	Utils::split(rtsp, lines, "\r\n");
	for (int i = 0; i < lines.size(); i++){
		sections.clear();
		Utils::split(lines[i], sections, ":");
		if (sections[0] == "Session"){
			Utils::split(sections[1], items, ";");
			return items[0];
		}
	}
	return "";
}

string RTSPParser::parseCommond(const string& rtsp){
	vector<string> lines;
	vector<string> sections;
	Utils::split(rtsp, lines, "\r\n");
	Utils::split(lines[0], sections, " ");
	return sections[0];
}

void RTSPParser::setSession(string& rtsp, const string& session){
	vector<string> lines;
	vector<string> sections;
	Utils::split(rtsp, lines, "\r\n");
	rtsp = "";
	bool bfind = false;
	for (int i = 0; i < lines.size(); i++){
		if (lines[i].find("Session") == 0){
			bfind = true;
			rtsp += "Session: " + session;
			rtsp += "\r\n";
		}
		else{
			rtsp += lines[i];
			rtsp += "\r\n";
		}
	}
	if (!bfind){
		rtsp += "Session: " + session;
		rtsp += "\r\n";
	}
	rtsp += "\r\n";
}