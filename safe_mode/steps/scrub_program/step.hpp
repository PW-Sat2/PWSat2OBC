#ifndef SAFE_MODE_STEPS_SCRUB_PROGRAM_STEP_HPP_
#define SAFE_MODE_STEPS_SCRUB_PROGRAM_STEP_HPP_

#include <cstdint>

class ScrubProgram
{
  public:
    void Perform();

  private:
    void PerformOnSlots(std::uint8_t slots);
};

#endif /* SAFE_MODE_STEPS_SCRUB_PROGRAM_STEP_HPP_ */
