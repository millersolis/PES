#ifndef RANGING_H
#define RANGING_H

#include <stdbool.h>
#include <stdint.h>

#include "dw_config.h"
#include "dw_helpers.h"

bool is_poll_msg(uint8_t* buffer);
bool is_final_msg(uint8_t* buffer);

int try_ranging(double* distance);

receive_status_t receive_poll_msg(uint8_t buffer[RX_BUF_LEN]);
send_status_t send_response_msg();
receive_status_t receive_final_msg(uint8_t buffer[RX_BUF_LEN]);
double retrieve_ranging_result(uint8_t buffer[RX_BUF_LEN]);

extern uint8_t ranging_init_msg[18];

#endif // RANGING_H
