#include <signal.h>

#include "Log/Logger.h"
#include "Server/Server.h"

Server* server;

void signalHandler(int s){
    printf("Caught signal %d\n",s);

    server->stopServer();
}

void initializeSignalHandler()
{
#ifdef _WIN32
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer handler;
	handler = signal(SIGINT, signalHandler);
#else
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // ignore sigpipe
    signal(SIGPIPE, SIG_IGN);
#endif
}

int main() {

    Logger* logger = new Logger();

    log->info("Starting Server");

    initializeSignalHandler();

    server = new Server();

    server->run();

    delete server;

    log->flush();
    delete logger;

    return 0;
}