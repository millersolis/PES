#include "ranging.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "port.h"
#include "stdio_d.h"

uint8_t ranging_init_msg[18] = {
	0x41, 0x8C,								// frame control; beacon, pan compression, and long addresses
	0,										// sequence number; filled by sender
	0xCA, 0xDE,								// PAN ID; default to 0xDECA
	'R', 'I', 'D', '0', '0', '0', '0', '1', // destination address; pdm extended identifier
	'P', '1',								// source address; pdm short identifier
	0x20,									// data; 0x20 is ranging init function code
	0, 0									// FCS; filled as CRC of the frame by hardware
};

static uint64_t get_tx_timestamp_u64();
static uint64_t get_rx_timestamp_u64();
static void final_msg_get_ts(const uint8_t *ts_field, uint32_t *ts);

static uint8_t frame_seq_nb = 0;

static uint64_t poll_rx_ts;

bool is_poll_msg(uint8_t* buffer)
{
	buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	return (memcmp(buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0);
}

bool is_final_msg(uint8_t* buffer)
{
	buffer[ALL_MSG_SN_IDX] = 0;	// Make received frame count 0 to compare
	return (memcmp(buffer, rx_final_msg, ALL_MSG_COMMON_LEN) == 0);
}

send_status_t send_ranging_init_msg()
{
    dwt_writetxdata(sizeof(ranging_init_msg), ranging_init_msg, 0);
    dwt_writetxfctrl(sizeof(ranging_init_msg), 0, 1);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
    	stdio_write("error: tx ranging init message failed.\r\n");
    	return STATUS_SEND_ERROR;
    }

    return STATUS_SEND_OK;
}

receive_status_t receive_poll_msg(uint8_t buffer[RX_BUF_LEN])
{
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags)) {}

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);
		stdio_write("error: rx poll message failed.\r\n");

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

	uint32 frameLen;

	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

	/* A frame has been received, read it into the local buffer. */
	frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(buffer, frameLen, 0);
	}

	return STATUS_RECEIVE_OK;
}

send_status_t send_response_msg()
{
	/* Retrieve poll reception timestamp. */
	poll_rx_ts = get_rx_timestamp_u64();

	/* Set send time for response. See NOTE 9 below. */
	uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
	dwt_setdelayedtrxtime(resp_tx_time);

	/* Set expected delay and timeout for final message reception. See NOTE 4 and 5 below. */
	dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
	dwt_setrxtimeout(FINAL_RX_TIMEOUT_UUS);

	/* Write and send the response message. See NOTE 10 below.*/
	tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
	dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 0); /* Zero offset in TX buffer, ranging. */
	if (dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS) {
		return STATUS_SEND_ERROR;
	}

	/* Poll DW1000 until TX frame sent event set. */
	while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
	{ };

	/* Increment frame sequence number after transmission of the response message (modulo 256). */
	frame_seq_nb++;

	return STATUS_SEND_OK;
}

receive_status_t receive_final_msg(uint8_t buffer[RX_BUF_LEN])
{
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags));

	if ((statusReg & SYS_STATUS_ALL_RX_TO) != 0) {
		print_timeout_errors(statusReg);
		stdio_write("error: rx ranging final message failed.\r\n");

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

	uint32 frameLen;

	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

	/* A frame has been received, read it into the local buffer. */
	frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(buffer, frameLen, 0);
	}

	return STATUS_RECEIVE_OK;
}

double retrieve_ranging_result(uint8_t buffer[RX_BUF_LEN])
{
	uint32 poll_tx_ts, resp_rx_ts, final_tx_ts;
	uint32 poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
	uint64 resp_tx_ts, final_rx_ts;
	double Ra, Rb, Da, Db;
	int64 tof_dtu;

	/* Retrieve response transmission and final reception timestamps. */
	resp_tx_ts = get_tx_timestamp_u64();
	final_rx_ts = get_rx_timestamp_u64();

	/* Get timestamps embedded in the final message. */
	final_msg_get_ts(&buffer[FINAL_MSG_POLL_TX_TS_IDX], &poll_tx_ts);
	final_msg_get_ts(&buffer[FINAL_MSG_RESP_RX_TS_IDX], &resp_rx_ts);
	final_msg_get_ts(&buffer[FINAL_MSG_FINAL_TX_TS_IDX], &final_tx_ts);

	/* Compute time of flight. 32-bit subtractions give correct answers even if clock has wrapped. See NOTE 12 below. */
	poll_rx_ts_32 = (uint32)poll_rx_ts;
	resp_tx_ts_32 = (uint32)resp_tx_ts;
	final_rx_ts_32 = (uint32)final_rx_ts;
	Ra = (double)(resp_rx_ts - poll_tx_ts);
	Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);
	Da = (double)(final_tx_ts - resp_rx_ts);
	Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);
	tof_dtu = (int64)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

	double tof = tof_dtu * DWT_TIME_UNITS;
	return tof * SPEED_OF_LIGHT;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_tx_timestamp_u64()
 *
 * @brief Get the TX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
static uint64_t get_tx_timestamp_u64(void)
{
    uint8_t ts_tab[5];
    uint64_t ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_rx_timestamp_u64()
 *
 * @brief Get the RX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
static uint64_t get_rx_timestamp_u64(void)
{
    uint8_t ts_tab[5];
    uint64_t ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn final_msg_get_ts()
 *
 * @brief Read a given timestamp value from the final message. In the timestamp fields of the final message, the least
 *        significant byte is at the lower address.
 *
 * @param  ts_field  pointer on the first byte of the timestamp field to read
 *         ts  timestamp value
 *
 * @return none
 */
static void final_msg_get_ts(const uint8_t *ts_field, uint32_t *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < FINAL_MSG_TS_LEN; i++)
    {
        *ts += ts_field[i] << (i * 8);
    }
}
