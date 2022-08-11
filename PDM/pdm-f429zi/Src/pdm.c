#include "pdm.h"

#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "main.h"
#include "pdm_can.h"
#include "port.h"
#include "ranging.h"
#include "stdio.h"
#include "stdio_d.h"

#define PDM_LOOP_DELAY 10

typedef enum {
	NO_OP,
	WAKEUP,
	LOCK
} ecu_action_t;

int init_dw1000();
void setup_frame_filtering();
int receive_blink();
ecu_action_t check_ecu_action();
void perform_action_on_bike(const ecu_action_t action);

static double distance = 0.0;
static double prevDistance = 0.0;

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

static uint8_t ranging_init_msg[] = {
	0x41, 0x8C,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', 'I', 'D', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'P', '1',								// source address; pdm short identifier
	0x20,									// data; 0x20 is ranging init function code
	0, 0									// FCS; filled as CRC of the frame by hardware
};
#define BLINK_RX_BUFFER_LEN 128
static uint8_t blink_rx_buffer[RX_BUFFER_LEN] = { 0 };

void pdm_main()
{
	stdio_write("\r\nstarting PDM\r\n");

	init_can(&hcan1);

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

    setup_frame_filtering();

    dwt_setpreambledetecttimeout(PRE_TIMEOUT);

	char dist_str[32] = {0};
	while (1) {
		dwt_setrxtimeout(0);
		dwt_setpreambledetecttimeout(0);
		dwt_rxenable(DWT_START_RX_IMMEDIATE);

		stdio_write("starting discovery\r\n");
		HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);

		if (receive_blink() != DWT_SUCCESS) {
			HAL_Delay(PDM_LOOP_DELAY);
			continue;
		}

		HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);
		stdio_write("starting ranging\r\n");
		HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);

		uint8_t numNoOps = 0;

		while (numNoOps < 50) {
			dwt_setrxtimeout(0);
			dwt_rxenable(DWT_START_RX_IMMEDIATE);

			double tmpDistance;
			if (try_ranging(&tmpDistance) != DWT_SUCCESS) {
				HAL_Delay(PDM_LOOP_DELAY);
				continue;
			}

			prevDistance = distance;
			distance = tmpDistance;

			/* Display computed distance. */
			snprintf(dist_str, 32, "DIST: %3.2f m\r\n", distance);
			stdio_write(dist_str);

			ecu_action_t action = check_ecu_action();

			if (action == NO_OP) {
				++numNoOps;
				HAL_Delay(PDM_LOOP_DELAY);
				continue;
			}

			perform_action_on_bike(action);

			HAL_Delay(PDM_LOOP_DELAY);
		}

		HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
		stdio_write("starting authentication\r\n");
		HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);

		// do auth here

		HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
		HAL_Delay(PDM_LOOP_DELAY);

		if (send_can_message() != CAN_OK) {
			stdio_write("canbad\r\n");
			continue;
		}
		stdio_write("cangood\r\n");
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

	dwt_setpreambledetecttimeout(PRE_TIMEOUT);

	return DWT_SUCCESS;
}

void setup_frame_filtering()
{
	uint16 pan_id = 0xDECA;
	uint16 short_addr = 0x3150; // 'P', '1' (little-endian)
	uint8 extended_addr[] = {'P', 'D', 'M', '0', '0', '0', '0', '1'};

    dwt_setpanid(pan_id);
    dwt_setaddress16(short_addr);
    dwt_seteui(extended_addr);

	dwt_enableframefilter(DWT_FF_BEACON_EN | DWT_FF_DATA_EN);
}

int receive_blink()
{
	// poll for blink message
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;
	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		return DWT_ERROR;
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		return DWT_ERROR;
	}

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

	memset(blink_rx_buffer, 0, sizeof(blink_rx_buffer));

	uint32_t frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	if (frameLen <= BLINK_RX_BUFFER_LEN) {
		dwt_readrxdata(blink_rx_buffer, frameLen, 0);
	}

	// zero out sequence number and FCS for comparison
	blink_rx_buffer[2] = 0;
	if (memcmp(blink_rx_buffer, blink_msg, BLINK_MSG_COMMON_LEN) != 0) {
		stdio_write("received non-blink message\r\n");
		return DWT_ERROR;
	}

    dwt_writetxdata(sizeof(ranging_init_msg), ranging_init_msg, 0);
    dwt_writetxfctrl(sizeof(ranging_init_msg), 0, 1);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
    	stdio_write("error: tx ranging init message failed.\r\n");
    	return DWT_ERROR;
    }

	return DWT_SUCCESS;
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
