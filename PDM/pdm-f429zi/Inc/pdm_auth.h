#ifndef PDM_AUTH_H
#define PDM_AUTH_H

#include <stdbool.h>
#include <stdint.h>
#include "dw_config.h"
#include "dw_helpers.h"

extern bool is_auth_reply_msg(uint8_t buffer[RX_BUF_LEN]);

extern send_status_t send_auth_request();
extern receive_status_t receive_auth_reply(uint8_t buffer[RX_BUF_LEN]);
extern send_status_t send_auth_ack(uint8_t data[16], uint8_t iv[16]);

extern uint8_t auth_request_msg[24];
extern uint8_t auth_reply_msg[56];
extern uint8_t auth_ack_msg[56];

/* Indexes to access to fields of the data frame in the auth messages array. */
#define AUTH_FRAME_DATA_IDX 22	// Data index (64-bit long)
#define AUTH_FRAME_FC_IDX 21	// Function code index
#define AUTH_FRAME_SRC_IDX 13	// Source address index
#define AUTH_FRAME_DEST_IDX 5	// Destination address index
#define AUTH_FRAME_COMMON_LEN 22

#endif // PDM_AUTH_H
