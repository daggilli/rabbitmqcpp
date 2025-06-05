#ifndef __RABBITMQCONSUMER_H__
#define __RABBITMQCONSUMER_H__

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "config.h"

namespace RabbitMQCpp {
  using ConsumerCallback = std::function<void(amqp_channel_t, amqp_bytes_t &, amqp_message_t &)>;

  class RabbitMQConsumer {
   public:
    RabbitMQConsumer() : connection(nullptr), socket(nullptr), channelOpen(false), consumerCb(nullCallback) {
      connection = amqp_new_connection();
      if (!connection) {
        throw std::runtime_error("create connection failed");
      }
      socket = amqp_tcp_socket_new(connection);
      if (!socket) {
        throw std::runtime_error(std::format("create TCP socket failed: {}", std::strerror(errno)));
      }
    }

    virtual ~RabbitMQConsumer() {
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

    virtual void prepare(const ConsumerConfiguration &config, ConsumerCallback &&callback) = 0;

    void consume() {
      amqp_envelope_t envelope;

      amqp_maybe_release_buffers(connection);

      auto reply = amqp_consume_message(connection, &envelope, NULL, 0);
      auto envstr = std::format("CHAN: {}\nCNSMR TAG: {}\nDLVRY TAG: {}\nRDLVRED: {}\nRTNG KEY: {}\n",
                                envelope.channel, byteString(envelope.consumer_tag), envelope.delivery_tag,
                                static_cast<bool>(envelope.redelivered), byteString(envelope.routing_key));
      std::cout << envstr;
      if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("consume message failed");
      }
      consumerCb(envelope.channel, envelope.consumer_tag, envelope.message);

      amqp_destroy_envelope(&envelope);
    }

    void setCallback(ConsumerCallback callback) {
      consumerCb = ConsumerCallback(std::forward<decltype(callback)>(callback));
    }

   protected:
    amqp_connection_state_t connection;
    amqp_socket_t *socket;
    bool channelOpen;
    ConsumerCallback consumerCb;

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

    void throwOnError(const std::string &msg) {
      auto amqpResult = amqp_get_rpc_reply(connection);
      if (amqpResult.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error(msg);
      }
    }

    static void nullCallback(amqp_channel_t channelId, amqp_bytes_t &consumerTag, amqp_message_t &msg) {}
  };  // RabbitMQConsumer

  class RabbitMQDirectConsumer : public RabbitMQConsumer {
   public:
    void prepare(const ConsumerConfiguration &config, ConsumerCallback &&callback) override {
      auto dyconfig = dynamic_cast<const DirectConsumerConfiguration *>(&config);
      openChannel(config.channelId);
      amqp_basic_consume(connection, config.channelId, amqp_cstring_bytes(dyconfig->queue.c_str()),
                         amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
      throwOnError("basic consume failed");

      setCallback(callback);
    }
  };  // RabbitMQDirectConsumer

  class RabbitMQSubscriber : public RabbitMQConsumer {
   public:
    void prepare(const ConsumerConfiguration &config, ConsumerCallback &&callback) override {
      const auto channelId = config.channelId;

      auto dyconfig = dynamic_cast<const SubscriberConfiguration *>(&config);
      openChannel(config.channelId);

      amqp_exchange_declare(connection, channelId, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                            amqp_cstring_bytes("fanout"), 0, 1, 0, 0, amqp_empty_table);
      throwOnError("declare exchange failed");

      //  empty queue paramater means pick a random queue name
      amqp_queue_declare_ok_t *queue =
          amqp_queue_declare(connection, channelId, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
      throwOnError("declare queue failed");

      auto queueName = amqp_bytes_malloc_dup(queue->queue);

      amqp_queue_bind(connection, channelId, queueName, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                      amqp_empty_bytes, amqp_empty_table);
      throwOnError("bind queue failed");

      amqp_basic_consume(connection, channelId, queueName, amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
      throwOnError("basic consume failed");

      amqp_bytes_free(queueName);

      setCallback(callback);
    }
  };  // RabbitMQSubscriber

  class RabbitMQTopicConsumer : public RabbitMQConsumer {
   public:
    void prepare(const ConsumerConfiguration &config, ConsumerCallback &&callback) override {
      const auto channelId = config.channelId;
      auto dyconfig = dynamic_cast<const TopicConsumerConfiguration *>(&config);
      openChannel(config.channelId);

      amqp_exchange_declare(connection, channelId, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                            amqp_cstring_bytes("topic"), 0, 1, 0, 0, amqp_empty_table);
      throwOnError("declare exchange failed");

      //  empty queue paramater means pick a random queue name
      amqp_queue_declare_ok_t *queue =
          amqp_queue_declare(connection, channelId, amqp_empty_bytes, 0, 0, 1, 1, amqp_empty_table);
      throwOnError("declare queue failed");

      auto queueName = amqp_bytes_malloc_dup(queue->queue);

      std::cout << "QN " << byteString(queueName) << "\n";

      for (auto &&topic : dyconfig->topics) {
        amqp_queue_bind(connection, channelId, queueName, amqp_cstring_bytes(dyconfig->exchange.c_str()),
                        amqp_cstring_bytes(topic.c_str()), amqp_empty_table);
        throwOnError("bind queue failed");
      }

      amqp_basic_consume(connection, channelId, queueName, amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
      throwOnError("basic consume failed");

      amqp_bytes_free(queueName);

      // consumerCb = ConsumerCallback(std::forward<ConsumerCallback &>(callback));
      setCallback(callback);
    }
  };
};  // namespace RabbitMQCpp
#endif
