/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/IRRemote
 * @Author     : https://salarizadi.github.io
 */

import Timer from "timer"
import RMT from "pins/rmt"

class IRRemote extends RMT {

    #Structure = {
        init : false,
        timer: false,
        type : 0x2
    }
    #Raw

    constructor ( options ) {
        if ( !options instanceof Object )
            options = {};

        if (options.method === 0x0) options.rmt = {
            channel: 0,
            timeout: 30000,
            ...options.rmt,
            pin: options.pin,
            divider: 80,
            filter: 255,
            ringbufferSize: 512,
            direction: "rx"
        }; else if (options.method === 0x1) options.rmt = {
            channel: 1,
            ...options.rmt,
            pin: options.pin,
            divider: 80,
            direction: "tx",
            tx_config: {
                loop_en: 0,
                carrier_freq_hz: 38000,
                carrier_duty_percent: 33,
                carrier_level: 1,
                carrier_en: 1,
                idle_level: 0,
                idle_output_en: 0
            }
        }

        super(options.rmt)

        this.#Structure = {
            ...this.#Structure,
            ...options,
            init: true
        }

        if (options.method === 0x0) {
            this.#Raw = new Uint16Array(this.#Structure.rmt.ringbufferSize)
            this.#receiver()
        } else if (options.method === 0x1)
            this.onWritable = () => {}
    }

    transmit ( raw ) {
        if (this.#Structure.init) {
            if (this.#Structure.method === 0x1 && raw instanceof Array) {
                return this.write(1, raw)
            } else return false
        }
    }

    #receiver ( ) {
        this.#Structure.timer = Timer.repeat(() => {
            let received = this.read(this.#Raw.buffer)
            if (received.count > 6) { // Prevent noises
                let code = 0x811c9dc5
                for (let i = 1; (i + 2) < received.count; i++) {
                    code = (code * 0x1000193) ^ (
                        (this.#Raw[i + 2] * 10 < this.#Raw[i] * 8) ? 0 : (
                            this.#Raw[i] * 10 < this.#Raw[i + 2] * 8
                        ) ? 2 : 1
                    )
                }
                this.#Structure.received(this.#Structure.type === 0x3 ? code : (
                    (code < 0 ? "-" : "") + "0x" + Math.abs(code).toString(16)
                ), this.#Raw.filter(n => n !== 0))
            }
        }, 100)
    }

    die ( ) {
        if (this.#Structure.init) {
            if (this.#Structure.timer) {
                Timer.clear(this.#Structure.timer)
                this.#Structure.timer = false
            }
            this.close()
            this.#Structure.init = false
        }
    }

}

IRRemote.Receiver    = 0x0
IRRemote.Transmitter = 0x1
IRRemote.Number = 0x2
IRRemote.Hex    = 0x3

Object.freeze(IRRemote.prototype)

export default IRRemote