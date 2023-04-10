/**
 * Copyright (c) 2023
 * @Version    : 1.5.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/SD
 * @Author     : https://salarizadi.github.io
 */

export default class SD @ "xs_sd_destructor" {
    mount ( config ) @ "xs_sd_mount";
    unmount ( ) @ "xs_sd_unmount";
    busFree ( ) @ "xs_sd_bus_free";
    disable ( ) {
        const unmount = this.unmount(), free = this.busFree();
        return {unmount, free};
    }
    info ( ) @ "xs_sd_info";
    // format ( ) @ "xs_sd_format"; Need ESP-IDF 5
}
