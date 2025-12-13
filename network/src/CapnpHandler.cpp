/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** CapnpHandler.cpp
*/

#include "../include/CapnpHandler.hpp"
#include "../include/GameMessage.capnp.h"

std::vector<std::uint8_t> CapnpHandler::serialize(const std::string &data) const
{
  capnp::MallocMessageBuilder message;
  auto netMsg = message.initRoot<NetworkMessage>();
  netMsg.setMessageType(data);

  kj::VectorOutputStream output;
  capnp::writeMessage(output, message);

  auto arr = output.getArray();
  return std::vector<std::uint8_t>(arr.begin(), arr.end());
}

std::optional<std::string> CapnpHandler::deserialize(const std::array<char, BUFFER_SIZE> &buffer,
                                                     std::size_t bytesTransferred) const
{
  if (bytesTransferred == 0) {
    return std::nullopt;
  }
  kj::ArrayPtr<const kj::byte> bytes(reinterpret_cast<const kj::byte *>(buffer.data()), bytesTransferred);
  kj::ArrayInputStream stream(bytes);

  try {
    capnp::InputStreamMessageReader reader(stream);
    auto netMsg = reader.getRoot<NetworkMessage>();
    auto type = netMsg.getMessageType();
    return std::string(type.cStr(), type.size());
  } catch (const kj::Exception &e) {
    return std::nullopt;
  }
}

std::vector<std::byte> CapnpHandler::stringToBytes(const std::string &str)
{
  const std::byte *bytePtr = reinterpret_cast<const std::byte *>(str.data());
  return std::vector<std::byte>(bytePtr, bytePtr + str.size());
}
