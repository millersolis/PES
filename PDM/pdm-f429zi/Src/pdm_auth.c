#include "pdm_auth.h"

#include <string.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "stdio_d.h"

#define AUTH_ACK_MSG_PAYLOAD_INDEX 22

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

bool is_auth_reply_msg(uint8_t buffer[RX_BUF_LEN])
{
	buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	return memcmp(buffer, auth_reply_msg, AUTH_FRAME_COMMON_LEN) == 0;
}

send_status_t send_auth_request()
{
    dwt_writetxdata(sizeof(auth_request_msg), auth_request_msg, 0);
    dwt_writetxfctrl(sizeof(auth_request_msg), 0, 1);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
    	stdio_write("error: tx auth request message failed.\r\n");
    	return STATUS_SEND_ERROR;
    }

	/* Poll DW1000 until TX frame sent event set. */
	while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
	{ };

	/* Increment frame sequence number after transmission of the response message (modulo 256). */
	frame_seq_nb++;

	return STATUS_SEND_OK;
}

receive_status_t receive_auth_reply(uint8_t buffer[RX_BUF_LEN])
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

send_status_t send_auth_ack(uint8_t data[16], uint8_t iv[16])
{
	memcpy(&auth_ack_msg[AUTH_ACK_MSG_PAYLOAD_INDEX], &data[0], 16);
	memcpy(&auth_ack_msg[AUTH_ACK_MSG_PAYLOAD_INDEX + 16], &iv[0], 16);

	auth_ack_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    dwt_writetxdata(sizeof(auth_ack_msg), auth_ack_msg, 0);
    dwt_writetxfctrl(sizeof(auth_ack_msg), 0, 1);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
    	stdio_write("error: tx auth ack message failed.\r\n");
    	return STATUS_SEND_ERROR;
    }

	/* Poll DW1000 until TX frame sent event set. */
	while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
	{ };

	/* Increment frame sequence number after transmission of the response message (modulo 256). */
	frame_seq_nb++;

	return STATUS_SEND_OK;
}
