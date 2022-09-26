#ifndef RID_C
#define RID_C

#include <stdbool.h>
#include <stdint.h>

void rid_main();

typedef enum rid_state_t {
	STATE_DISCOVERY,
	STATE_RANGING_POLL,
	STATE_RANGING_FINAL,
	STATE_RANGING,
	STATE_AUTHENTICATION
} rid_state_t;

typedef struct state_t {
	rid_state_t value;
	bool hasChanged;
} state_t;

extern uint8_t blink_msg[24];

#define RNG_INIT_MAX_RX_RETRY 3

#endif // RID_C
