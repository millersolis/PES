#ifndef PDM_CAN_H
#define PDM_CAN_H

#include "stm32f4xx.h"
#include "pdm.h"

typedef enum can_status_t {
	CAN_OK, CAN_ERR
} can_status_t;

void init_can(CAN_HandleTypeDef* canHandle);
can_status_t send_can_message();
can_status_t send_can_control_msg(ecu_action_t state);

#endif // PDM_CAN_H
