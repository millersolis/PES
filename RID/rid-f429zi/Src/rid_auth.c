#include "rid_auth.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "deca_regs.h"
#include "dw_config.h"
#include "stdio_d.h"

#define AUTH_REPLY_MSG_PAYLOAD_INDEX 22

uint8_t auth_request_msg[24] = {
	0x41, 0x88,								// frame control; data frame, pan compression, 64-bit addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', '1',	// destination address; rid #1 extended identifier
	'P', '1',	// source address; pdm #1 extended identifier
	0x80,									// data; 0x80 is the auth request function code
	0, 0									// FCS; filled as CRC of the frame by hardware
};

uint8_t auth_reply_msg[56] = {
	0x41, 0xCC,								// frame control; data frame, pan compression, 64-bit addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'P', 'D', 'M', '0', '0', '0', '0', '1',	// destination address; pdm #1 extended identifier
	'R', 'I', 'D', '0', '0', '0', '0', '1',	// source address; rid #1 extended identifier
	0x81,									// data; 0x81 is auth reply function code with 32 bytes for encrypted payload
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0									// FCS; filled as CRC of the frame by hardware
};

uint8_t auth_ack_msg[56] = {
	0x41, 0xCC,								// frame control; data frame, pan compression, 64-bit addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', 'I', 'D', '0', '0', '0', '0', '1',	// destination address; rid #1 extended identifier
	'P', 'D', 'M', '0', '0', '0', '0', '1',	// source address; pdm #1 extended identifier
	0x82, 									// data; 0x82 is auth ack function code with 32 bytes for encrypted payload
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0									// FCS; filled as CRC of the frame by hardware
};

static uint8_t frame_seq_nb = 0;

bool is_auth_request_msg(uint8_t* rx_buffer)
{
	rx_buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	auth_request_msg[ALL_MSG_SN_IDX] = 0;
	return (memcmp(rx_buffer, auth_request_msg, AUTH_FRAME_COMMON_LEN) == 0);
}

bool is_auth_ack_msg(uint8_t buffer[RX_BUF_LEN])
{
	buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	auth_ack_msg[ALL_MSG_SN_IDX] = 0;
	return memcmp(buffer, auth_ack_msg, AUTH_FRAME_COMMON_LEN) == 0;
}

receive_status_t receive_auth_request(uint8_t buffer[RX_BUF_LEN])
{
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);

		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		dwt_rxreset();
		return STATUS_RECEIVE_TIMEOUT;
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);

		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		dwt_rxreset();
		return STATUS_RECEIVE_ERROR;
	}

	uint32 frame_len;

	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

	/* A frame has been received, read it into the local buffer. */
	frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
	if (frame_len <= RX_BUF_LEN) {
		dwt_readrxdata(buffer, frame_len, 0);
	}

	return STATUS_RECEIVE_OK;
}

send_status_t send_auth_reply(uint8_t data[16], uint8_t iv[16])
{
	memcpy(&auth_reply_msg[AUTH_REPLY_MSG_PAYLOAD_INDEX], &data[0], 16);
	memcpy(&auth_reply_msg[AUTH_REPLY_MSG_PAYLOAD_INDEX + 16], &iv[0], 16);

	auth_reply_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
	dwt_writetxdata(sizeof(auth_reply_msg), auth_reply_msg, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(auth_reply_msg), 0, 0); /* Zero offset in TX buffer, ranging. */

	if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
		stdio_write("tx auth reply failed\r\n");
		return STATUS_SEND_ERROR;
	}

	++frame_seq_nb;

	return STATUS_SEND_OK;
}

receive_status_t receive_auth_ack(uint8_t buffer[RX_BUF_LEN])
{
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);

		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		dwt_rxreset();
		return STATUS_RECEIVE_TIMEOUT;
	}
	else if ((statusReg & SYS_STATUS_ALL_RX_ERR) != 0) {
		print_status_errors(statusReg);

		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		dwt_rxreset();
		return STATUS_RECEIVE_ERROR;
	}

	uint32 frame_len;

	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

	/* A frame has been received, read it into the local buffer. */
	frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
	if (frame_len <= RX_BUF_LEN) {
		dwt_readrxdata(buffer, frame_len, 0);
	}

	return STATUS_RECEIVE_OK;
}

void print_auth_request_msg(uint8_t* rx_buffer)
{
	if (is_auth_request_msg(rx_buffer))
	{
		char str[200] = { '\0' };

		// Parse sequence number
		char seq_num_str[2] = { '\0' };
		sprintf(seq_num_str, "%i", rx_buffer[ALL_MSG_SN_IDX]);

		// Parse source ID
		char source_id_str[ADDRESS_LEN + 1] = { '\0' };
		for (int i = 0 ; i < ADDRESS_LEN; i++)
		{
			source_id_str[i] = (char)rx_buffer[i + AUTH_FRAME_SRC_IDX];
		}

		// Format and print
		strcat(str, "AUTH REQ = ");
		strcat(str, "Seq Num: ");
		strcat(str, seq_num_str);
		strcat(str, ", Src ID: ");
		strcat(str, source_id_str);
		strcat(str, "\r\n");

		stdio_write(str);
	}
	else
	{
		stdio_write("ERROR: Message is not of auth request type. Cannot print its contents.");
	}
}
