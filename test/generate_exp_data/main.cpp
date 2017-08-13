#include <cstdio>

extern "C" {
extern void initialise_monitor_handles(void);
extern void __libc_init_array(void);
extern int kill(pid_t, int);
}

int main()
{
    __libc_init_array();

    initialise_monitor_handles();

    printf("test\n");

    kill(-1, 0);

    return 0;
}
