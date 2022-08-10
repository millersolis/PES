#include "pdm.h"

#include "dw_config.h"
#include "port.h"
#include "ranging.h"
#include "stdio.h"
#include "stdio_d.h"

typedef enum {
	NO_OP,
	WAKEUP,
	LOCK
} ecu_action_t;

ecu_action_t check_ecu_action();
void perform_action_on_bike(const ecu_action_t action);

static double distance = 0.0;
static double prevDistance = 0.0;

int init_dw1000()
{
	setup_DW1000RSTnIRQ(0);

	reset_DW1000();
	port_set_dw1000_slowrate();

	if (dwt_initialise(DWT_LOADUCODE) != DWT_SUCCESS) {
		return DWT_ERROR;
	}

	port_set_dw1000_fastrate();

	dwt_configure(&dw1000_config);

	dwt_setrxantennadelay(RX_ANT_DLY);
	dwt_settxantennadelay(TX_ANT_DLY);

	dwt_setpreambledetecttimeout(PRE_TIMEOUT);

	return DWT_SUCCESS;
}

void pdm_main()
{
	stdio_write("\r\nstarting PDM\r\n");

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

	char dist_str[32] = {0};

	while (1) {
		double tmpDistance;

		if (try_ranging(&tmpDistance) != DWT_SUCCESS) {
			continue;
		}

		prevDistance = distance;
		distance = tmpDistance;

		/* Display computed distance. */
		snprintf(dist_str, 32, "DIST: %3.2f m\r\n", distance);
		stdio_write(dist_str);

		ecu_action_t action = check_ecu_action();

		if (action == NO_OP) {
			continue;
		}

		perform_action_on_bike(action);
	}
}

ecu_action_t check_ecu_action()
{
	if (prevDistance >= 1.0 && distance < 1.0) {
		return WAKEUP;
	}

	if (prevDistance <= 1.0 && distance > 1.0) {
		return LOCK;
	}

	return NO_OP;
}

void perform_action_on_bike(const ecu_action_t action) {
	if (action == WAKEUP) {
		stdio_write("got wakeup signal\r\n");
	}
	else if (action == LOCK) {
		stdio_write("got lock signal\r\n");
	}
}
