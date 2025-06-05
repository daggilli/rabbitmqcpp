#ifndef __RABBITMQPRODUCER_H__
#define __RABBITMQPRODUCER_H__

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

#include <concepts>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "config.h"
#include "sendconcept.h"

using namespace std::string_literals;

namespace RabbitMQCpp {
  template <typename Derived>
  class RabbitMQProducer {
   public:
    RabbitMQProducer() : connection(nullptr), socket(nullptr), channelOpen(false) {
      static_assert(HasVoidSendMember<Derived>,
                    "Derived class must have a send(...) member function that returns void");

      connection = amqp_new_connection();
      if (!connection) {
        throw std::runtime_error("create connection failed");
      }
      socket = amqp_tcp_socket_new(connection);
      if (!socket) {
        throw std::runtime_error(std::format("create TCP socket failed: {}", std::strerror(errno)));
      }
    }

    virtual ~RabbitMQProducer() {
      if (channelOpen) {
        amqp_channel_close(connection, 1, AMQP_REPLY_SUCCESS);
      }

      if (connection) {
        amqp_connection_close(connection, AMQP_REPLY_SUCCESS);
      }

      amqp_destroy_connection(connection);
    }

    void login(const ConnectionConfiguration &config) {
      if ((amqp_socket_open(socket, config.hostname.c_str(), config.port)) != 0) {
        throw std::runtime_error("socket open failed");
      }
      auto loginResult = amqp_login(connection, config.vhost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                                    config.username.c_str(), config.password.c_str());
      if (loginResult.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("Login failed");
      }
    }

    virtual void prepare(const ProducerConfiguration &config) = 0;

   protected:
    amqp_connection_state_t connection;
    amqp_socket_t *socket;
    bool channelOpen;

    void openChannel(const int channelId) {
      amqp_channel_open(connection, channelId);
      auto channelOpenResult = amqp_get_rpc_reply(connection);
      if (channelOpenResult.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("channel open failed");
      }
      channelOpen = true;
    }

    std ::string byteString(const amqp_bytes_t &bytes) {
      return std::string(static_cast<char *>(bytes.bytes), bytes.len);
    }

    void throwOnError(const std::string &msg, const bool verbose = false) {
      auto amqpResult = amqp_get_rpc_reply(connection);
      if (amqpResult.reply_type != AMQP_RESPONSE_NORMAL) {
        if (verbose) {
          auto replyType = amqpResult.reply_type;
          std::string reason;
          if (replyType == AMQP_RESPONSE_NONE) {
            reason = "no response from server"s;
          } else {
            reason = replyType == AMQP_RESPONSE_SERVER_EXCEPTION ? "server error" : "lib error";
          }
          auto err =
              std::format("{} [code: {}, reason: {}]", msg, static_cast<int>(amqpResult.reply_type), reason);
          throw std::runtime_error(err);
        }
        throw std::runtime_error(std::move(msg));
      }
    }
  };  // RabbitMQProducer

  class RabbitMQDirectProducer : public RabbitMQProducer<RabbitMQDirectProducer> {
   public:
    void prepare(const ProducerConfiguration &config) { openChannel(config.channelId); }
    void send(const ProducerConfiguration &config, const std::string &msg) {
      auto dyconfig = dynamic_cast<const DirectProducerConfiguration *>(&config);

      amqp_basic_publish(connection, 1, amqp_empty_bytes, amqp_cstring_bytes(dyconfig->queue.c_str()), 0, 0,
                         NULL, amqp_cstring_bytes(msg.c_str()));
      throwOnError("publish failed", true);
    }
    void foo(const std::string &s, const unsigned long u, const bool b) {}
  };

  class RabbitMQPublisher : public RabbitMQProducer<RabbitMQPublisher> {
   public:
    void prepare(const ProducerConfiguration &config) {
      auto dyconfig = dynamic_cast<const PublisherConfiguration *>(&config);

      openChannel(config.channelId);
      amqp_exchange_declare(connection, config.channelId, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                            amqp_cstring_bytes("fanout"), 0, 1, 0, 0, amqp_empty_table);
      throwOnError("declare exchange failed");
    }

    void send(const ProducerConfiguration &config, const std::string &msg) {
      auto dyconfig = dynamic_cast<const PublisherConfiguration *>(&config);
      amqp_basic_publish(connection, 1, amqp_cstring_bytes(dyconfig->exchange.c_str()), amqp_empty_bytes, 0,
                         0, NULL, amqp_cstring_bytes(msg.c_str()));
      throwOnError("publish failed", true);
    }

    void foo(const std::string &s, const unsigned long u, const bool b) {}
  };

  class RabbitMQTopicProducer : public RabbitMQProducer<RabbitMQTopicProducer> {
   public:
    void prepare(const ProducerConfiguration &config) {
      auto dyconfig = dynamic_cast<const TopicProducerConfiguration *>(&config);

      openChannel(config.channelId);
      amqp_exchange_declare(connection, config.channelId, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                            amqp_cstring_bytes("topic"), 0, 1, 0, 0, amqp_empty_table);
      throwOnError("declare exchange failed");
    }
    void send(const ProducerConfiguration &config, const std::string &key, const std::string &msg) {
      auto dyconfig = dynamic_cast<const TopicProducerConfiguration *>(&config);
      amqp_basic_publish(connection, 1, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                         amqp_cstring_bytes(key.c_str()), 0, 0, NULL, amqp_cstring_bytes(msg.c_str()));
      throwOnError("publish failed", true);
    }
  };
};  // namespace RabbitMQCpp
#endif