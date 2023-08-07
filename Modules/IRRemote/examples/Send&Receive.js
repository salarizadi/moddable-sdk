import Timer from "timer";
import IRRemote from "IRRemote";

const $Receiver = new IRRemote({
    pin     : 33, // ESP32
    method  : IRRemote.Receiver,
    type    : IRRemote.Hex,
    received: (code, raw) => {
        trace(`\nCode : ${code}`)
        trace(`\nRaw  : ${JSON.stringify(raw)}`)
    }
})
const $Transmitter = new IRRemote({
    pin   : 14, // ESP32
    method: IRRemote.Transmitter
})

// Sample raw data of infrared remote controller
// Button  : Power
// Code    : -1046180468
// HexCode : -0x3e5b7274
const powerButton  = [9041, 4493, 578, 579, 582, 577, 581, 577, 581, 577, 580, 578, 580, 577, 580, 577, 581, 579, 579, 1653, 582, 1652, 582, 1653, 581, 1655, 581, 1654, 581, 1652, 581, 1655, 580, 1654, 582, 1653, 581, 578, 580, 1654, 581, 578, 580, 578, 580, 578, 578, 1656, 580, 578, 579, 579, 578, 1654, 581, 577, 582, 1653, 580, 1654, 581, 1653, 583, 576, 579, 1655, 581];
const loopTransmit = Timer.repeat(() =>
    $Transmitter.transmit(powerButton), 1000
);

Timer.set(() => {
    Timer.clear(loopTransmit)
    $Receiver.die()
    $Transmitter.die()
    trace("\nIRRemote is died")
}, 5000)