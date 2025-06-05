#include <unistd.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "config.h"
#include "rabbitmqproducer.h"

using json = nlohmann::json;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    throw std::runtime_error("Exchange name and routing key missing");
  }
  std::cout << "TOPIC PUBLISH\n";
  auto connConfig = RabbitMQCpp::loadConnectionConfiguration("config/config.json");

  RabbitMQCpp::TopicProducerConfiguration producerConfig(argv[1]);

  RabbitMQCpp::RabbitMQTopicProducer producer;
  producer.login(connConfig);

  producer.prepare(producerConfig);
  producer.send(producerConfig, argv[2], "test topic prod c++");
  return 0;
}
