#ifndef _PROXYSERVER_H_
#define _PROXYSERVER_H_

#include <string>
#include <map>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "F:/SVN_local/libuv-1.x/include/uv.h"
#include "TCPClient.h"

using namespace std;
class ProxyServer;
class WSServer;

struct server_conf : public websocketpp::config::asio {

};

class WSServer :public websocketpp::server < server_conf > {
public:
	WSServer(ProxyServer* p){ proxy = p; };
public:
	ProxyServer* proxy;
};
typedef WSServer::message_ptr message_ptr;

class ProxyServer{
public:
	ProxyServer() :m_wsserver(this){};
	void start();
	void stop();
private:
	static void start_thread(void*); 
	static void delete_client_thread(void*);
	static bool validate(WSServer *s, websocketpp::connection_hdl hdl);
	static void on_close(WSServer* s, websocketpp::connection_hdl hdl);
	static void on_message(WSServer* s, websocketpp::connection_hdl hdl, message_ptr msg);
	static void on_socket_connect_cb();

	int genChannelId(){ return m_channel_id++; }
public:
	map<int, TCPClient*> m_controls;
	map<int, TCPClient*> m_datas;
private:
	WSServer m_wsserver;
	int m_channel_id;
};
#endif