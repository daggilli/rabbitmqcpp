#include <unistd.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "config.h"
#include "rabbitmqproducer.h"
#include "randomInt.h"

using json = nlohmann::json;
using namespace std::string_literals;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Queue name missing");
  }
  auto connConfig = RabbitMQCpp::loadConnectionConfiguration("config/config.json");

  RabbitMQCpp::DirectProducerConfiguration producerConfig(argv[1]);

  RabbitMQCpp::RabbitMQDirectProducer producer;
  producer.login(connConfig);

  producer.prepare(producerConfig);
  RabbitMQCpp::RandomInt ri(0, (1 << 17) - 1);
  json j = {{"msg", "test producer message"s}, {"id", ri()}};

  producer.send(producerConfig, j.dump());

  sleep(1);
  return 0;
}
