#include "dw_utils.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "dw_config.h"
#include "dw_helpers.h"
#include "port.h"
#include "stdio_d.h"

receive_status_t receive_msg(uint8_t buffer[RX_BUF_LEN])
{
	uint32_t statusReg = 0;
	uint32_t msgReceivedFlags = SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR;

	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & msgReceivedFlags)) {}

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

	uint32 frameLen;

	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

	/* A frame has been received, read it into the local buffer. */
	frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
//	frameLen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;

	memset(buffer, 0, RX_BUF_LEN);
	if (frameLen <= RX_BUF_LEN) {
		dwt_readrxdata(buffer, frameLen, 0);
	}

	return STATUS_RECEIVE_OK;
}
