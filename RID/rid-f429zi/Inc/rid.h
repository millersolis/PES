#ifndef RID_C
#define RID_C

#include <stdint.h>
#include <stdbool.h>

void rid_main();

typedef enum {
	DISCOVERY,
	POLL,
	RANGING_FINAL,
	AUTH_REPLY
} rid_state_t;


/* Receive response after Blink timeout, expressed in UWB microseconds. */
#define DISCOVERY_RESP_TO_UUS 0xffff

static uint8_t blink_msg[] = {
	0x41, 0xCC,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', 'D', 'M', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'R', 'I', 'D', '0', '0', '0', '0', '1', // source address; rid extended identifier
	0x21,									// data; 0x21 is function code for blink
	0, 0									// FCS; filled as CRC of the frame by hardware
};

#endif // RID_C
