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

public struct Flash
{
	unsigned char bootIndex;
	$shift_by(0x00002000 - 1);
	unsigned char bootCounter;
	$shift_by(0x00002000 - 1);
	[format("b16")] unsigned short crc;
	$shift_by(0x00080000 - 0x00004000 - 1 - 1);
	Entry entries[7];
};