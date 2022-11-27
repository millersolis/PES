#ifndef PDM_H
#define PDM_H

#include <stdbool.h>
#include <stdint.h>

#define SIM_CONNECTED
#define FENCING_THRESHOLD 2.0		// in meters

// DEBUG
//#define TRIGGER_ACTION_PERIODICALLY
//#define INFINITE_RETRIES_RX

extern void pdm_main();

typedef enum ecu_action_t {
	NO_OP,
	WAKEUP,
	LOCK,
	START,
	PANIC
} ecu_action_t;

typedef enum pdm_state_t {
	STATE_DISCOVERY,
	STATE_RANGING,
	STATE_AUTHENTICATION
} pdm_state_t;

typedef struct state_t {
	pdm_state_t value;
	bool hasChanged;
} state_t;

#define BLINK_MSG_COMMON_LEN 22
static uint8_t blink_msg[] = {
	0x41, 0xCC,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', 'D', 'M', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'R', 'I', 'D', '0', '0', '0', '0', '1', // source address; rid extended identifier
	0x21,									// data; 0x21 is function code for blink
	0, 0									// FCS; filled as CRC of the frame by hardware
};

#define POLL_MAX_RX_RETRY 3

#endif // PDM_H
