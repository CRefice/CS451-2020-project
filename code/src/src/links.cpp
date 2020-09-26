#include "links.hpp"

enum MessageType : char {
  Sent = 0,
  Received = 1,
};

static std::uint16_t local_port(Parser& parser) {
  for (auto& host : parser.hosts()) {
    if (host.id == parser.id()) {
      return host.port;
    }
  }
  throw std::runtime_error("no host with the given id found in the hosts file");
}

static constexpr std::size_t MESSAGE_SIZE =
    sizeof(MessageType) + sizeof(int) +
    sizeof(unsigned long); // Avoid using sizeof(Message) due to padding bytes

static void serialize(const MessageLink::Message& msg, MessageType type,
                      char buf[MESSAGE_SIZE]) {
  std::memcpy(buf, &type, sizeof(type));
  std::memcpy(buf + sizeof(type), &msg.sender, sizeof(msg.sender));
  std::memcpy(buf + sizeof(type) + sizeof(msg.sender), &msg.sequence_num,
              sizeof(msg.sequence_num));
}

static MessageType deserialize(const char buf[MESSAGE_SIZE],
                               MessageLink::Message& msg) {
  std::memcpy(&msg.sender, buf + sizeof(MessageType), sizeof(msg.sender));
  std::memcpy(&msg.sequence_num, buf + sizeof(MessageType) + sizeof(msg.sender),
              sizeof(msg.sequence_num));
  return MessageType(buf[0]);
}

MessageLink::MessageLink(Parser& parser)
    : process_id(parser.id()), socket(local_port(parser)) {
  host_addrs.resize(parser.hosts().size());
  for (auto& host : parser.hosts()) {
    address_of(host.id) = udp::socket_address(host.ip, host.port);
  }
}

void MessageLink::send(unsigned long receiver_id, const Message& msg) {
  char buffer[MESSAGE_SIZE];
  serialize(msg, Sent, buffer);
  socket.send(address_of(receiver_id), buffer, MESSAGE_SIZE);
}

void MessageLink::send_to(unsigned long receiver_id, int sequence_num) {
  send(receiver_id, Message{process_id, sequence_num});
  sent_msgs.emplace(sequence_num, receiver_id);
}

std::optional<MessageLink::Message> MessageLink::try_recv() {
  char buffer[MESSAGE_SIZE];
  if (socket.try_recv(buffer, MESSAGE_SIZE).has_value()) {
    Message msg;
    if (deserialize(buffer, msg) == Sent) {
      // New message incoming. Send confirmation
      buffer[0] = Received;
      socket.send(address_of(msg.sender), buffer, MESSAGE_SIZE);
      return msg;
    } else {
      // Receipt confirmation
      sent_msgs.erase(msg.sequence_num);
      return std::nullopt;
    }
  }
  return std::nullopt;
}

void MessageLink::send_heartbeats() {
  for (const auto& [sent_num, receiver] : sent_msgs) {
    send(receiver, Message{process_id, sent_num});
  }
}
