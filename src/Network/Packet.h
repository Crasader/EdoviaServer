#pragma once

#include "ByteBuffer.h"

/**
 * @brief A packet containg data via ByteBuffer
 *        and an opcode, it's type.
 */
class Packet : public ByteBuffer
{
    /// Opcode of the packet, basically the packet type
    uint16_t mOpcode;

public:
    /**
     * Constructs a packet
     * @param opcode the opcode number of the packet
     * @return
     */
    Packet(uint16_t opcode) : mOpcode(opcode)
    {

    }

    /**
     * Constructs a packet, and moves a std vector as bytes in
     * @param opcode opcode number of the packet
     * @param bytes the vector which will be moved
     */
    Packet(uint16_t opcode, std::vector<uint8_t> bytes) : ByteBuffer(bytes), mOpcode(opcode)
    {

    }

    /**
     * Constructs a packet, and moves a range of a std::vector
     * @param opcode opcode number of the packet
     * @param begin iterator of the first element to move
     * @param end iterator of the last element to move
     */
    Packet(uint16_t opcode, std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end) : ByteBuffer(begin, end), mOpcode(opcode)
    {

    }

    /**
     * @returns the opcode (packet type) if this packet
     */
    inline uint16_t getOpcode()
    {
        return mOpcode;
    }
};
