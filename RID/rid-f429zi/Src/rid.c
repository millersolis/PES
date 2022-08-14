#include "rid.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "aes.h"
#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "dw_utils.h"
#include "main.h"
#include "port.h"
#include "ranging.h"
#include "rid_auth.h"
#include "stdio_d.h"

static state_t state;
static uint8_t rid_rx_buffer[RX_BUF_LEN] = { 0 };
static struct AES_ctx aes_context;
static uint8_t data[16] = { 0 };
static uint8_t iv[16] = { 0 };
static uint8_t symmetric_key[16] = {
	'a', 'n', 'e', 'x', 't', 'r', 'a',
	'b', 'o', 'r', 'i', 'n', 'g', 'k', 'e', 'y'
};
static uint64_t rolling_code = 0xB632F836BF96F22C;
static uint8_t rng_init_rx_retry_counter = 0;

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
rid_state_t perform_authentication();

void rid_main()
{
	stdio_write("\r\nstarting RID\r\n");

	AES_init_ctx(&aes_context, &symmetric_key[0]);

	if (init_dw1000() != DWT_SUCCESS) {
		stdio_write("initializing dw1000 failed; spinlocking.\r\n");
		while (1);
	}

	while (1) {
		switch (state.value) {
			case STATE_DISCOVERY:
				print_state_if_changed(&state, "\r\nIn DISCOVERY\r\n");
				clear_and_set_led(LD1_Pin);

				dwt_setrxaftertxdelay(800);
				dwt_setrxtimeout(0xffff);
				dwt_setpreambledetecttimeout(RANGING_PREAMBLE_TIMEOUT);

				update_state(&state, perform_blink());
				HAL_Delay(100);
				break;

			case STATE_RANGING:
				print_state_if_changed(&state, "\r\nIn RANGING\r\n");
				clear_and_set_led(LD2_Pin);

				update_state(&state, perform_ranging());
//				HAL_Delay(MIN_DELAY_RANGING);
				break;

			case STATE_AUTHENTICATION:
				print_state_if_changed(&state, "\r\nIn AUTHENTICATION\r\n");
				clear_and_set_led(LD3_Pin);

				update_state(&state, perform_authentication());
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

	setup_frame_filtering();

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

    if (receive_ranging_init_msg(rid_rx_buffer) != STATUS_RECEIVE_OK) {
    	return STATE_DISCOVERY;
    }

    if (is_ranging_init_msg(rid_rx_buffer) == false) {
		stdio_write("received non-ranging-init message\r\n");
		return STATE_DISCOVERY;
    }

    return STATE_RANGING;
}

bool retry_ranging_init_rx() {
	if (rng_init_rx_retry_counter < RNG_INIT_MAX_RX_RETRY) {
		++rng_init_rx_retry_counter;
		return true;
	}

	rng_init_rx_retry_counter = 0;
	return false;
}

rid_state_t perform_ranging()
{
	dwt_setrxtimeout(0);
	dwt_setpreambledetecttimeout(0xf000);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	// Receive Ranging Init
//	receive_status_t rx_status = receive_msg(rid_rx_buffer);
//	switch (rx_status) {
//
//		case STATUS_RECEIVE_OK:
//			break;
//
//		case STATUS_RECEIVE_ERROR:
//			stdio_write("DEBUG: error on rx rng init message\r\n");
//			return STATE_RANGING;
//
//		case STATUS_RECEIVE_TIMEOUT:
//			stdio_write("DEBUG: timeout on rx rng init message\r\n");
//			if (retry_ranging_init_rx()) {
//				return STATE_RANGING;
//			}
//			return STATE_DISCOVERY;
//	}

	if (receive_ranging_init_msg(rid_rx_buffer) != STATUS_RECEIVE_OK) {
		return STATE_RANGING;
	}


	if (is_auth_request_msg(rid_rx_buffer)) {
		stdio_write("received auth request message\r\n");
		return STATE_AUTHENTICATION;
	}
	else if (!is_ranging_init_msg(rid_rx_buffer)) {
		stdio_write("received non-ranging-init message\r\n");
		return STATE_RANGING;
	}

	// manually force back to idle mode so the rest of ranging works
	dwt_forcetrxoff();

	dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
	dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

	if (send_poll_msg() != STATUS_SEND_OK) {
		return STATE_RANGING;
	}

	receive_status_t recResult = receive_response_msg(rid_rx_buffer);
	if (recResult != STATUS_RECEIVE_OK) {
		return STATE_RANGING;
	}

	if (is_ranging_init_msg(rid_rx_buffer)) {
		stdio_write("received ranging init message\r\n");
		return STATE_RANGING;
	}
	else if (is_auth_request_msg(rid_rx_buffer)) {
		stdio_write("received auth request message\r\n");
		return STATE_AUTHENTICATION;
	}

	if (send_final_msg() != STATUS_SEND_OK) {
		stdio_write("failed to send ranging final message\r\n");
		return STATE_RANGING;
	}

	return STATE_RANGING;
}

rid_state_t perform_authentication()
{
	dwt_setpreambledetecttimeout(5000);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	if (receive_auth_request(rid_rx_buffer) != STATUS_RECEIVE_OK) {
		return STATE_AUTHENTICATION;
	}

	if (is_auth_request_msg(rid_rx_buffer) == false) {
		stdio_write("received non-auth-request message\r\n");
		return STATE_AUTHENTICATION;
	}

	// seed initial vector for AES128 CBC encryption
	for (int index = 0; index < 4; ++index) {
		HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*) &iv[4*index]);
	}

	// set lower 8 bytes to the rolling code to encrypt, and also
	// zero out the upper 8 bytes
	memcpy(&data[0], (uint8_t*) &rolling_code, sizeof(uint64_t));
	memset(&data[8], 0, 8);

	// set initial vector for AES128 CBC encryption, then encrypt the data
	AES_ctx_set_iv(&aes_context, &iv[0]);
	AES_CBC_encrypt_buffer(&aes_context, &data[0], 16);

	if (send_auth_reply(data, iv) != STATUS_SEND_OK) {
		return STATE_AUTHENTICATION;
	}

	dwt_setpreambledetecttimeout(0x4000);
	dwt_setrxtimeout(0);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	if (receive_auth_ack(rid_rx_buffer) != STATUS_RECEIVE_OK) {
		return STATE_AUTHENTICATION;
	}

	if (is_auth_ack_msg(rid_rx_buffer) == false) {
		stdio_write("received non-auth-ack msg\r\n");
		return STATE_AUTHENTICATION;
	}

	// copy out the data and initial vector, then decrypt the data
	memcpy(&data[0], &rid_rx_buffer[AUTH_FRAME_COMMON_LEN], 16);
	memcpy(&iv[0], &rid_rx_buffer[AUTH_FRAME_COMMON_LEN+16], 16);
	AES_ctx_set_iv(&aes_context, &iv[0]);
	AES_CBC_decrypt_buffer(&aes_context, &data[0], 16);

	uint8_t action = data[0];

	// if decrypted action is not a no-op, party time
	if (action != 0) {
		asm("nop");
	}

	++rolling_code;
	return STATE_DISCOVERY;
}
