#ifndef _WSPARSER_H_
#define _WSPARSER_H_

#include <string>
#include "Responser.h"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp""

using namespace std;

class WSServer;

typedef struct{
	string seq;
	websocketpp::frame::opcode::value opcode;
}WaitResponseItem;


#define WSRESPONSER_BUF_SIZE 128*1024
class WSResponser :public Responser{
public:
	WSResponser(WSServer* s, int channel_id);
	virtual ~WSResponser(){};
	
	virtual int on_connect();
	virtual void on_disconnect();
	virtual int on_receive(const char*, ssize_t);

	void pushToWaitResponseSq(WaitResponseItem& waitRsp);
	void setControlHandle(websocketpp::connection_hdl hdl){ m_control_hdl = hdl; };
	void setDataHandle(websocketpp::connection_hdl hdl){ m_data_hdl = hdl; };
	int getChannelId(){ return m_channel_id; };
	bool isSetup(){ return m_setup; };
	void setup(){ m_setup = true; };
	string getSession(){
		return m_session;
	};

private:
	WSServer* m_wsserver;
	queue<WaitResponseItem> m_waitRspItms;
	websocketpp::connection_hdl m_control_hdl;
	websocketpp::connection_hdl m_data_hdl;
	int m_channel_id;
	char m_buf[WSRESPONSER_BUF_SIZE];
	int m_buf_len;
	string m_session;
	bool m_setup;
};

#endif