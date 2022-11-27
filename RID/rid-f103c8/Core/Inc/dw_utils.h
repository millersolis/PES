#ifndef DW_UTILS_H
#define DW_UTILS_H

#include <stdint.h>
#include "dw_config.h"
#include "dw_helpers.h"

receive_status_t receive_msg(uint8_t buffer[RX_BUF_LEN]);

#endif // DW_UTILS_H
