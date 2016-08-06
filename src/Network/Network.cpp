#include "Network.h"

#include "Log/Logger.h"

Network::Network()
{
    uv_loop_init(&uv_loop);
}

Network::~Network()
{
    // Close all UV handles correctly
    // http://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly
    uv_stop(&uv_loop);

    // destroy all connection
    for (auto &&connection: mConnections)
    {
        delete connection;
    }
    mConnections.clear();

    uv_walk(&uv_loop,
            [] (uv_handle_t* handle, void* arg) -> void
            {
                if (uv_is_closing(handle) == 0)
                    uv_close(handle, NULL);
            },
            NULL);

    uv_run(&uv_loop, UV_RUN_DEFAULT);

    while (uv_loop_close(&uv_loop) == UV_EBUSY)
    {
        uv_run(&uv_loop, UV_RUN_DEFAULT);
    }
}

void Network::onNewConnection(uv_stream_t* server, int status)
{
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    Connection* connection = createConnection();

    uv_tcp_t* client = connection->getUVClient();
    client->data = connection;

    if (uv_accept(server, (uv_stream_t*) client) == 0)
    {
        uv_read_start((uv_stream_t*) client, Network::allocationCallback,
                [] (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                {
                    (reinterpret_cast<Connection*>(stream->data))->readCallback(stream, nread, buf);
                }
        );
    }
    else
    {
        destroyConnection(connection);
        connection = 0;
    }

}

void Network::listen(int port)
{
    log->info("Start listening on Network Port {}", port);

    uv_tcp_init(&uv_loop, &uv_server);

    // for stopping
    uv_async_init(&uv_loop, &uv_async,
                [](uv_async_t* handle)
                {
                    reinterpret_cast<Network*>(handle->data)->stop();
                }
    );
    uv_async.data = this;

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", port, &addr);

    uv_tcp_nodelay(&uv_server, 1);
    uv_tcp_bind(&uv_server, (const struct sockaddr*)&addr, 0);

    uv_server.data = this; // to call the member instead of the static function later

    int r = uv_listen((uv_stream_t*)&uv_server, 128,
                  [](uv_stream_t* server, int status)
                  {
                      reinterpret_cast<Network*>(server->data)->onNewConnection(server, status);
                  }
    );

    if (r) {
        fprintf(stdout, "Listen error %s\n", uv_strerror(r));
    }

    uv_run(&uv_loop, UV_RUN_DEFAULT);

    log->info("Stop listening on Network Port {}", port);

    if (uv_is_closing((uv_handle_t*)&uv_async) == 0)
        uv_close((uv_handle_t*)&uv_async, NULL);

    if (uv_is_closing((uv_handle_t*)&uv_server) == 0)
        uv_close((uv_handle_t*)&uv_server, NULL);
}
