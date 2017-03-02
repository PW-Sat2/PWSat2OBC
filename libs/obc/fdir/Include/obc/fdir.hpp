#ifndef LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_
#define LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_

#include "error_counter/error_counter.hpp"

namespace obc
{
    class FDIR final : private error_counter::IErrorCountingCallback, private error_counter::IErrorCountingConfigration
    {
      public:
        FDIR();

        void Initalize();

        void PostStartInitialize();

        error_counter::ErrorCounting& ErrorCounting();

      private:
        virtual void LimitReached(error_counter::Device device, error_counter::CounterValue errorsCount) override;
        virtual error_counter::CounterValue Limit(error_counter::Device device) override;
        virtual error_counter::CounterValue Increment(error_counter::Device device) override;

        error_counter::ErrorCounting _errorCounting;
    };
}

#endif /* LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_ */
