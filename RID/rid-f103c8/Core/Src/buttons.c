#include "buttons.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "deca_regs.h"
#include "dw_config.h"
#include "stdio_d.h"


uint8_t user_lock_msg[24] = {
	0x41, 0x88,								// frame control; data frame, pan compression, 64-bit addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', '1',	// destination address; PDM #1 extended identifier
	'R', '1',	// source address; RID #1 extended identifier
	0x79,									// data; 0x79 is the user requested lock msg code
	0, 0									// FCS; filled as CRC of the frame by hardware
};
uint8_t user_panic_msg[24] = {
	0x41, 0x88,								// frame control; data frame, pan compression, 64-bit addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', '1',	// destination address; PDM #1 extended identifier
	'R', '1',	// source address; RID #1 extended identifier
	0x78,									// data; 0x78 is the user requested lock msg code
	0, 0									// FCS; filled as CRC of the frame by hardware
};


static uint8_t frame_seq_nb = 0;


bool is_user_lock_msg(uint8_t* rx_buffer)
{
	rx_buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	user_lock_msg[ALL_MSG_SN_IDX] = 0;
	return (memcmp(rx_buffer, user_lock_msg, USER_BTN_FRAME_COMMON_LEN) == 0);
}

send_status_t send_user_lock_msg()
{
	user_lock_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
	dwt_writetxdata(sizeof(user_lock_msg), user_lock_msg, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(user_lock_msg), 0, 0); /* Zero offset in TX buffer, non-ranging. */

	if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
		stdio_write("tx user lock message failed\r\n");
		return STATUS_SEND_ERROR;
	}

	stdio_write("BTN: tx user lock message sent\r\n");
	++frame_seq_nb;

	return STATUS_SEND_OK;
}

bool is_user_panic_msg(uint8_t* rx_buffer)
{
	rx_buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	user_panic_msg[ALL_MSG_SN_IDX] = 0;
	return (memcmp(rx_buffer, user_panic_msg, USER_BTN_FRAME_COMMON_LEN) == 0);
}

send_status_t send_user_panic_msg()
{
	user_panic_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
	dwt_writetxdata(sizeof(user_panic_msg), user_panic_msg, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(user_panic_msg), 0, 0); /* Zero offset in TX buffer, non-ranging. */

	if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
		stdio_write("tx user panic message failed\r\n");
		return STATUS_SEND_ERROR;
	}

	stdio_write("BTN: tx user panic message sent\r\n");
	++frame_seq_nb;

	return STATUS_SEND_OK;
}
