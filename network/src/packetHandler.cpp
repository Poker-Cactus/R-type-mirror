/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/packetHandler.hpp"

std::vector<uint8_t> PacketHandler::serialize(const std::string &data)
{
    capnp::MallocMessageBuilder message;
    auto netMsg = message.initRoot<NetworkMessage>();
    netMsg.setMessageType(data);

    kj::VectorOutputStream output;
    capnp::writeMessage(output, message);

    auto arr = output.getArray();
    return std::vector<uint8_t>(arr.begin(), arr.end());
}

std::string PacketHandler::deserialize(const std::array<char, 1024> &recvBuffer, const std::size_t bytesTransferred)
{
    if (bytesTransferred == 0) {
        throw std::runtime_error("Empty buffer received");
    }
    kj::ArrayPtr<const kj::byte> bytes(reinterpret_cast<const kj::byte *>(recvBuffer.data()), bytesTransferred);
    kj::ArrayInputStream stream(bytes);

    try {
        capnp::InputStreamMessageReader reader(stream);
        auto netMsg = reader.getRoot<NetworkMessage>();
        auto type = netMsg.getMessageType();
        return std::string(type.cStr(), type.size());
    } catch (const kj::Exception &e) {
        throw std::runtime_error(std::string("Cap'n Proto error: ") + e.getDescription().cStr());
    }
}
