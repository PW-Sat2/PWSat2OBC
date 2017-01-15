#ifndef LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_
#define LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_

#include <cstdint>

namespace experiment
{
    namespace fibo
    {
        class Fibonacci
        {
          public:
            Fibonacci();
            void Next();
            std::uint32_t Current() const;

          private:
            std::uint32_t _counter;
            std::uint32_t _prev;
            std::uint32_t _current;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_ */
