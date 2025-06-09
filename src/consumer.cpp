#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "config.h"
#include "rabbitmqconsumer.h"

using json = nlohmann::json;
using namespace std::string_literals;

void cb(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &message);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Queue name missing");
  }
  auto connConfig = RabbitMQCpp::loadConnectionConfiguration("config/config.json");

  RabbitMQCpp::DirectConsumerConfiguration consumerConfig(argv[1]);

  RabbitMQCpp::RabbitMQDirectConsumer consumer;
  consumer.login(connConfig);
  consumer.prepare(consumerConfig, cb);

  while (true) {
    consumer.consume();
  }

  return 0;
}

void cb(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg) {
  std::cout << "CB\n" << std::string(static_cast<char *>(msg.body.bytes), msg.body.len) << "\n";
}