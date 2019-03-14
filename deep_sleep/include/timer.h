#include <stdint.h>
#include <chrono>

void ConfigureBurtc();
void ArmBurtc();
std::chrono::milliseconds GetTime();