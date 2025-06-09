# RabbitMQ C++ Wrapper

This is a simple set of classes and test harnesses to wrap the RabbitMQ C client API in a C++ interface.

## Prerequisites

A C++20 compiler is required.

The `rabbitmq-c` client library must be installed to provide headers and libraries.

A RabbitMQ server is required. This can be insstalled locally, in a container or on a remote server.

## Build

Run `mkdir build`, then `./CMAKE`, then `./BUILD`. Executables will be created in `build`.

## Namespace

Everything is in a namespace `RabbitMQCpp`.

## Classes

There are two sets of classes, producers and consumers. In each case there is an asbtract base class, `RabbitMQProducer` and `RabbitMQConsumer` respectively.

`RabbitMQProducer` has three derived classes:

- `RabbitMQDirectProducer`
- `RabbitMQPublisher`
- `RabbitMQTopicProducer`

`RabbitMQConsumer` has three derived classes:

- `RabbitMQDirectConsumer`
- `RabbitMQSubscriber`
- `RabbitMQTopicConsumer`

These implement the direct, publish/subscribe abd topic exchanges respectively.

There are three pairs of test harnesses using the three exchange categories:

- `prodicer`/`consumer`
- `publisher`/`subscriber`
- `topicproducer`/`topicconsumer`

## Configuration

The test harnesses are configured by reading a JSON file. This file is parsed by N Lohmann's JSON support library. Installation of this is completely optional but the test harnesses will not work as is without it. A configuration file looks like:

```json
{
  "hostname": "<host>",
  "username": "<user>",
  "password": "<password>",
  "vhost": "<vhost>",
  "port": <the port, usually 5672>
}
```
