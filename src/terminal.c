#include <em_gpio.h>
#include "em_leuart.h"
#include "port_map.h"
#include "leuart.h"

void terminal_handle_command(uint8_t* buffer, uint32_t length)
{
	terminal_send_new_line();

	if(strcmp(buffer, "ping") == 0)
	{
		terminal_send_new_prefix();
		leuart_send_string("pong");
		terminal_send_new_line();
	}

	terminal_send_new_prefix();
}

void terminal_send_new_line()
{
	leuart_send_string("\r\n");
}

void terminal_send_new_prefix()
{
	leuart_send_string("PW-SAT2->");
}

