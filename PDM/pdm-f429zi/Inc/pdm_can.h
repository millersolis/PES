#ifndef PDM_CAN_H
#define PDM_CAN_H

#include "stm32f4xx.h"

typedef enum can_status_t {
	CAN_OK, CAN_ERR
} can_status_t;

void init_can(CAN_HandleTypeDef* canHandle);
can_status_t send_can_message();

#endif // PDM_CAN_H
