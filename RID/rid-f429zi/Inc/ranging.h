#ifndef RANGING_H
#define RANGING_H

#include <stdbool.h>
#include <stdint.h>

#include "dw_config.h"

// needed to avoid timeouts while waiting for PDM processing
#define MIN_DELAY_RANGING 30

typedef enum receive_status_t {
	STATUS_RECEIVE_OK,
	STATUS_RECEIVE_ERROR,
	STATUS_RECEIVE_TIMEOUT
} receive_status_t;

typedef enum send_status_t {
	STATUS_SEND_OK,
	STATUS_SEND_ERROR
} send_status_t;

bool is_ranging_init_msg(uint8_t* buffer);
bool is_rx_resp_msg(uint8_t* buffer);

send_status_t send_poll_msg();
receive_status_t receive_response_msg(uint8_t buffer[RX_BUF_LEN]);
send_status_t send_final_msg();

#define RANGING_INIT_MSG_COMMON_LEN 16

static uint8_t ranging_init_msg[] = {
	0x41, 0x8C,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', 'I', 'D', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'P', '1',								// source address; pdm short identifier
	0x20,									// data; 0x20 is ranging init function code
	0, 0									// FCS; filled as CRC of the frame by hardware
};

#endif // RANGING_H
