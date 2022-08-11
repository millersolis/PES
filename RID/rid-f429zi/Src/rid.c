#include "rid.h"

#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "main.h"
#include "port.h"
#include "ranging.h"
#include "rid_auth.h"
#include "stdio.h"
#include <stdbool.h>
#include "stdio_d.h"
#include <stdint.h>
#include <string.h>

int init_dw1000();
void setup_frame_filtering();
void send_blink_msg();
rid_state_t perform_blink();

static rid_state_t rid_state = DISCOVERY;	// Initial state

uint8 rx_buffer[RX_BUF_LEN] = { 0 };


void rid_main()
{
	stdio_write("\r\nstarting RID\r\n");
	HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

	setup_frame_filtering();
//	dwt_setpreambledetecttimeout(RANGING_PREAMBLE_TIMEOUT);
	dwt_setpreambledetecttimeout(0);

	uint8_t numErrors = 0;


	while (1) {
//		dwt_setrxaftertxdelay(800);
//		dwt_setrxtimeout(0xffff);

		// State Machine
		switch(rid_state)
		{
			case DISCOVERY:
				stdio_write("\r\nIn DISCOVERY\r\n");
				HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);

				rid_state = perform_blink();
				break;

			case POLL:
				stdio_write("\r\nIn POLL\r\n");
				HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);

				rid_state = init_ranging();
				break;

			case RANGING_FINAL:
				stdio_write("\r\nIn RANGING_FINAL\r\n");
				HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);

				rid_state = POLL;

				if (send_ranging_final_msg() == DWT_ERROR) {
					// Check for max errors in ranging
					if (numErrors >= 5) {
						stdio_write("WARNING: Exiting ranging due to excessive errors\r\n");
						rid_state = DISCOVERY;

						// Reset ranging error count
						numErrors = 0;
					}
					else {
						++numErrors;
					}
				}
				break;

			case AUTH_REPLY:
				stdio_write("\r\nIn AUTH_REPLY\r\n");
				HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);

				// do auth here
				print_auth_request_msg(rx_buffer);


				rid_state = DISCOVERY;	// DEBUG
				HAL_Delay(2000);	//DEBUG

				HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
				break;

			default:
				// default state
				stdio_write("\r\nWARNING: In default state\r\n");
				rid_state = DISCOVERY;
		}
//		HAL_Delay(500);
	}
}

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

    return DWT_SUCCESS;
}

void setup_frame_filtering()
{
	uint16 pan_id = 0xDECA;
	uint16 short_addr = 0x3152; // 'R', '1' (little-endian)
	uint8 extended_addr[] = {'R', 'I', 'D', '0', '0', '0', '0', '1'};

    dwt_setpanid(pan_id);
    dwt_setaddress16(short_addr);
    dwt_seteui(extended_addr);

	dwt_enableframefilter(DWT_FF_BEACON_EN | DWT_FF_DATA_EN);
}

rid_state_t perform_blink()
{
	dwt_setrxtimeout(DISCOVERY_RESP_TO_UUS);
//	dwt_setrxtimeout(0);

	send_blink_msg();

	rid_state_t next_state = DISCOVERY;

	// poll for ranging init
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;
	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);

        /* Clear good RX frame event in the DW1000 status register. */
        dwt_write32bitreg(SYS_STATUS_ID, msgReceivedFlags);
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);

        /* Clear good RX frame event in the DW1000 status register. */
        dwt_write32bitreg(SYS_STATUS_ID, msgReceivedFlags);
	}
	else {
		memset(rx_buffer, 0, sizeof(rx_buffer));

		uint32 frameLen;
		frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		if (frameLen <= RX_BUF_LEN) {
			dwt_readrxdata(rx_buffer, frameLen, 0);
		}

		if (is_ranging_init_msg(rx_buffer)) {
			stdio_write("received ranging-init message\r\n");
			next_state = POLL;
		}
		else if (is_auth_request_msg(rx_buffer)) {
			stdio_write("received auth request message\r\n");
			next_state = AUTH_REPLY;
		}
		else {
			stdio_write("received non-ranging-init or non-auth-request message\r\n");
		}
	}

    return next_state;
}

void send_blink_msg()
{
	/* Write frame data to DW1000 and prepare transmission. See NOTE 7 below. */
	dwt_writetxdata(sizeof(blink_msg), blink_msg, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(blink_msg), 0, 0); /* Zero offset in TX buffer, no ranging. */

	/* Start transmission, indicating that a response is expected so that reception is enabled immediately after the frame is sent. */
    if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS) {
    	stdio_write("ERROR: Sending blink message failed.\r\n");
    }
}

