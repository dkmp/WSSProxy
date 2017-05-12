#include "WSPParser.h"
#include "Utils.h"

string WSPParser::responseChannel(int channel_id, const string& seq){
	char chn[10];
	sprintf(chn, "%d", channel_id);
	return "WSP/1.1 200 OK\r\nseq: " + seq + "\r\nchannel: " + chn + "\r\n\r\n";
}

string WSPParser::responseRtsp(const string& rtsp, const string& seq){
	return "WSP/1.1 200 OK\r\nseq: " + seq + "\r\n\r\n" + rtsp;
}

string WSPParser::responseVoid(const string& seq){
	return "WSP/1.1 200 OK\r\nseq: " + seq + "\r\n\r\n";
}

string WSPParser::parseCommond(const string& wsp){
	vector<string> lines;
	vector<string> sections;
	Utils::split(wsp, lines, "\r\n");
	Utils::split(lines[0], sections, " ");
	return sections[1];
}

int WSPParser::parseInit(const string& wsp, string& host, int& port, string& seq){
	vector<string> lines;
	vector<string> sections;
	vector<string> items;
	Utils::split(wsp, lines, "\r\n");
	if (lines[0].find("INIT") != -1){
		items.clear();
		Utils::split(lines[2], items, ":");
		host = Utils::trim(items[1]);

		items.clear();
		Utils::split(lines[3], items, ":");
		string p = Utils::trim(items[1]);
		port = atoi(p.c_str());

		items.clear();
		Utils::split(lines[4], items, ":");
		seq = Utils::trim(items[1]);
	}
	else{
		return -1;
	}
	return 0;
}

int WSPParser::parseWrap(const string& wsp,const string& host, string& seq,string& rtsp){
	vector<string> lines;
	vector<string> sections;
	vector<string> items;
	Utils::split(wsp, lines, "\r\n");
	if (lines[0].find("WRAP") != -1){
		items.clear();
		Utils::split(lines[2], items, ":");
		seq = Utils::trim(items[1]);

		rtsp = "";
		for (int i = 3; i < lines.size(); i++){
			if (lines[i].find("DESCRIBE") != -1){
				vector<string> tmps;
				Utils::split(lines[i], tmps, " ");
				if (tmps[1].find("rtsp://") != -1){
					rtsp += lines[i];
				}
				else{
					rtsp += tmps[0] + " rtsp://" + host + tmps[1] + " " + tmps[2];
				}
			}
			else{
				rtsp += lines[i];
			}

			rtsp += "\r\n";
		}
		rtsp += "\r\n";
	}
	else{
		return -1;
	}
	return 0;
}

int WSPParser::parseJoin(const string& wsp, int& channel, string& seq){
	vector<string> lines;
	vector<string> sections;
	vector<string> items;
	Utils::split(wsp, lines, "\r\n");
	if (lines[0].find("JOIN") != -1){
		items.clear();
		Utils::split(lines[1], items, ":");
		channel = atoi(Utils::trim(items[1]).c_str());

		items.clear();
		Utils::split(lines[2], items, ":");
		seq = Utils::trim(items[1]);
	}
	else{
		return -1;
	}
	return 0;
}
