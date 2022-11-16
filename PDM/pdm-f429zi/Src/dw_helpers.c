#include "dw_helpers.h"

#include <stdio.h>
#include <string.h>
#include "deca_regs.h"
#include "dw_config.h"
#include "stdio_d.h"

void print_timeout_errors(uint32_t statusRegister)
{
	if ((statusRegister & SYS_STATUS_RXRFTO) != 0) {
		stdio_write("RX frame wait timeout event detected.\r\n");
	}
	if ((statusRegister & SYS_STATUS_RXPTO) != 0) {
		stdio_write("RX preamble detection timeout event detected.\r\n");
	}
}

void print_status_errors(uint32_t statusRegister)
{
	if (statusRegister & SYS_STATUS_RXPHE) {
		stdio_write("RX PHY header error event detected.\r\n");
	}
	if (statusRegister & SYS_STATUS_RXFCE){
		stdio_write("RX FCS error event detected.\r\n");
	}
	if (statusRegister & SYS_STATUS_RXRFSL) {
		stdio_write("RX Reed-Solomon frame sync loss event detected.\r\n");
	}
	if (statusRegister & SYS_STATUS_RXSFDTO) {
		stdio_write("RX SFD timeout event detected.\r\n");
	}
	if (statusRegister & SYS_STATUS_AFFREJ) {
		stdio_write("RX automatic frame filtering rejection event detected.\r\n");
	}
	if (statusRegister & SYS_STATUS_LDEERR) {
		stdio_write("RX LDE error event detected.\r\n");
	}
}

uint16_t format_frame_control(frame_control_config_t config)
{
	uint16_t frameControl = 0;

	switch (config.type) {
	case FRAME_TYPE_BEACON: 		frameControl |= FCF_FRAME_TYPE_BEACON; 		break;
	case FRAME_TYPE_DATA:   		frameControl |= FCF_FRAME_TYPE_DATA;   		break;
	case FRAME_TYPE_ACK:    		frameControl |= FCF_FRAME_TYPE_ACK;   		break;
	case FRAME_TYPE_MAC_COMMAND:	frameControl |= FCF_FRAME_TYPE_MAC_COMMAND; break;
	}

	if (config.securityEnabled != 0) {
		frameControl |= FCF_SECURITY_ENABLED;
	}

	if (config.framePending != 0) {
		frameControl |= FCF_FRAME_PENDING;
	}

	if (config.ackRequest != 0) {
		frameControl |= FCF_ACK_REQUEST;
	}

	if (config.panIdCompress != 0) {
		frameControl |= FCF_PAN_ID_COMPRESS;
	}

	switch (config.destAddr) {
	case FRAME_ADDR_ABSENT:		frameControl |= FCF_DEST_ADDR_ABSENT;   break;
	case FRAME_ADDR_SHORT:		frameControl |= FCF_DEST_ADDR_SHORT;    break;
	case FRAME_ADDR_EXTENDED:	frameControl |= FCF_DEST_ADDR_EXTENDED; break;
	}

	switch(config.frameVersion) {
	case FRAME_V_2003:   frameControl |= FCF_FRAME_VERSION_2003;   break;
	case FRAME_V_MODERN: frameControl |= FCF_FRAME_VERSION_MODERN; break;
	}

	switch(config.srcAddr) {
	case FRAME_ADDR_ABSENT:		frameControl |= FCF_SRC_ADDR_ABSENT;   break;
	case FRAME_ADDR_SHORT:		frameControl |= FCF_SRC_ADDR_SHORT;    break;
	case FRAME_ADDR_EXTENDED:	frameControl |= FCF_SRC_ADDR_EXTENDED; break;
	}

	return frameControl;
}

// returns length written to buffer
uint16_t format_frame_header(uint8_t buffer[128], frame_control_config_t fcConfig, frame_addr_config_t addrConfig)
{
	uint16_t frameControl = format_frame_control(fcConfig);
	buffer[0] = (uint8_t) (frameControl >> 8);
	buffer[1] = (uint8_t) (frameControl);

	// ignore buffer[2], caller manages their own sequence numbers

	uint16_t index = 3;

	// if PAN ID compression is off, there are two cases, based on whether the dest addr is present:
	//     present: first PAN ID will be the destination PAN ID
	//     absent:  first PAN ID will be the either source PAN ID or nothing, which is handled later
	if (fcConfig.panIdCompress == 0) {
		if (fcConfig.destAddr != FRAME_ADDR_ABSENT) {
			buffer[index++] = (uint8_t) (addrConfig.destPanId);
			buffer[index++] = (uint8_t) (addrConfig.destPanId >> 8);
		}
	}
	// if PAN ID compression is on, the first PAN ID is always the destination PAN ID
	else {
		buffer[index++] = (uint8_t) (addrConfig.destPanId);
		buffer[index++] = (uint8_t) (addrConfig.destPanId >> 8);
	}

	if (fcConfig.destAddr == FRAME_ADDR_SHORT) {
		buffer[index++] = (uint8_t) (addrConfig.destAddr);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 8);
	}
	else if (fcConfig.destAddr == FRAME_ADDR_EXTENDED) {
		buffer[index++] = (uint8_t) (addrConfig.destAddr);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 8);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 16);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 24);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 32);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 40);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 48);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 54);
	}

	if (fcConfig.panIdCompress == 0 && fcConfig.srcAddr != FRAME_ADDR_ABSENT) {
		buffer[index++] = (uint8_t) (addrConfig.srcPanId);
		buffer[index++] = (uint8_t) (addrConfig.srcPanId >> 8);
	}

	if (fcConfig.destAddr == FRAME_ADDR_SHORT) {
		buffer[index++] = (uint8_t) (addrConfig.destAddr);
		buffer[index++] = (uint8_t) (addrConfig.destAddr >> 8);
	}
	else if (fcConfig.srcAddr == FRAME_ADDR_EXTENDED) {
		buffer[index++] = (uint8_t) (addrConfig.srcAddr);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 8);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 16);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 24);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 32);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 40);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 48);
		buffer[index++] = (uint8_t) (addrConfig.srcAddr >> 54);
	}

	// allow caller to append their own payload and the FCS/CRC field
	return index;
}

static char print_buf[2*RX_BUF_LEN + 1] = { 0 };
void print_bytes(uint8_t* data, size_t length)
{
	memset(print_buf, 0, 2*RX_BUF_LEN + 1);
	for (int index = 0; index < length; ++index) {
		snprintf(&print_buf[2*index], 3, "%02x", data[index]);
	}
	print_buf[2*length + 1] = '\0';
	stdio_write(print_buf);
}

void print_status_register()
{
	uint32_t statusRegister = dwt_read32bitreg(SYS_STATUS_ID);
	stdio_write("status:     0x");
	print_bytes(&statusRegister, 4);

	uint32_t stateRegister = dwt_read32bitreg(SYS_STATE_ID);
	stdio_write("    state:      0x");
	print_bytes(&stateRegister, 4);

	uint32_t sysConfigRegister = dwt_read32bitreg(SYS_CFG_ID);
	stdio_write("    system config: 0x");
	print_bytes(&sysConfigRegister, 4);

	stdio_write("\r\n");

	uint32_t pmscCtrl0Register = dwt_read32bitoffsetreg(PMSC_ID, PMSC_CTRL0_OFFSET);
	stdio_write("pmsc ctrl0: 0x");
	print_bytes(&pmscCtrl0Register, 4);

	uint32_t pmscCtrl1Register = dwt_read32bitoffsetreg(PMSC_ID, PMSC_CTRL1_OFFSET);
	stdio_write("    pmsc ctrl1: 0x");
	print_bytes(&pmscCtrl1Register, 4);

	uint32_t pmscSnoztRegister = dwt_read32bitoffsetreg(PMSC_ID, PMSC_SNOZT_OFFSET);
	stdio_write("    pmsc snozt:    0x");
	print_bytes(&pmscSnoztRegister, 4);

	uint32_t pmscTxfseqRegister = dwt_read32bitoffsetreg(PMSC_ID, PMSC_TXFINESEQ_OFFSET);
	stdio_write("    pmsc txfseq: 0x");
	print_bytes(&pmscTxfseqRegister, 4);

	uint32_t pmscLedControlRegister = dwt_read32bitoffsetreg(PMSC_ID, PMSC_LEDC_OFFSET);
	stdio_write("    pmsc led control: 0x");
	print_bytes(&pmscLedControlRegister, 4);

	stdio_write("\r\n\r\n");
}
