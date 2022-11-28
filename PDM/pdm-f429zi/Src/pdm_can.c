#include "pdm_can.h"

#include "pdm.h"
#include "string.h"
#include "stdio_d.h"

CAN_HandleTypeDef* hcan = 0;

CAN_TxHeaderTypeDef txHeader;
CAN_RxHeaderTypeDef rxHeader;

uint8_t txData[8];
uint8_t rxData[8];

uint32_t txMailbox[4];

void init_can(CAN_HandleTypeDef* canHandle)
{
	hcan = canHandle;
	HAL_CAN_Start(hcan);

	//Activate notification
//  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

	txHeader.DLC = 1; // Data length in bits
	txHeader.ExtId = 0;
	txHeader.IDE = CAN_ID_STD;
	txHeader.RTR = CAN_RTR_DATA;
	txHeader.StdId = 0x103;
	txHeader.TransmitGlobalTime = DISABLE;
}

can_status_t send_can_message()
{
	if (hcan == 0) {
		return CAN_ERR;
	}

	memset(&txData, 0, sizeof(txData));

	txData[0] = 'd';
	txData[1] = 'e';
	txData[2] = 'c';
	txData[3] = 'a';
	txData[4] = 'w';
	txData[5] = 'a';
	txData[6] = 'v';
	txData[7] = 'e';

	if (HAL_CAN_AddTxMessage(hcan, &txHeader, &txData[0], &txMailbox[0]) != HAL_OK) {
		return CAN_ERR;
	}

	return CAN_OK;
}

can_status_t send_can_control_msg(ecu_action_t action)
{
	txHeader.DLC = 2; // Data length in bits
	txHeader.ExtId = 0;
	txHeader.IDE = CAN_ID_STD;
	txHeader.RTR = CAN_RTR_DATA;
	txHeader.StdId = 0x103; // Arbitration_ID
	txHeader.TransmitGlobalTime = DISABLE;

	txData[0] = 0x01;

	switch (action)
	{
		case WAKEUP:
			txData[1] = 0x01;
			break;
		case START:
			txData[1] = 0x02;
			break;
		case LOCK:
			txData[1] = 0x03;
			break;
		case PANIC:
			txData[1] = 0x04;
			break;
		case NO_OP:
			return CAN_OK;
		default:
			txData[1] = 0x00;
			stdio_write("got unregistered action, cannot send CAN msg");
			break;
	}

	if (HAL_CAN_AddTxMessage(hcan, &txHeader, txData, &txMailbox[0]) != HAL_OK) {
		return CAN_ERR;
	}

	return CAN_OK;
}
