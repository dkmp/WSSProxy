#include "TCPClient.h"
#include "eco/thread.h"
#include "elog.h"

TCPClient::TCPClient() :m_bconnect(false), m_binit(false)
{
	uv_mutex_init(&m_loop_lock);
	uv_mutex_init(&m_close_lock);
};

TCPClient::~TCPClient(){
	close();
	uv_mutex_destroy(&m_loop_lock);
	uv_mutex_destroy(&m_close_lock);
}

void TCPClient::connect_alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
	conn *c;
	c = CONTAINER_OF(handle, conn, handle);

	buf->base = c->buf;

	if (buf->len > TCPCLIENT_BUF_SIZE){
		log_error("receive buf size: %u,too large", buf->len);
		buf->len = TCPCLIENT_BUF_SIZE;
	}
	else{
		buf->len = sizeof(c->buf);
	}
	memset(c->buf, 0, TCPCLIENT_BUF_SIZE);
}

void TCPClient::timer_tick(uv_timer_t* timer){

}

void TCPClient::start_thread(void* arg)
{
	TCPClient* self = (TCPClient*)arg;
	self->m_loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
	uv_timer_t timer;

	uv_loop_init(self->m_loop);

	timer.loop = self->m_loop;
	uv_timer_init(self->m_loop, &timer);
	uv_timer_start(&timer, timer_tick, 1000, 1);

	uv_async_init(self->m_loop, &self->m_connect.write_handle, write_async_cb);

	struct sockaddr_in sa;
	uv_ip4_addr(self->m_ip.c_str(), self->m_port, &sa);
	uv_tcp_init(self->m_loop, &self->m_connect.handle.tcp);
	self->m_connect.client = self;
	uv_tcp_connect(&self->m_connect.connect,
		&self->m_connect.handle.tcp,
		(const struct sockaddr *)&sa,
		connect_done_cb);
	
	uv_run(self->m_loop, UV_RUN_DEFAULT);

	uv_mutex_lock(&self->m_loop_lock);
	free(self->m_loop);
	self->m_loop = NULL;
	uv_mutex_unlock(&self->m_loop_lock);
	if (self->m_connect.responser != NULL){
		delete self->m_connect.responser;
		self->m_connect.responser = NULL;
	}
}

void TCPClient::connect_close_cb(uv_handle_t *handle) {
	conn *connect;
	connect = CONTAINER_OF(handle, conn, handle);
	if (connect->responser != NULL)
	{
		connect->responser->on_disconnect();
	}
}

void TCPClient::read_done_cb(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf){
	conn *connect = CONTAINER_OF(handle, conn, handle);
	if (nread < 0){
		if (!uv_is_closing(&connect->handle.handle))
		{
			uv_mutex_lock(&connect->client->m_close_lock);
			if (!uv_is_closing(&connect->handle.handle))
			{
				uv_close(&connect->handle.handle, connect_close_cb);
			}
			uv_mutex_lock(&connect->client->m_close_lock);
		}
		return;
	}
	int ret = connect->responser->on_receive(buf->base, nread);
	if (ret < 0){
		if (!uv_is_closing(&connect->handle.handle)){
			uv_mutex_lock(&connect->client->m_close_lock);
			if (!uv_is_closing(&connect->handle.handle))
			{
				uv_close(&connect->handle.handle, connect_close_cb);
			}
			uv_mutex_lock(&connect->client->m_close_lock);
		}
		return;
	}
}

void TCPClient::connect_done_cb(uv_connect_t *req, int status) {
	conn* connect = CONTAINER_OF(req, conn, connect);
	connect->client->m_bconnect = true;
	connect->handle.stream = *req->handle;
	uv_read_start((uv_stream_t*)&connect->client->m_connect.handle.tcp, connect_alloc_cb, read_done_cb);
	int ret = connect->responser->on_connect();
	if (ret < 0){
		if (!uv_is_closing(&connect->handle.handle))
		{
			uv_mutex_lock(&connect->client->m_close_lock);
			if (!uv_is_closing(&connect->handle.handle))
			{
				uv_close(&connect->handle.handle, connect_close_cb);
			}
			uv_mutex_unlock(&connect->client->m_close_lock);
		}
	}
}

void TCPClient::init(const char* ip, int port, Responser* responser){
	if (m_binit){
		return;
	}
	m_binit = true;
	m_ip = ip;
	m_port = port;
	uv_thread_t handle;
	uv_thread_create(&handle, TCPClient::start_thread, this);
	m_connect.responser = responser;
}

void TCPClient::write_done_cb(uv_write_t* req, int status) {
	int ss = 1;
	ss = 2;
}

void TCPClient::write_async_cb(uv_async_t* handle) {	
	conn *connect = CONTAINER_OF(handle, conn, write_handle);
	uv_buf_t buf;
	string str;
	str = connect->client->msgs.front();
	connect->client->msgs.pop();
	buf = uv_buf_init((char*)str.c_str(), str.size());
	int dd = uv_write(&connect->client->m_write, (uv_stream_t*)&connect->client->m_connect.handle.tcp, &buf, 1, write_done_cb);
	dd = 1;
}

void TCPClient::write(string str){
	msgs.push(str);
	uv_async_send(&m_connect.write_handle);
}

Responser* TCPClient::getResponser()
{ 
	return m_connect.responser;
};

void TCPClient::close(){
	if (!uv_is_closing(&m_connect.handle.handle))
	{
		uv_mutex_lock(&m_close_lock);
		if (!uv_is_closing(&m_connect.handle.handle))
		{
			uv_close(&m_connect.handle.handle, connect_close_cb);
		}
		uv_mutex_unlock(&m_close_lock);
	}
	if (m_loop != NULL){
		uv_mutex_lock(&m_loop_lock);
		if (m_loop != NULL)
		{
			uv_stop(m_loop);
		}
		uv_mutex_unlock(&m_loop_lock);
		while (m_loop != NULL){
			ECO_SLEEP_MS(1000);
		}
	}
}