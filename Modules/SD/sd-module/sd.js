/**
 * Copyright (c) 2023 Salarizadi
 * Github : https://github.com/salarizadi
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