/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/RCSwitch
 * @Author     : https://salarizadi.github.io
 */

class RCSwitch @ "xs_rcswitch_destructor" {
    constructor(config) @ "xs_rcswitch"

    read () @ "xs_rcswitch_read"
    send (code) @ "xs_rcswitch_send"
    sendTristate (code) @ "xs_rcswitch_send_tristate"

    setProtocol (protocol) @ "xs_rcswitch_set_protocol"
    setRepeat (repeat) @ "xs_rcswitch_set_repeat"
    setPulseLength (length) @ "xs_rcswitch_set_pulse_length"
    setTolerance (tolerance) @ "xs_rcswitch_set_tolerance"

    close () @ "xs_rcswitch_close"
}

RCSwitch.RX = 0
RCSwitch.TX = 1

Object.freeze(RCSwitch.prototype)

export default RCSwitch