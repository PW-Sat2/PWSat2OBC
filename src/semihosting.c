#if SEMIHOSTING
#include <stdio.h>

extern void initialise_monitor_handles(void);
extern void __libc_init_array(void);

void initialize_semihosting()
{
	__libc_init_array();

	initialise_monitor_handles();
}

#endif
