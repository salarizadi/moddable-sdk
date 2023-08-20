/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/RCSwitch
 * @Author     : https://salarizadi.github.io
 * @Description: This part is for synchronizing the XS Moddable SDK with RCSwitch.c
*/

#include "xsmc.h"
#include "mc.xs.h"
#include "mc.defines.h"
#include "xsHost.h"

#include "include/RCSwitch.c"

typedef struct {
	xsMachine *the;
	xsSlot	  obj;
	RCSWITCH_t RCSwitch;
	TaskHandle_t task;

	uint8_t	mode;
	uint8_t	pin;
	unsigned long code;
	uint8_t	bit;
	uint8_t	protocol;
    uint8_t closed;
} xsRCSwitchRecord, *xsRCSwitch;

void xs_rcswitch_destructor (void *data) {
    xsRCSwitch rcswitch = data;
    if (NULL == rcswitch)
    	return;
    	
    gpio_set_intr_type(rcswitch->pin, GPIO_INTR_DISABLE);
    gpio_isr_handler_remove(rcswitch->pin);

    if (rcswitch->mode == 0)
        vTaskDelete(rcswitch->task);

    c_free(rcswitch);
}

void xs_rcswitch_on_change (void *the, void *_rcswitch, uint8_t *message, uint16_t messageLength) {
	xsRCSwitch rcswitch = _rcswitch;
	xsBeginHost(the);
    xsCall0(rcswitch->obj, xsID_onChanged);
    xsEndHost(the);
}

void xs_rcswitch_receiver (void *_rcswitch) {
	xsRCSwitch rcswitch = _rcswitch;

    rcswitch_init(&rcswitch->RCSwitch);
    rcswitch_enable_receive(&rcswitch->RCSwitch, rcswitch->pin);

	while (1) {
        if (rcswitch_available(&rcswitch->RCSwitch)) {
            rcswitch->code     = rcswitch_received_value(&rcswitch->RCSwitch);
            rcswitch->bit      = rcswitch_received_bit_length(&rcswitch->RCSwitch);
            rcswitch->protocol = rcswitch_received_protocol(&rcswitch->RCSwitch);
            modMessagePostToMachineFromISR(rcswitch->the, xs_rcswitch_on_change, rcswitch);
            rcswitch_reset_available(&rcswitch->RCSwitch);
        } else {
            vTaskDelay(1);
        }
	}
}

void xs_rcswitch_read (xsMachine *the) {
	xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    xsResult = xsmcNewObject();

    xsmcVars(1);

	xsmcSetInteger(xsVar(0), rcswitch->pin);
    xsmcSet(xsResult, xsID_pin, xsVar(0));

	xsmcSetInteger(xsVar(0), rcswitch->code);
    xsmcSet(xsResult, xsID_code, xsVar(0));

	xsmcSetInteger(xsVar(0), rcswitch->bit);
    xsmcSet(xsResult, xsID_bit, xsVar(0));

	xsmcSetInteger(xsVar(0), rcswitch->protocol);
    xsmcSet(xsResult, xsID_protocol, xsVar(0));
}
void xs_rcswitch_send (xsMachine *the) {
    xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    if (rcswitch->closed == 0) {
        rcswitch_send_binary(&rcswitch->RCSwitch, xsmcToString(xsArg(0)));
    }
}
void xs_rcswitch_send_tristate (xsMachine *the) {
    xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    rcswitch_send_tristate(&rcswitch->RCSwitch, xsmcToString(xsArg(0)));
}

void xs_rcswitch_set_protocol (xsMachine *the) {
	xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    rcswitch_set_protocol(&rcswitch->RCSwitch, xsmcToInteger(xsArg(0)));
}
void xs_rcswitch_set_repeat (xsMachine *the) {
	xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    rcswitch_set_repeat_transmit(&rcswitch->RCSwitch, xsmcToInteger(xsArg(0)));
}
void xs_rcswitch_set_pulse_length (xsMachine *the) {
	xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    rcswitch_set_pulse_length(&rcswitch->RCSwitch, xsmcToInteger(xsArg(0)));
}
void xs_rcswitch_set_tolerance (xsMachine *the) {
	xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
    rcswitch_set_receive_tolerance(&rcswitch->RCSwitch, xsmcToInteger(xsArg(0)));
}

void xs_rcswitch_close (xsMachine *the) {
    xsRCSwitch rcswitch = xsmcGetHostData(xsThis);
	xsForget(rcswitch->obj);
	rcswitch->closed = true;
	xs_rcswitch_destructor(rcswitch);
	xsmcSetHostData(xsThis, NULL);
}

void xs_rcswitch (xsMachine *the) {
	xsRCSwitch rcswitch;
	int mode, pin;

    xsmcVars(1);

    if (!xsmcHas(xsArg(0), xsID_mode))
        xsUnknownError("mode missing");

    if (!xsmcHas(xsArg(0), xsID_pin))
        xsUnknownError("pin missing");

    xsmcGet(xsVar(0), xsArg(0), xsID_mode);
    mode = xsmcToInteger(xsVar(0));

    xsmcGet(xsVar(0), xsArg(0), xsID_pin);
    pin = xsmcToInteger(xsVar(0));

    rcswitch = c_malloc(sizeof(xsRCSwitchRecord));
	if (!rcswitch)
		xsUnknownError("no memory");

    rcswitch->the    = the;
    rcswitch->obj    = xsThis;
    rcswitch->mode   = mode;
    rcswitch->pin    = pin;
    rcswitch->closed = false;

    switch (rcswitch->mode) {
         case 0:
             xTaskCreate(&xs_rcswitch_receiver, "xs_rcswitch_receiver", 1024 * 4, rcswitch, 2, &rcswitch->task);
             break;
         case 1:
             rcswitch_init(&rcswitch->RCSwitch);
             rcswitch_enable_transmit(&rcswitch->RCSwitch, rcswitch->pin);
             break;
         default:
             xsTypeError("mode is wrong (Receiver(0) OR Transmitter(1))");
    }

    xsRemember(rcswitch->obj);
    xsmcSetHostData(xsThis, rcswitch);
}
