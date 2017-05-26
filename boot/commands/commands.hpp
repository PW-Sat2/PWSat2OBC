#ifndef BOOT_COMMANDS_COMMANDS_HPP_
#define BOOT_COMMANDS_COMMANDS_HPP_

void Test();
void TestSRAM();
void TestEEPROM();

void BootUpper();
void BootToSelected();
void TMRBoot();
void SetRunlevel();

void SetBootIndex();

void UploadApplication();
void UploadSafeMode();

void PrintBootTable();

#endif /* BOOT_COMMANDS_COMMANDS_HPP_ */
