#ifndef BOOT_COMMANDS_COMMANDS_HPP_
#define BOOT_COMMANDS_COMMANDS_HPP_

void Test();
void TestSRAM();
void TestEEPROM();

void BootUpper();
void SetRunlevel();
void SetClearState();

void SetBootIndex();
void ShowBootSettings();
void SetBootSlotToSafeMode();
void SetBootSlotToUpper();

void UploadApplication();
void UploadSafeMode();
void CopyBootloader();

void PrintBootTable();
void EraseBootTable();

void Check();
void Recovery();
#endif /* BOOT_COMMANDS_COMMANDS_HPP_ */
