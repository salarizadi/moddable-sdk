/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/RCSwitch
 * @Author     : https://salarizadi.github.io
 */

import RCSwitch from "RCSwitch"
import Timer from "timer"

const $Receiver = new RCSwitch({
    pin: 22,
    mode: RCSwitch.RX
})

$Receiver.onChanged = function () {
    trace(`\n${JSON.stringify(this.read())}`)
}

Timer.delay(2000)

$Receiver.close()