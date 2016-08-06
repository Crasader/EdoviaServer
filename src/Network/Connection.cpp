#include "Connection.h"

#include <alloca.h>
#include <cassert>

#include "Log/Logger.h"
#include "Network.h"
#include "OpcodeHandler.h"

Connection::Connection(uv_loop_t* uv_loop, Network* network) : mNetwork(network)
{
    uv_client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

    uv_tcp_init(uv_loop, uv_client);

    mPacketBuffer.reserve(512);

    uv_async_write_notification = (uv_async_t*)malloc(sizeof(uv_async_t));
    memset(uv_async_write_notification, 0, sizeof(uv_async_t));
    uv_async_init(uv_loop, uv_async_write_notification,
                  [](uv_async_t* handle)
                  {
                      reinterpret_cast<Connection*>(handle->data)->sendQueuedPackets();
                  }
    );
    uv_async_write_notification->data = this;

    mPlayerSession = new PlayerSession(this);
    // TODO after login
    mPlayerSession->enterGame();
}

Connection::~Connection()
{
    delete mPlayerSession;
    mPlayerSession = nullptr;

    if (uv_is_closing((uv_handle_t*)uv_async_write_notification) == 0)
    {
        uv_close((uv_handle_t *)uv_async_write_notification, [](uv_handle_t *handle) -> void
        {
            free(handle);
        });
    }

    if (uv_is_closing((uv_handle_t*)uv_client) == 0)
    {
        uv_close((uv_handle_t*)uv_client,
                 [](uv_handle_t* handle) ->
                         void
                 {
                     free(handle);
                 }
        );
    }
}

void Connection::disconnect()
{
    getNetwork()->destroyConnection(this);
}

void Connection::readCallback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread == 0)
    {
        // need to read again later
        return;
    }
    else if (nread == UV_EOF)
    {
        disconnect();
    }
    else if (nread > 0)
    {
        readPacket(buf->base, nread);

        if (buf->len > 0)
            getNetwork()->deallocateBuffer(buf);
    }
    else // error
    {
        if (buf->len > 0)
            getNetwork()->deallocateBuffer(buf);
    }
}

void Connection::readPacket(const char* buffer, ssize_t size)
{
    // the current packet size. If 0, it's a new packet
    if (mCurrentPacketSize == 0)
    {
        if (size >= 2) // received packet length
        {
            mCurrentPacketSize = (((uint16_t)buffer[0]) << 8) | buffer[1];
            mCurrentPacketSize = ntohs(mCurrentPacketSize);

            // if we received more than just the packet length
            if (size > 2)
            {
                readPacketBody(buffer+2, size-2);
            }
        }
        else if (size == 1) // only received half of packet length
        {
            mCurrentPacketSize = (((uint16_t)buffer[0]) << 8);

            // wait until it's completly received
            mCurrentPacketSizeReadHalf = true;
        }
    }
    else if (mCurrentPacketSizeReadHalf == true) // need to read the rest of the length, because only one byte of length received
    {
        mCurrentPacketSizeReadHalf = false;
        mCurrentPacketSize = mCurrentPacketSize | buffer[0];
        mCurrentPacketSize = ntohs(mCurrentPacketSize);

        // if we received more than just the packet length
        if (size > 1)
        {
            readPacketBody(buffer+1, size-1);
        }
    }
    else
    {
        readPacketBody(buffer, size);
    }
}

void Connection::readPacketBody(const char* buffer, ssize_t size)
{
    for (int i = 0; i < size; i++)
    {
        // if our packet is still not complete
        if (mCurrentPacketBytesRead < mCurrentPacketSize)
        {
            mPacketBuffer.push_back(buffer[i]);
            mCurrentPacketBytesRead++;

            // we have completly read a packet
            if (mCurrentPacketBytesRead == mCurrentPacketSize)
            {
                // process it
                processPacket();

                // reset everything, to make it ready for a new one
                mPacketBuffer.clear();

                mCurrentPacketBytesRead = 0;
                mCurrentPacketSize = 0;

                // if we have completly read the packet, but there is more in the buffer
                if (size > i)
                {
                    // read the next "i"
                    readPacket(buffer+i+1, size-i-1);
                }
                // we're done here, return
                return;
            }
        }
        else
        {
            // should never happen, as if a packet is read completly, it parses the next ones
            assert(false);
        }
    }

    // gets here, when the packet is not parsed completly
}

void Connection::processPacket()
{
    // opcode 2 bytes big, body must be a minimum of one byte
    if (mPacketBuffer.size() <= 2+1)
    {
        // TODO: Add IP
        log->error("Connection: Malformed packet size {}", mPacketBuffer.size());

        //disconnect();
        return;
    }

    uint16_t opcode = ntohs((((uint16_t)mPacketBuffer[0]) << 8) | mPacketBuffer[1]);
    if (opcode > (int)OpcodeHandler::Opcodes::NUM)
    {
        log->error("Connection: Invalid Opcode {}", opcode);

        //disconnect();
        return;
    }

    //log->info("Packet received!");

    // Fill packet structure
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(opcode, mPacketBuffer.begin()+2, mPacketBuffer.end());

    opcodeHandler.processPacket(mPlayerSession, packet);
}

void Connection::sendPacket(std::shared_ptr<Packet> packet)
{
    uint16_t bodylength = packet->getSize()+2; // length = content + opcode 2 bytes
    uint16_t opcode = packet->getOpcode();

    assert(bodylength < 102400); // smaller than 100 kilobytes, because allocating on stack

    char* buffer;
    buffer = (char*)alloca(bodylength+2); // body + 2 bytes of header, allocated on stack so no free needed

    memcpy(buffer, &bodylength, 2); // copy in the length of the packet
    memcpy(buffer+2, &opcode, 2); // copy the opcode number in
    memcpy(buffer+2+2, packet->getRawData(), packet->getSize()); // copy the whole packet body

    uv_write_t* write_request = (uv_write_t*)malloc(sizeof(uv_write_t));

    uv_buf_t uv_buffer = uv_buf_init(buffer, bodylength+2);

    uv_write(write_request, (uv_stream_t*)uv_client, &uv_buffer, 1, [](uv_write_t* req, int status) {
        free(req);

        if (status != 0)
            log->error("Failed writing packet");
    });

}

void Connection::sendQueuedPackets()
{
    std::shared_ptr<Packet> packet;

    while (mPacketWriteQueue.try_dequeue(packet))
    {
        sendPacket(packet);
    }
}
