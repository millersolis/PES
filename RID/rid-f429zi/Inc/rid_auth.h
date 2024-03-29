#ifndef RID_AUTH_H
#define RID_AUTH_H

#include <stdbool.h>
#include <stdint.h>
#include "dw_config.h"
#include "dw_helpers.h"

extern bool is_auth_request_msg(uint8_t rx_buffer[RX_BUF_LEN]);
extern bool is_auth_ack_msg(uint8_t buffer[RX_BUF_LEN]);

extern receive_status_t receive_auth_request(uint8_t buffer[RX_BUF_LEN]);
extern send_status_t send_auth_reply(uint8_t data[16], uint8_t iv[16]);
extern receive_status_t receive_auth_ack(uint8_t buffer[RX_BUF_LEN]);

void print_auth_request_msg(uint8_t* rx_buffer);

/* NOTE A. EUI64 is not actually used in this example but the DW1000 is set up with this dummy value, to have it set to something. This would be required
*    for a real application, i.e. because short addresses (and PAN ID) are typically assigned by a PAN coordinator.*/

/*     - byte 0/1: frame control (0xCC41 for 64-bit source and destination address).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN ID (0xDECA).
 *     - byte 5/6: destination address, R1 for RID #1.
 *     - byte 7/8: source address, P1 for PDM #1.
 *     - byte 9: function code 0x80(specific values to indicate which message it is in the authentication process).*/
extern uint8_t auth_request_msg[24];

/* The frame sent in this example is a data frame encoded as per the IEEE 802.15.4-2011 standard. It is a 21-byte frame composed of the following
 * fields:
 *     - byte 0/1: frame control (0xCC41 to indicate a data frame using 64-bit addressing).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN ID (0xDECA) NOTE A
 *     - byte 5/6: destination address, see NOTE 2 below.
 *     - byte 7/8: source address, see NOTE 2 below.
 *     - byte 9 to 18: MAC payload, see NOTE 1 below.
 *     - byte 19/20: frame check-sum, automatically set by DW1000. */
extern uint8_t auth_reply_msg[56];

/*     - byte 0/1: frame control (0xCC41 to indicate a data frame using 64-bit addressing).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN ID (0xDECA) NOTE A
 *     - byte 5/6: destination address, see NOTE 2 below.
 *     - byte 7/8: source address, see NOTE 2 below.
 *     - byte 9 to 18: MAC payload, see NOTE 1 below.
 *     - byte 19/20: frame check-sum, automatically set by DW1000. */
extern uint8_t auth_ack_msg[56];

/* Indexes to access to fields of the data frame in the auth messages array. */
#define AUTH_FRAME_DATA_IDX 22	// Data index (64-bit long)
#define AUTH_FRAME_FC_IDX 21	// Function code index
#define AUTH_FRAME_SRC_IDX 13	// Source address index
#define AUTH_FRAME_DEST_IDX 5	// Destination address index
#define AUTH_FRAME_COMMON_LEN 22

/* Buffer to store received frame. See NOTE 1 below. */
#define FRAME_LEN_MAX 127
#define ADDRESS_LEN 8	// 64-bit / 8-byte address used for both PDM and RID

#endif // RID_AUTH_H
