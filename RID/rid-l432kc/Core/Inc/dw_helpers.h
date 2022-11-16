#ifndef DW_HELPERS_H
#define DW_HELPERS_H

#include <stddef.h>
#include <stdint.h>

extern void print_timeout_errors(uint32_t statusRegister);
extern void print_status_errors(uint32_t statusRegister);
extern void print_bytes(uint8_t* data, size_t length);
extern void print_status_register();

typedef enum receive_status_t {
	STATUS_RECEIVE_OK,
	STATUS_RECEIVE_ERROR,
	STATUS_RECEIVE_TIMEOUT
} receive_status_t;

typedef enum send_status_t {
	STATUS_SEND_OK,
	STATUS_SEND_ERROR
} send_status_t;

typedef enum {
	FRAME_TYPE_BEACON,
	FRAME_TYPE_DATA,
	FRAME_TYPE_ACK,
	FRAME_TYPE_MAC_COMMAND
} frame_type_t;

typedef enum {
	FRAME_ADDR_ABSENT,
	FRAME_ADDR_SHORT,
	FRAME_ADDR_EXTENDED
} fcf_addr_t;

typedef enum {
	FRAME_V_2003,
	FRAME_V_MODERN
} fcf_version_t;

typedef struct frame_control_config_t {
	frame_type_t type;
	uint8_t securityEnabled;
	uint8_t framePending;
	uint8_t ackRequest;
	uint8_t panIdCompress;
	fcf_addr_t destAddr;
	fcf_version_t frameVersion;
	fcf_addr_t srcAddr;
} frame_control_config_t;

typedef struct frame_addr_config_t {
	uint8_t destPanId;
	uint64_t destAddr;
	uint8_t srcPanId;
	uint64_t srcAddr;
} frame_addr_config_t;

extern uint16_t format_frame_control(frame_control_config_t config);
extern uint16_t format_frame_header(uint8_t buffer[128], frame_control_config_t fcConfig, frame_addr_config_t addrConfig);

#endif // DW_HELPERS_H
