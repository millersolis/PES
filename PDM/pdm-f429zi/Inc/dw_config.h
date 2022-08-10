#ifndef DW_CONFIG_H
#define DW_CONFIG_H

#include <stdint.h>

#include "deca_device_api.h"

//conversion factor from microseconds to DW1000 device time units
#define UUS_TO_DWT_TIME 65536

// delays for ranging
#define TX_ANT_DLY 16505                // tx antenna delay (default 16505)
#define RX_ANT_DLY 16505                // rx antenna delay (default 16505)
#define POLL_RX_TO_RESP_TX_DLY_UUS 2750 // timeout from poll rx timestamp to response tx timestamp (default 2750)
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500 // timeout from ranging response tx response to rx enable (default 500)
#define FINAL_RX_TIMEOUT_UUS 3300       // timeout to receive final ranging packet (default 3300)
#define PRE_TIMEOUT 8                   // preamble timeout (default 8)

/* Default communication configuration. We use here EVK1000's default mode (mode 3). */
extern dwt_config_t dw1000_config;

// standard ranging packet formats
extern uint8_t rx_poll_msg[12];
extern uint8_t tx_resp_msg[15];
extern uint8_t rx_final_msg[24];

// ranging packet parameters
#define ALL_MSG_SN_IDX 2             // frame byte index of sequence number (default 2)
#define FINAL_MSG_POLL_TX_TS_IDX 10  // index of poll tx timestamp in final ranging frame (default 10)
#define FINAL_MSG_RESP_RX_TS_IDX 14  // index of response rx timestamp in final ranging frame (default 14)
#define FINAL_MSG_FINAL_TX_TS_IDX 18 // index of final tx timestamp in final ranging frame (default 18)
#define ALL_MSG_COMMON_LEN 10        // common length of ranging packets (default 10)
#define FINAL_MSG_TS_LEN 4           // length of timestamp in final ranging frame (default 4)

/* Speed of light in air, in metres per second. */
#define SPEED_OF_LIGHT 299702547

#endif // DW_CONFIG_H
