#include "receiver.h"

#include <deca_device_api.h>
#include <deca_regs.h>
#include <port.h>
#include <string.h>
#include "dw_config.h"
#include "dw_helpers.h"

#define MAX_RX_SIZE 128

int init_dw1000_receiver();

void receiver_main()
{
	stdio_write("\r\nstarting receiver\r\n");

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

    setup_frame_filtering();

	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	uint8_t rx_buffer[MAX_RX_SIZE] = { 0 };

	while (1) {
		stdio_write("waiting for message... ");

		// set pdm to idle and wait for next blink message
		dwt_setrxtimeout(0);
		dwt_setpreambledetecttimeout(0);
		dwt_rxenable(DWT_START_RX_IMMEDIATE);

		statusReg = 0;
		while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

		if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
			print_timeout_errors(statusReg);

	        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
			continue;
		}
		else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
			print_status_errors(statusReg);

	        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
			continue;
		}

		memset(&rx_buffer[0], 0, MAX_RX_SIZE);

		uint32_t frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		if (frameLen <= RX_BUF_LEN) {
			dwt_readrxdata(rx_buffer, frameLen, 0);
		}

		stdio_write("received: 0x");
		print_bytes(&rx_buffer[0], frameLen);
		stdio_write("\r\n");
	}
}

int init_dw1000_receiver()
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
