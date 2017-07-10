#ifndef SAFE_MODE_STEPS_STEPS_HPP_
#define SAFE_MODE_STEPS_STEPS_HPP_

#include <cstdint>
#include <tuple>

template <typename... Steps> class RecoverySteps
{
  public:
    void Perform();

  private:
    template <std::uint8_t Tag, typename Head, typename... Rest> void PerformEachStep()
    {
        std::get<Head>(this->_steps).Perform();
        PerformEachStep<Tag, Rest...>();
    }

    template <std::uint8_t Tag> void PerformEachStep()
    {
    }

    std::tuple<Steps...> _steps;
};

template <typename... Steps> void RecoverySteps<Steps...>::Perform()
{
    this->PerformEachStep<0, Steps...>();
}

#endif /* SAFE_MODE_STEPS_STEPS_HPP_ */
