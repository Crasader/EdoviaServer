#include "OpcodeHandler.h"

OpcodeHandler opcodeHandler;

OpcodeHandler::OpcodeHandler()
{
    initializeOpcodes();
}

void OpcodeHandler::initializeOpcodes()
{
    memset(callbacks, 0, sizeof(callbacks));

    callbacks[(int)Opcodes::CS_MOVEPACKET] = {&PlayerSession::handleMovementPacket};
}