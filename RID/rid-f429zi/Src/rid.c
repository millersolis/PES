#include "rid.h"

#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "main.h"
#include "port.h"
#include "ranging.h"
#include "stdio.h"
#include "stdio_d.h"

int init_dw1000();
void setup_frame_filtering();
int perform_blink();


static uint8_t blink_msg[] = {
	0x41, 0xCC,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', 'D', 'M', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'R', 'I', 'D', '0', '0', '0', '0', '1', // source address; rid extended identifier
	0x21,									// data; 0x21 is function code for blink
	0, 0									// FCS; filled as CRC of the frame by hardware
};

#define RANGING_INIT_MSG_COMMON_LEN 16
static uint8_t ranging_init_msg[] = {
	0x41, 0x8C,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', 'I', 'D', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'P', '1',								// source address; pdm short identifier
	0x20,									// data; 0x20 is ranging init function code
	0, 0									// FCS; filled as CRC of the frame by hardware
};

static uint8_t rx_buffer[RX_BUF_LEN] = { 0 };

void rid_main()
{
	stdio_write("\r\nstarting RID\r\n");

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

	setup_frame_filtering();

	dwt_setpreambledetecttimeout(RANGING_PREAMBLE_TIMEOUT);

	while (1) {
		dwt_setrxaftertxdelay(800);
		dwt_setrxtimeout(0xffff);

		stdio_write("starting discovery\r\n");
		HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);

		if (perform_blink() != DWT_SUCCESS) {
			HAL_Delay(5);
			continue;
		}

		HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);
		stdio_write("starting ranging\r\n");
		HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);

		// quick reset of dw1000 state; for some reason ranging will not work
		// if this is not done first
		if (init_dw1000() != DWT_SUCCESS) {
			stdio_write("initializing dw1000 failed; spinlocking.\r\n");
			while (1);
		}

		// smaller reset that also works, but using a full reset for now just
		// in case this one introduces weird side-effects
//		HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_RESET);
//		usleep(1);
//		setup_DW1000RSTnIRQ(0);
//		Sleep(2);
//		dwt_configure(&dw1000_config);
//		dwt_setrxantennadelay(RX_ANT_DLY);
//		dwt_settxantennadelay(TX_ANT_DLY);

		// this is not part of the state reset; this is normally done for ranging
		dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
		dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

		uint8_t numErrors = 0;

		while (numErrors < 5) {
			if (perform_ranging() != DWT_SUCCESS) {
				++numErrors;
				HAL_Delay(5);
				continue;
			}

			HAL_Delay(100);
		}

		HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
		stdio_write("starting authentication\r\n");
		HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);

		// do auth here

		HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
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

int perform_blink()
{
    dwt_writetxdata(sizeof(blink_msg), blink_msg, 0);
    dwt_writetxfctrl(sizeof(blink_msg), 0, 1);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS) {
    	stdio_write("error: tx blink message failed.\r\n");
    	return DWT_ERROR;
    }

	// poll for ranging init
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;
	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	memset(rx_buffer, 0, sizeof(rx_buffer));

	uint32_t frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(rx_buffer, frameLen, 0);
	}

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);

        dwt_write32bitreg(SYS_STATUS_ID, msgReceivedFlags);
		return DWT_ERROR;
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);

        dwt_write32bitreg(SYS_STATUS_ID, msgReceivedFlags);
		return DWT_ERROR;
	}

	memset(rx_buffer, 0, sizeof(rx_buffer));

	frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(rx_buffer, frameLen, 0);
	}

	rx_buffer[2] = 0;
	if (memcmp(rx_buffer, ranging_init_msg, RANGING_INIT_MSG_COMMON_LEN) != 0) {
		stdio_write("received non-ranging-init message\r\n");
		return DWT_ERROR;
	}

    return DWT_SUCCESS;
}
