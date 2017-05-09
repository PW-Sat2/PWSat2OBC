.boot_params : 
{
	PROVIDE(__boot_params_start = .);
	
  	KEEP(*(SORT(.boot_param*)))  	
} > INTERNAL_RAM

ASSERT(__boot_params_start == 0x20000000, "Boot params must be at the begining of internal RAM")