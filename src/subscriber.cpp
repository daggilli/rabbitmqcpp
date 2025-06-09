#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

#include "config.h"
#include "rabbitmqconsumer.h"

using json = nlohmann::json;

struct CallableCallback {
  CallableCallback() { std::cout << "G CTOR\n"; }
  CallableCallback(const CallableCallback &g) { std::cout << "G COPY CTOR\n"; }
  CallableCallback(CallableCallback &&g) { std::cout << "G MOVE CTOR\n"; }

  void operator()(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg) {
    std::cout << "CALLABLE " << channelId << " "
              << std::string(static_cast<char *>(msg.body.bytes), msg.body.len) << "\n";
  }
};

RabbitMQCpp::ConnectionConfiguration loadConnectionConfiguration(const std::string &configPath);
void message(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg);

int main(const int argc, char *const argv[]) {
  if (argc == 1) {
    throw std::runtime_error("Exchange name missing");
  }
  std::cout << "SUBSCRIBE\n";
  auto connConfig = RabbitMQCpp::loadConnectionConfiguration("config/config.json");

  RabbitMQCpp::SubscriberConfiguration subscriberConfig{argv[1]};
  RabbitMQCpp::RabbitMQSubscriber subscriber;
  subscriber.login(connConfig);

  CallableCallback ccb;
  RabbitMQCpp::ConsumerCallback cb = message;
  subscriber.prepare(subscriberConfig, std::ref(cb));

  while (true) {
    subscriber.consume();
  }
  return 0;
}

void message(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg) {
  std::cout << "MESSAGE +++ " << std::string(static_cast<char *>(msg.body.bytes), msg.body.len) << "\n";
}
