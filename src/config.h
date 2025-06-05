#ifndef __LOAD_CONFIG_H__
#define __LOAD_CONFIG_H__
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace RabbitMQCpp {
  struct ConnectionConfiguration {
    std::string hostname;
    std::string username;
    std::string password;
    std::string vhost;
    int port;
  };

  class ConsumerConfiguration {
   public:
    ConsumerConfiguration() = delete;
    ConsumerConfiguration(const int chanId) : channelId(chanId) {}

    int channelId;

    virtual ~ConsumerConfiguration() = default;
  };

  class DirectConsumerConfiguration : public ConsumerConfiguration {
   public:
    DirectConsumerConfiguration(const std::string &q, const int chanId = 1)
        : ConsumerConfiguration(chanId), queue(q) {}
    std::string queue;
  };

  class SubscriberConfiguration : public ConsumerConfiguration {
   public:
    SubscriberConfiguration(const std::string &e, const int chanId = 1)
        : ConsumerConfiguration(chanId), exchange(e) {}
    std::string exchange;
  };

  class TopicConsumerConfiguration : public ConsumerConfiguration {
   public:
    TopicConsumerConfiguration(const std::string &e, const std::vector<std::string> &t, const int chanId = 1)
        : ConsumerConfiguration(chanId), exchange(e), topics(std::move(t)) {}
    std::string exchange;
    std::vector<std::string> topics;
  };

  class ProducerConfiguration {
   public:
    ProducerConfiguration() = delete;
    ProducerConfiguration(const int chanId) : channelId(chanId) {}

    int channelId;

    virtual ~ProducerConfiguration() = default;
  };

  class DirectProducerConfiguration : public ProducerConfiguration {
   public:
    DirectProducerConfiguration(const std::string &q, const int chanId = 1)
        : ProducerConfiguration(chanId), queue(q) {}
    std::string queue;
  };

  class PublisherConfiguration : public ProducerConfiguration {
   public:
    PublisherConfiguration(const std::string &e, const int chanId = 1)
        : ProducerConfiguration(chanId), exchange(e) {}
    std::string exchange;
  };

  class TopicProducerConfiguration : public ProducerConfiguration {
   public:
    TopicProducerConfiguration(const std::string &e, const int chanId = 1)
        : ProducerConfiguration(chanId), exchange(e) {}
    std::string exchange;
  };

  RabbitMQCpp::ConnectionConfiguration loadConnectionConfiguration(const std::string &configPath) {
    std::ifstream configfile(configPath);
    json config = json::parse(configfile);

    return RabbitMQCpp::ConnectionConfiguration{config["hostname"], config["username"], config["password"],
                                                config["vhost"], config["port"]};
  }
};  // namespace RabbitMQCpp
#endif