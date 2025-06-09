#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "config.h"
#include "rabbitmqconsumer.h"

using namespace std::string_literals;

using json = nlohmann::json;

void message(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg);

int main(const int argc, char *const argv[]) {
  if (argc < 3) {
    throw std::runtime_error("Queue and topic names missing");
  }
  std::cout << "SUBSCRIBE\n";
  auto connConfig = RabbitMQCpp::loadConnectionConfiguration("config/config.json");

  std::vector<std::string> topics{argv + 2, argv + argc};
  for (auto &t : topics) {
    std::cout << "Topic: " << t << "\n";
  }

  RabbitMQCpp::TopicConsumerConfiguration consumerConfig{argv[1], std::move(topics)};

  RabbitMQCpp::RabbitMQTopicConsumer consumer;
  consumer.login(connConfig);

  const RabbitMQCpp::ConsumerCallback cb = message;
  consumer.prepare(consumerConfig, std::ref(cb));

  while (true) {
    consumer.consume();
  }
  return 0;
}

void message(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg) {
  std::cout << "MESSAGE +++ " << std::string(static_cast<char *>(msg.body.bytes), msg.body.len) << "\n";
}
