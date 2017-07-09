#ifndef SAFE_MODE_STEPS_REBOOT_STEP_HPP_
#define SAFE_MODE_STEPS_REBOOT_STEP_HPP_

#include <core_cm3.h>

class RebootStep
{
  public:
    inline void Perform();
};

void RebootStep::Perform()
{
    NVIC_SystemReset();
}

#endif /* SAFE_MODE_STEPS_REBOOT_STEP_HPP_ */
