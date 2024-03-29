/**
 * Copyright (c) 2023
 * @Version    : 1.2.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/cluster
 * @Author     : https://salarizadi.github.io
 */
import TextDecoder from "text/decoder";

export default class Cluster {

    #dictionary = {
        transmit: 17,
        receive : 16,
        baud: 115200,
        port: 2
    };

    constructor ( dictionary = {} ) {
        this.#dictionary = {...this.#dictionary, ...dictionary};
        this.serial = new device.io.Serial({
            ...device.Serial.default,
            ...this.#dictionary,
            format : "buffer",
            onReadable: this.#onReadable
        });
        if (!Array.isArray(global.ClusterCommands)) {
            global.ClusterCommands = [];
        }
    }

    #onReadable ( count ) {
        try {
            if ( count ) {
                const read = this.read();
                if ( read && read.byteLength ) {
                    const decoder = new TextDecoder;
                    const string  = decoder.decode(read, {stream: true});
                    if ( typeof string === "string" ) {
                        const message = /^\s*(\{|\[)/.test(string) ? JSON.parse(string) : "";
                        if ( typeof message === "object" ) {
                            global.ClusterCommands.forEach((value, index, array) => {
                                if ( value.name === message.name ) {
                                    value.method(message.data)
                                }
                            })
                        }
                    }
                }
            }
        } catch (e) { }
    }

    bind ( name, method ) {
        if (typeof name === "string") {
            if (typeof method === "function") global.ClusterCommands.push({name, method})
        }
    }

    trigger ( name, data ) {
        if (typeof name === "string")
            if (typeof data === "object") this.serial.write(ArrayBuffer.fromString(
                JSON.stringify({name, data})
            ));
    }

}
