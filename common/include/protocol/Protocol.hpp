/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Network Protocol Definitions
*/

#pragma once

#include <cstdint>

namespace Protocol {

    enum class PacketType : uint8_t {
        CONNECT = 0x01,
        DISCONNECT = 0x02,
        PLAYER_INPUT = 0x03,
        ENTITY_UPDATE = 0x04,
        SPAWN_ENTITY = 0x05,
        DESTROY_ENTITY = 0x06
    };

    struct PacketHeader {
        PacketType type;
        uint32_t size;
    };

}
