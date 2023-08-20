/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/RCSwitch
 * @Author     : https://salarizadi.github.io
 */

import RCSwitch from "RCSwitch"
import Timer from "timer"

const $Transmitter = new RCSwitch({
    pin: 13,
    mode: RCSwitch.TX
})

$Transmitter.onChanged = function () {
    trace(`\n${JSON.stringify(this.read())}`)
}

$Transmitter.setProtocol(2)
$Transmitter.setRepeat(15)
$Transmitter.setPulseLength(650)
$Transmitter.setTolerance(60)

// Send tristate code
$Transmitter.sendTristate("00000FFF0F0F") // Code = 5393

Timer.delay(1000)

$Transmitter.setProtocol(1)

// Send binary code
$Transmitter.send("0000011111100111") // Code = 2023

Timer.delay(2000)

$Transmitter.close()