#include "ProxyServer.h"
#include "TCPClient.h"
#include "WSPParser.h"
#include "WSResponser.h"
#include "elog.h"
#include "RTSPParser.h"
#include "eco/thread.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

bool ProxyServer::validate(WSServer *s, websocketpp::connection_hdl hdl) {
	WSServer::connection_ptr con = s->get_con_from_hdl(hdl);
	
	const std::vector<std::string> & subp_requests = con->get_requested_subprotocols();
	
	if (subp_requests.size() > 0) {
		con->select_subprotocol(subp_requests[0]);
	}

	return true;
}

void ProxyServer::delete_client_thread(void* arg)
{
	TCPClient* client = (TCPClient*)arg;
	delete client;
	client = NULL;
}

void ProxyServer::on_close(WSServer* s, websocketpp::connection_hdl hdl) {
	int sockethd = s->get_con_from_hdl(hdl)->get_socket().native_handle();
	map<int, TCPClient*>::iterator controlIter = s->proxy->m_controls.find(sockethd);
	if (controlIter != s->proxy->m_controls.end()){
		TCPClient* client = controlIter->second;

		WSResponser* responser = (WSResponser*)client->getResponser();
		int channelId = responser->getChannelId();
		s->proxy->m_controls.erase(controlIter);

		map<int, TCPClient*>::iterator dataIter = s->proxy->m_datas.find(channelId);
		s->proxy->m_datas.erase(dataIter);


		uv_thread_t handle;
		uv_thread_create(&handle, ProxyServer::delete_client_thread, client);
	}
}

void ProxyServer::on_message(WSServer* s, websocketpp::connection_hdl hdl, message_ptr msg) {
	string wsp = msg->get_payload();
	int sockethd = s->get_con_from_hdl(hdl)->get_socket().native_handle();
	string command = WSPParser::parseCommond(wsp);
	if (command == "INIT"){
		string host;
		int port;
		string seq;
		WSPParser::parseInit(wsp,host,port,seq);
		TCPClient* client = new TCPClient();
		s->proxy->m_controls[sockethd] = client;
		int channel = s->proxy->genChannelId();
		s->proxy->m_datas[channel] = client;

		WSResponser* responser = new WSResponser(s,channel);
		responser->setControlHandle(hdl);

		WaitResponseItem waitRspItm;
		waitRspItm.opcode = msg->get_opcode();
		waitRspItm.seq = seq;
		responser->pushToWaitResponseSq(waitRspItm);

		client->init(host.c_str(), port, responser);
	}
	else if (command == "WRAP"){
		TCPClient* client = s->proxy->m_controls[sockethd];
		string seq;
		string rtsp;
		WSPParser::parseWrap(wsp, client->getHost(), seq, rtsp);
		WSResponser* responser = (WSResponser*)client->getResponser();

		WaitResponseItem waitRspItm;
		waitRspItm.opcode = msg->get_opcode();
		waitRspItm.seq = seq;
		if (!responser->isSetup()){
			string command = RTSPParser::parseCommond(rtsp);
			if (command == "SETUP"){
				responser->setup();
			}
		}
		else{
			while (responser->getSession().empty()){
				ECO_SLEEP_MS(100);
			}
			RTSPParser::setSession(rtsp, responser->getSession());
		}

			string session = RTSPParser::parseSession(rtsp);
			if (!session.empty()){
				RTSPParser::setSession(rtsp, session);
			}

		responser->pushToWaitResponseSq(waitRspItm);
		client->write(rtsp);
	}
	else if (command == "JOIN"){
		int channel;
		string seq;
		WSPParser::parseJoin(wsp, channel,seq);
		TCPClient* client = s->proxy->m_datas[channel];
		WSResponser* parser = (WSResponser*)client->getResponser();
		parser->setDataHandle(hdl);
		s->send(hdl, WSPParser::responseVoid(seq), msg->get_opcode());
	}
}

void ProxyServer::start_thread(void* arg)
{
	ProxyServer* self = (ProxyServer*)arg; 
	self->m_wsserver.set_access_channels(websocketpp::log::alevel::none);

	self->m_wsserver.init_asio();
	self->m_wsserver.set_reuse_addr(true);

	self->m_wsserver.set_message_handler(bind(&on_message, &self->m_wsserver, ::_1, ::_2));
	self->m_wsserver.set_close_handler(bind(&on_close, &self->m_wsserver, ::_1));
	self->m_wsserver.set_validate_handler(bind(&validate, &self->m_wsserver, ::_1));

	self->m_wsserver.listen(9002);

	self->m_wsserver.start_accept();
	self->m_wsserver.run();
}

void ProxyServer::start(){
	uv_thread_t handle;
	uv_thread_create(&handle, ProxyServer::start_thread, this);
}

void ProxyServer::stop(){
	m_wsserver.stop();
}
