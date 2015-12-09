#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "D:/PW-Sat/googletest/googletest/include/gtest/gtest.h"

extern "C" {
  extern void initialise_monitor_handles(void);
  extern void __libc_init_array(void);
}

static inline int
do_AngelSWI (int reason, void * arg)
{
  int value;
  asm volatile ("mov r0, %1; mov r1, %2; bkpt %a3; mov %0, r0"
       : "=r" (value) /* Outputs */
       : "r" (reason), "r" (arg), "i" (0xAB) /* Inputs */
       : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
		/* Clobbers r0 and r1, and lr if in supervisor mode */);
                /* Accordingly to page 13-77 of ARM DUI 0040D other registers
                   can also be clobbered.  Some memory positions may also be
                   changed by a system call, so they should not be kept in
                   registers. Note: we are assuming the manual is right and
                   Angel is respecting the APCS.  */
  return value;
}

#define ARGS_BUF_ARRAY_SIZE 80
#define ARGV_BUF_ARRAY_SIZE 10

typedef struct
{
  char* pCommandLine;
  int size;
} CommandLineBlock;

void __initialize_args (int* p_argc, char*** p_argv)
{

  // Array of chars to receive the command line from the host
  static char args_buf[ARGS_BUF_ARRAY_SIZE];

  // Array of pointers to store the final argv pointers (pointing
  // in the above array).
  static char* argv_buf[ARGV_BUF_ARRAY_SIZE];

  int argc = 0;
  int isInArgument = 0;

  CommandLineBlock cmdBlock;
  cmdBlock.pCommandLine = args_buf;
  cmdBlock.size = sizeof(args_buf) - 1;

  int ret = do_AngelSWI (0x15, &cmdBlock);
  if (ret == 0)
    {
      printf("Args: '%s'\n", cmdBlock.pCommandLine);
      // In case the host send more than we can chew, limit the
      // string to our buffer.
      args_buf[ARGS_BUF_ARRAY_SIZE - 1] = '\0';

      // The command line is a null terminated string
      char* p = cmdBlock.pCommandLine;

      int delim = '\0';
      int ch;

      while ((ch = *p) != '\0')
	{
	  if (isInArgument == 0)
	    {
	      if (!isblank(ch))
		{
		  if (argc
		      >= (int) ((sizeof(argv_buf) / sizeof(argv_buf[0])) - 1))
		    break;

		  if (ch == '"' || ch == '\'')
		    {
		      // Remember the delimiter to search for the
		      // corresponding terminator
		      delim = ch;
		      ++p;                        // skip the delimiter
		      ch = *p;
		    }
		  // Remember the arg beginning address
		  argv_buf[argc++] = p;
		  isInArgument = 1;
		}
	    }
	  else if (delim != '\0')
	    {
	      if ((ch == delim))
		{
		  delim = '\0';
		  *p = '\0';
		  isInArgument = 0;
		}
	    }
	  else if (isblank(ch))
	    {
	      delim = '\0';
	      *p = '\0';
	      isInArgument = 0;
	    }
	  ++p;
	}
    }

  if (argc == 0)
    {
      // No args found in string, return a single empty name.
      args_buf[0] = '\0';
      argv_buf[0] = &args_buf[0];
      ++argc;
    }

  // Must end the array with a null pointer.
  argv_buf[argc] = NULL;

  *p_argc = argc;
  *p_argv = &argv_buf[0];
  return;
}

GTEST_TEST(MyTest, MyName)
{
  printf("Running my test\n");
  EXPECT_EQ(2, 4);
}

GTEST_TEST(MyTest, ThisOneWorks)
{
  printf("Running my second test");
  EXPECT_EQ(2, 2);
}

void unit_tests(int argc, char ** argv)
{
  printf("Running unit tests\n");
  ::testing::InitGoogleTest(&argc, argv);
  printf("Running unit tests2\n");
  RUN_ALL_TESTS();
}

void test_file(void)
{
  char buf[] = "Hello ARM";
  FILE * f = fopen("D:/arm.txt", "w");
  fwrite(buf, sizeof(char), sizeof(buf), f);
  fclose(f);
}

int my_func(int i)
{
  if(i == 0) return i;
  return my_func(i-1) * i;
}

int main(){
  int i;

  char ** argv;
  int argc;

  __libc_init_array();

  initialise_monitor_handles();

  __initialize_args (&argc, &argv);

  //__run_init_array();

  printf("Hello World from QEMU guest\n");
  printf("Arg count: %d\n", argc);

  for(i = 0; i < 3; i++)
  {
    int j;
    //write();
    printf("Iteration %d\n", i);
    //for(j=0;j<10000000;j++);
  }

  test_file();

  unit_tests(argc, argv);

  printf("Result: %d\n", kill(-1, -1));
  return 73;
}
