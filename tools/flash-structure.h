// Flash structure definition for Hex Editor Neo
// Dump whole flash and bind structure Flash to address

[display(ref(description))]
struct Entry
{
	unsigned int length;
	$shift_by(32 - 4);
	[format("b16")] unsigned short crc;
	$shift_by(32 - 2);
	[format("b16")] unsigned char isValid;
	if(isValid == 0xAA)
	{
		$shift_by(64 - 1);
		string description[128];
		$shift_by(1024-128-length(description));
		char program[length];
		$shift_by(512 * 1024 - length - 1024);
	}
	else
	{
		$print("description", "Not valid");
		$shift_by(512 * 1024 - 64 - 1);
	}
};

struct BootloaderCopy
{
	unsigned char content[64 * 1024];
};

public struct Flash
{	
	Entry entries[6];
	BootloaderCopy copies[5];
};