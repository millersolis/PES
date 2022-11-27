#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>
#include "dw_config.h"
#include "dw_helpers.h"

#define USER_BTN_FRAME_COMMON_LEN 22

bool is_user_lock_msg(uint8_t* rx_buffer);
bool is_user_panic_msg(uint8_t* rx_buffer);
send_status_t send_user_lock_msg();
send_status_t send_user_panic_msg();


#endif // BUTTONS_H
