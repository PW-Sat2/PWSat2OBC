#include "fibo.h"

namespace experiment
{
    namespace fibo
    {
        Fibonacci::Fibonacci() : _counter(1), _prev(1), _current(1)
        {
        }

        void Fibonacci::Next()
        {
            this->_counter++;

            switch (this->_counter)
            {
                case 1:
                    break;
                case 2:
                    break;
                default:
                    auto a = this->_prev;
                    this->_prev = this->_current;
                    this->_current += a;
                    break;
            }
        }

        std::uint32_t Fibonacci::Current() const
        {
            return this->_current;
        }
    }
}
