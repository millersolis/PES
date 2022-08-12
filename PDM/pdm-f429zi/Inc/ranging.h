#ifndef RANGING_H
#define RANGING_H

#include <stdbool.h>
#include <stdint.h>

#include "dw_config.h"

typedef enum receive_status_t {
	STATUS_RECEIVE_OK,
	STATUS_RECEIVE_ERROR,
	STATUS_RECEIVE_TIMEOUT
} receive_status_t;

typedef enum send_status_t {
	STATUS_SEND_OK,
	STATUS_SEND_ERROR
} send_status_t;

bool is_poll_msg(uint8_t* buffer);
bool is_final_msg(uint8_t* buffer);

int try_ranging(double* distance);

receive_status_t receive_poll_msg(uint8_t buffer[RX_BUF_LEN]);
send_status_t send_response_msg();
receive_status_t receive_final_msg(uint8_t buffer[RX_BUF_LEN]);
double retrieve_ranging_result(uint8_t buffer[RX_BUF_LEN]);

extern uint8_t ranging_init_msg[18];

#endif // RANGING_H
