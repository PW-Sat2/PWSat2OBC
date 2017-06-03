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
void ShowBootSettings();

void UploadApplication();
void UploadSafeMode();

void PrintBootTable();
void EraseBootTable();

#endif /* BOOT_COMMANDS_COMMANDS_HPP_ */
