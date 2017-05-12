#include "WSResponser.h"
#include "ProxyServer.h"
#include "WSPParser.h"
#include "RTSPParser.h"
#include "elog.h"

WSResponser::WSResponser(WSServer* s, int channel_id){
	m_wsserver = s;
	m_channel_id = channel_id;
	m_buf_len = 0;
	m_session = "";
	m_setup = false;
}

int WSResponser::on_connect(){
	WaitResponseItem waitRspItm = m_waitRspItms.front();
	m_waitRspItms.pop();

	string wsp = WSPParser::responseChannel(m_channel_id, waitRspItm.seq);

	try{
		m_wsserver->send(m_control_hdl, wsp, waitRspItm.opcode);
	}
	catch (...){
		log_info("websocket maybe close.");
		return -1;
	}
}

void WSResponser::on_disconnect(){

};

int WSResponser::on_receive(const char* buf, ssize_t size){
	if (size < 0){
		log_error("receive buf size is %d",size);
		return -1;
	}

	if (size + m_buf_len> WSRESPONSER_BUF_SIZE){
		log_error("remain size:%d,receive buf size:%d,too large.", m_buf_len, size);
		size = WSRESPONSER_BUF_SIZE;
	}

	memset(m_buf + m_buf_len, 0, WSRESPONSER_BUF_SIZE - m_buf_len);
	memcpy(m_buf+m_buf_len, buf, size);
	m_buf_len += size;

	while (m_buf_len>0){
		if (m_buf[0] == '$'){
			char interleaved = m_buf[1];
			int len = ((m_buf[2] << 8) & 0xff00) + (0xff&m_buf[3]);
			if (m_buf_len < len + 4)
			{
				return 0;
			}
			try{
				m_wsserver->send(m_data_hdl, m_buf, len + 4, websocketpp::frame::opcode::value::binary);
			}
			catch (...){
				log_info("websocket maybe close.");
				return -1;
			}
			m_buf_len -= len + 4;
			memcpy(m_buf, m_buf + len + 4, m_buf_len);
		}
		else if (m_buf_len > 3 && m_buf[0] == 'R'&&m_buf[1] == 'T'&&m_buf[2] == 'S'&&m_buf[3] == 'P'){
			int pos = RTSPParser::find_end_pos(m_buf, m_buf_len);
			if (pos == -1){//unfinish
				return 0;
			}
			WaitResponseItem waitRspItm = m_waitRspItms.front();
			m_waitRspItms.pop();
			string rtsp(m_buf, 0, pos);
			if (isSetup()&&m_session.empty()){
				m_session = RTSPParser::parseSession(rtsp);
			}
			string wsp = WSPParser::responseRtsp(rtsp, waitRspItm.seq);
			try{
				m_wsserver->send(m_control_hdl, wsp, waitRspItm.opcode);
			}
			catch (...){
				log_info("websocket maybe close.");
				return -1;
			}
			m_buf_len -= pos;
			memcpy(m_buf, m_buf + pos, m_buf_len);
		}
		else{
			log_error("receive unkown packet.");
			return -1;
		}
	}
}

void WSResponser::pushToWaitResponseSq(WaitResponseItem& waitRspItm){
	m_waitRspItms.push(waitRspItm);
}