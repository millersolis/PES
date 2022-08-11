#ifndef RANGING_H
#define RANGING_H

#include <stdint.h>
#include <stdbool.h>
#include "rid.h"

rid_state_t init_ranging();
int send_ranging_final_msg();
bool is_ranging_init_msg(uint8_t* rx_buffer);
bool is_rx_resp_msg(uint8_t* rx_buffer);
void send_poll_msg();

#define RANGING_INIT_MSG_COMMON_LEN 16
#define POLL_RESP_TO_UUS 50000

static uint8_t ranging_init_msg[] = {
	0x41, 0x8C,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', 'I', 'D', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'P', '1',								// source address; pdm short identifier
	0x20,									// data; 0x20 is ranging init function code
	0, 0									// FCS; filled as CRC of the frame by hardware
};

// standard ranging packet formats
extern uint8_t tx_poll_msg[12];
extern uint8_t rx_resp_msg[15];
extern uint8_t tx_final_msg[24];

#endif // RANGING_H
