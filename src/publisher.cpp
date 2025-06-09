#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "config.h"
#include "rabbitmqproducer.h"
#include "randomInt.h"

using namespace std::string_literals;

using json = nlohmann::json;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Exchange name missing");
  }
  std::cout << "PUBLISH\n";
  auto connConfig = RabbitMQCpp::loadConnectionConfiguration("config/config.json");

  RabbitMQCpp::PublisherConfiguration producerConfig(argv[1]);

  RabbitMQCpp::RabbitMQPublisher publisher;
  publisher.login(connConfig);

  publisher.prepare(producerConfig);
  RabbitMQCpp::RandomInt ri(0, 16383);

  publisher.send(producerConfig, std::format("{} {}", "pubsub message"s, ri()));
  return 0;
}
