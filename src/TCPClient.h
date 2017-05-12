#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include "F:/SVN_local/libuv-1.x/include/uv.h"
#include <stdlib.h>
#include <string>
#include <queue>

#include "Responser.h"

using namespace std;

#define CONTAINER_OF(ptr, type, field)                                        \
  ((type *) ((char *) (ptr) - ((char *) &((type *) 0)->field)))

#define TCPCLIENT_BUF_SIZE 32*1024
class TCPClient;

typedef struct{
	TCPClient* client;
	uv_connect_t connect;
	union {
		uv_handle_t handle;
		uv_stream_t stream;
		uv_tcp_t tcp;
	}handle;
	Responser* responser;
	uv_async_t write_handle;
	char buf[TCPCLIENT_BUF_SIZE];
}conn;


class TCPClient{
public:
	TCPClient();
	virtual ~TCPClient();
	void init(const char* ip, int port, Responser* responser);
	Responser* getResponser();
	void write(string str);
	void close();
	string getHost(){
		return m_ip;
	};

private:
	static void start_thread(void*);
	static void connect_done_cb(uv_connect_t *req, int status);
	static void connect_close_cb(uv_handle_t *handle);
	static void connect_alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf);
	static void read_done_cb(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
	static void write_done_cb(uv_write_t* req, int status);
	static void write_async_cb(uv_async_t* handle);
	static void timer_tick(uv_timer_t*);

private:
	bool m_binit;
	bool m_bconnect;
	uv_loop_t* m_loop;
	conn m_connect;
	string m_ip;
	int m_port;
	uv_write_t m_write;
	queue<string> msgs;
	uv_mutex_t m_loop_lock;
	uv_mutex_t m_close_lock;
};

#endif