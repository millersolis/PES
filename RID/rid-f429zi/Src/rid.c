#include "rid.h"

#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "main.h"
#include "port.h"
#include "ranging.h"
#include "rid_auth.h"
#include "stdio.h"
#include "stdio_d.h"

static state_t state;
static uint8_t rid_rx_buffer[RX_BUF_LEN] = { 0 };

uint8_t blink_msg[24] = {
	0x41, 0xCC,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', 'D', 'M', '0', '0', '0', '0', '1',	// destination address; pdm extended identifier
	'R', 'I', 'D', '0', '0', '0', '0', '1',	// source address; rid extended identifier
	0x21,									// data; 0x21 is function code for blink
	0, 0									// FCS; filled as CRC of the frame by hardware
};

void update_state(state_t* state, const rid_state_t value);
void print_state_if_changed(const state_t* state, const char str[32]);
void clear_and_set_led(uint16_t gpioPin);

int init_dw1000();
void setup_frame_filtering();
rid_state_t perform_blink();
rid_state_t perform_ranging();

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
		switch (state.value) {
			case STATE_DISCOVERY:
				print_state_if_changed(&state, "\r\nIn DISCOVERY\r\n");
				clear_and_set_led(LD1_Pin);

				dwt_setrxaftertxdelay(800);
				dwt_setrxtimeout(0xffff);

				update_state(&state, perform_blink());
				HAL_Delay(100);
				break;

			case STATE_RANGING:
				print_state_if_changed(&state, "\r\nIn RANGING\r\n");
				clear_and_set_led(LD2_Pin);

				dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
				dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

				update_state(&state, perform_ranging());
				HAL_Delay(MIN_DELAY_RANGING);
				break;

			case STATE_AUTH_REPLY:
				break;
		}
	}
}

void update_state(state_t* state, const rid_state_t value)
{
	if (state->value == value) {
		state->hasChanged = 0;
		return;
	}

	state->value = value;
	state->hasChanged = true;

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}
}

void print_state_if_changed(const state_t* state, const char str[32])
{
	// if state is unchanged, don't print a new state value
	if (state->hasChanged == false) {
		return;
	}

	stdio_write(str);
}

void clear_and_set_led(uint16_t gpioPin)
{
	HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOB, gpioPin, GPIO_PIN_SET);
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
    dwt_writetxdata(sizeof(blink_msg), blink_msg, 0);
    dwt_writetxfctrl(sizeof(blink_msg), 0, 1);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS) {
    	stdio_write("error: tx blink message failed.\r\n");
    	return STATE_DISCOVERY;
    }

	// poll for ranging init
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;
	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	memset(rid_rx_buffer, 0, sizeof(rid_rx_buffer));

	uint32_t frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(rid_rx_buffer, frameLen, 0);
	}

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);
        dwt_write32bitreg(SYS_STATUS_ID, msgReceivedFlags);
		return STATE_DISCOVERY;
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);
        dwt_write32bitreg(SYS_STATUS_ID, msgReceivedFlags);
		return STATE_DISCOVERY;
	}

	memset(rid_rx_buffer, 0, sizeof(rid_rx_buffer));

	frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(rid_rx_buffer, frameLen, 0);
	}

	rid_rx_buffer[2] = 0;
	if (memcmp(rid_rx_buffer, ranging_init_msg, RANGING_INIT_MSG_COMMON_LEN) != 0) {
		stdio_write("received non-ranging-init message\r\n");
		return STATE_DISCOVERY;
	}

    return STATE_RANGING;
}

rid_state_t perform_ranging()
{
	if (send_poll_msg() != STATUS_SEND_OK) {
		return STATE_RANGING;
	}

	receive_status_t recResult = receive_response_msg(rid_rx_buffer);
	if (recResult == STATUS_RECEIVE_TIMEOUT) {
		return STATE_RANGING;
	}
	else if (recResult == STATUS_RECEIVE_ERROR) {
		return STATE_RANGING;
	}

	if (is_ranging_init_msg(rid_rx_buffer)) {
		stdio_write("received ranging init message\r\n");
		return STATE_RANGING;
	}
	else if (is_auth_request_msg(rid_rx_buffer)) {
		stdio_write("received auth request message\r\n");
		return STATE_AUTH_REPLY;
	}

	if (send_final_msg() != STATUS_SEND_OK) {
		stdio_write("failed to send ranging final message\r\n");
		return STATE_RANGING;
	}

	return STATE_RANGING;
}
