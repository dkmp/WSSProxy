#include "ProxyServer.h"
#include "eco/thread.h"
#include "elog.h"

int main(int argc, char** argv){
	log_reg(argc, argv);
	ProxyServer server;
	server.start();
	while (1){
		ECO_SLEEP_MS(10000);
	}
	return 0;
}