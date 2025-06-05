cmake_minimum_required(VERSION 3.28)
project(rabbitmqclass LANGUAGES CXX VERSION 0.0.1)
add_compile_options(-std=c++23 -Wall -Wunused -Wnrvo)
find_library(RABBITMQ rabbitmq)
find_package(nlohmann_json 3.12.0 REQUIRED)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer PUBLIC "${RABBITMQ}")

add_executable(subscriber subscriber.cpp)
target_link_libraries(subscriber PUBLIC "${RABBITMQ}")

add_executable(topicconsumer topicconsumer.cpp)
target_link_libraries(topicconsumer PUBLIC "${RABBITMQ}")

add_executable(producer producer.cpp)
target_link_libraries(producer PUBLIC "${RABBITMQ}")

add_executable(publisher publisher.cpp)
target_link_libraries(publisher PUBLIC "${RABBITMQ}")

add_executable(topicproducer topicproducer.cpp)
target_link_libraries(topicproducer PUBLIC "${RABBITMQ}")
