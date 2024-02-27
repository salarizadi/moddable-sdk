/**
 * Copyright (c) 2024
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/keypad
 * @Author     : https://salarizadi.github.io
 */

import Timer from "timer"

const Digital = device.io.Digital

export default class {

    #config = {
        map    : [],
        rows   : [],
        columns: []
    };
    #callback

    #pins     = {}

    #previous = null
    #count    = 0

    constructor (config, callback) {
        if (typeof config !== "object")
            return false

        this.#config = {
            ...this.#config,
            ...config
        }

        this.#callback = typeof callback === "function" ? callback : () => {}

        this.#turnOnRows()
    }

    #pin (config) {
        let $this = this, which = null

        which = $this.#config.rows.includes(config.pin) ? "rows" : which
        which = $this.#config.columns.includes(config.pin) ? "cols" : which
        
        $this.#pins[config.pin]?.close()

        return $this.#pins[config.pin] = new Digital({
            edge: Digital.Rising | Digital.Falling,
            ...config,
            onReadable: function () {
                if (which === "cols") return false
                $this.#pressed(config, $this.#pins[config.pin], which, "click")
            }
        })
    }

    #turnOnRows ( ) {
        this.#config.rows.map(pin => this.#pin({
            pin : pin,
            mode: Digital.InputPullUp,
            edge: Digital.Falling
        }))
        this.#config.columns.map(pin => this.#pin({
            pin : pin,
            mode: Digital.InputPullDown
        }))
    }

    #turnOnCols ( ) {
        this.#config.columns.map(pin => this.#pin({
            pin : pin,
            mode: Digital.InputPullUp
        }))
        this.#config.rows.map(pin => this.#pin({
            pin : pin,
            mode: Digital.InputPullDown,
            edge: Digital.Falling
        }))
    }

    #pressed (config) {
        try {
            this.#turnOnCols()

            let column = false

            this.#config.columns.map(pin => {
                if (!this.#pins[pin].read()) {
                    column = this.#config.columns.indexOf(pin)
                    return
                }
            })

            if (column || column === 0) {
                const indexRow = this.#config.rows.indexOf(config.pin)
                if (typeof this.#config.map[indexRow] !== "undefined") {
                    const key = this.#config.map[indexRow][column]

                    if (this.#count === 0 && this.#previous !== key) {
                        this.#callback(key)

                        Timer.set(() => {
                            this.#previous = null
                            this.#count = 0
                        }, 150)
                    }

                    this.#previous = key
                    this.#count = 1
                }
            }

            this.#turnOnRows()
        } catch (e) { trace(`\nError keypad pressed : ${e}`) }
    }

}
