#include "pdm.h"

#include <stdio.h>
#include "aes.h"
#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "main.h"
#include "pdm_can.h"
#include "pdm_auth.h"
#include "port.h"
#include "ranging.h"
#include "stdio_d.h"

#define PDM_LOOP_DELAY 10

static state_t state;
static uint8_t pdm_rx_buffer[RX_BUF_LEN] = { 0 };
static struct AES_ctx aes_context;
static uint8_t data[16] = { 0 };
static uint8_t iv[16] = { 0 };
static uint8_t symmetric_key[16] = {
	'a', 'n', 'e', 'x', 't', 'r', 'a',
	'b', 'o', 'r', 'i', 'n', 'g', 'k', 'e', 'y'
};
static uint64_t rolling_code = 0xB632F836BF96F22C;

typedef enum {
	NO_OP,
	WAKEUP,
	LOCK
} ecu_action_t;

void update_state(state_t* state, const pdm_state_t value);
void print_state_if_changed(const state_t* state, const char str[32]);
void clear_and_set_led(uint16_t gpioPin);

int init_dw1000();
void setup_frame_filtering();
pdm_state_t receive_blink();
pdm_state_t perform_ranging();
pdm_state_t perform_authentication();
ecu_action_t check_ecu_action();
void perform_action_on_bike(const ecu_action_t action);

static uint8_t numNoOps = 0;
static double distance = 0.0;
static double prevDistance = 0.0;
static ecu_action_t nextAction =  NO_OP;

void pdm_main()
{
	stdio_write("\r\nstarting PDM\r\n");

	AES_init_ctx(&aes_context, &symmetric_key[0]);

	init_can(&hcan1);

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

    setup_frame_filtering();

    dwt_setpreambledetecttimeout(PRE_TIMEOUT);

    while (1) {
    	switch(state.value) {
    		case STATE_DISCOVERY:
				print_state_if_changed(&state, "\r\nIn DISCOVERY\r\n");
				clear_and_set_led(LD1_Pin);

				// set pdm to idle and wait for next blink message
    			dwt_setrxtimeout(0);
    			dwt_setpreambledetecttimeout(0);
    			dwt_rxenable(DWT_START_RX_IMMEDIATE);

    			update_state(&state, receive_blink());
    			break;

    		case STATE_RANGING:
				print_state_if_changed(&state, "\r\nIn RANGING\r\n");
				clear_and_set_led(LD2_Pin);

				if (state.hasChanged == true) {
					numNoOps = 0;
				}

				update_state(&state, perform_ranging());
				HAL_Delay(PDM_LOOP_DELAY);
    			break;

    		case STATE_AUTHENTICATION:
    			print_state_if_changed(&state, "\r\nIn AUTHENTICATION\r\n");
    			clear_and_set_led(LD3_Pin);

    			update_state(&state, perform_authentication());
    			HAL_Delay(1000);
    			break;
    	}
    }
}

void update_state(state_t* state, const pdm_state_t value)
{
	if (state->value == value) {
		state->hasChanged = 0;
		return;
	}

	state->value = value;
	state->hasChanged = true;
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

pdm_state_t receive_blink()
{
	// poll for blink message
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		return STATE_DISCOVERY;
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		return STATE_DISCOVERY;
	}

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

	memset(pdm_rx_buffer, 0, sizeof(pdm_rx_buffer));

	uint32_t frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(pdm_rx_buffer, frameLen, 0);
	}

	// zero out sequence number and FCS for comparison
	pdm_rx_buffer[2] = 0;
	if (memcmp(pdm_rx_buffer, blink_msg, BLINK_MSG_COMMON_LEN) != 0) {
		stdio_write("received non-blink message\r\n");
		return STATE_DISCOVERY;
	}

	if (send_ranging_init_msg() != STATUS_SEND_OK) {
		stdio_write("failed to send ranging init\r\n");
		return STATE_DISCOVERY;
	}

	return STATE_RANGING;
}

pdm_state_t perform_ranging()
{
	if (send_ranging_init_msg() != STATUS_SEND_OK) {
		stdio_write("failed to send ranging init\r\n");
		return STATE_RANGING;
	}

	dwt_setrxtimeout(0);
	dwt_setpreambledetecttimeout(100);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	if (receive_poll_msg(pdm_rx_buffer) != STATUS_RECEIVE_OK) {
		return STATE_RANGING;
	}

	if (is_poll_msg(pdm_rx_buffer) == false) {
		stdio_write("received non-poll message\r\n");
		return STATE_RANGING;
	}

	if (send_response_msg() != STATUS_SEND_OK) {
		return STATE_RANGING;
	}

	if (receive_final_msg(pdm_rx_buffer) != STATUS_RECEIVE_OK) {
		return STATE_RANGING;
	}

	if (is_final_msg(pdm_rx_buffer) == false) {
		stdio_write("received non-final message\r\n");
		return STATE_RANGING;
	}

	distance = retrieve_ranging_result(pdm_rx_buffer);

	/* Display computed distance. */
	char dist_str[32] = {0};
	snprintf(dist_str, 32, "DIST: %3.2f m\r\n", distance);
	stdio_write(dist_str);

	nextAction = check_ecu_action();

	if (nextAction == NO_OP) {
		++(numNoOps);
	}

	if (numNoOps < 10) {
		return STATE_RANGING;
	}

	nextAction = WAKEUP;
	perform_action_on_bike(nextAction);

	return STATE_AUTHENTICATION;
}

pdm_state_t perform_authentication()
{
	if (send_auth_request() != STATUS_SEND_OK) {
		return STATE_AUTHENTICATION;
	}

	dwt_setpreambledetecttimeout(100);
	dwt_setrxtimeout(0);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	if (receive_auth_reply(pdm_rx_buffer) != STATUS_RECEIVE_OK) {
		return STATE_AUTHENTICATION;
	}

	if (is_auth_reply_msg(pdm_rx_buffer) == false) {
		stdio_write("received non-auth-reply message\r\n");
		return STATE_AUTHENTICATION;
	}

	memcpy(&data[0], &pdm_rx_buffer[AUTH_FRAME_COMMON_LEN], 16);
	memcpy(&iv[0], &pdm_rx_buffer[AUTH_FRAME_COMMON_LEN+16], 16);
	AES_ctx_set_iv(&aes_context, &iv[0]);
	AES_CBC_decrypt_buffer(&aes_context, &data[0], 16);

	uint64_t receivedRollingCode = 0;
	memcpy((uint8_t*) &receivedRollingCode, &data[0], sizeof(uint64_t));

	if ((receivedRollingCode - rolling_code) >= 100) {
		stdio_write("received bad rolling code\r\n");
		return STATE_DISCOVERY;
	}

	rolling_code = receivedRollingCode;

	// seed initial vector for AES128 CBC encryption
	for (int index = 0; index < 4; ++index) {
		HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*) &iv[4*index]);
	}

	// zero out the full 16 bytes, then add the action's enum value for encryption
	memset(&data[0], 0, 16);
	memcpy(&data[0], (uint8_t*) &nextAction, sizeof(uint8_t));

	// set initial vector for AES128 CBC encryption, then encrypt the data
	AES_ctx_set_iv(&aes_context, &iv[0]);
	AES_CBC_encrypt_buffer(&aes_context, &data[0], 16);

	if (send_auth_ack(data, iv) != STATUS_SEND_OK) {
		return STATE_AUTHENTICATION;
	}

	HAL_Delay(PDM_LOOP_DELAY);
	if (send_can_message() != CAN_OK) {
		stdio_write("canbad\r\n");
		return STATE_AUTHENTICATION;
	}
	stdio_write("cangood\r\n");

	return STATE_DISCOVERY;
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
