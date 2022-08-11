#include "rid_auth.h"

#include <stdbool.h>
#include <string.h>
#include "stdio.h"
#include "stdio_d.h"
#include "dw_config.h"
#include "rid_auth.h"


bool is_auth_request_msg(uint8_t* rx_buffer)
{
	rx_buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	return (memcmp(rx_buffer, auth_request_msg, sizeof(auth_request_msg)) == 0);
}

void print_auth_request_msg(uint8_t* rx_buffer)
{
	if (is_auth_request_msg(rx_buffer))
	{
		char str[200] = { '\0' };

		// Parse sequence number
		char seq_num_str[2] = { '\0' };
		sprintf(seq_num_str, "%i", rx_buffer[ALL_MSG_SN_IDX]);

		// Parse source ID
		char source_id_str[ADDRESS_LEN + 1] = { '\0' };
		for (int i = 0 ; i < ADDRESS_LEN; i++)
		{
			source_id_str[i] = (char)rx_buffer[i + AUTH_FRAME_SRC_IDX];
		}

		// Format and print
		strcat(str, "AUTH REQ = ");
		strcat(str, "Seq Num: ");
		strcat(str, seq_num_str);
		strcat(str, ", Src ID: ");
		strcat(str, source_id_str);
		strcat(str, "\r\n");

		stdio_write(str);
	}
	else
	{
		stdio_write("ERROR: Message is not of auth request type. Cannot print its contents.");
	}
}
