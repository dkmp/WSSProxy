#ifndef	_PARSER_H_
#define _PARSER_H_

#include "F:/SVN_local/libuv-1.x/include/uv.h"

class Responser{
public:
	Responser(){};
	virtual ~Responser(){};
	virtual int on_connect(){ return 0; };
	virtual void on_disconnect(){};
	virtual int on_receive(const char*, ssize_t){ return 0; };
};

#endif