#ifndef __RANDOMINT_H__
#define __RANDOMINT_H__

#include <random>

namespace RabbitMQCpp {
  using uidist = std::uniform_int_distribution<std::mt19937::result_type>;

  class RandomInt {
   public:
    RandomInt(const int lower, const int upper) : rng((std::random_device())()), ud(lower, upper) {}
    int operator()() { return ud(rng); }

   private:
    std::mt19937 rng;
    uidist ud;
  };
};  // namespace RabbitMQCpp
#endif