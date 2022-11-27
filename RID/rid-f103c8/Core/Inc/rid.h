#ifndef RID_C
#define RID_C

#include <stdbool.h>
#include <stdint.h>

// DEBUG
#define INFINITE_RETRIES_RX

void rid_main();

typedef enum rid_state_t {
	STATE_DISCOVERY,
	STATE_RANGING,
	STATE_AUTHENTICATION
} rid_state_t;

typedef struct state_t {
	rid_state_t value;
	bool hasChanged;
} state_t;

extern uint8_t blink_msg[24];

#define MAX_RNG_RETRY 3

#endif // RID_C
