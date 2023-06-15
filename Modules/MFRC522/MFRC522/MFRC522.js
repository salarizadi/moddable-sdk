/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/MFRC522
 * @Author     : https://salarizadi.github.io
 */

export default class MFRC522 {

    init ( ) @ "xs_mfrc522";

    card ( ) @ "xs_mfrc522_card";

    serialCard ( ) @ "xs_mfrc522_serial_card";

    authenticate ( command, block, keys ) @ "xs_mfrc522_authenticate";

    read ( block, bufferSize ) @ "xs_mfrc522_read";

    write ( block, data ) @ "xs_mfrc522_write";

    info ( ) @ "xs_mfrc522_info";

    toHex ( int ) {return int.toString(16)}

    #mifareClassicSectorToSerial ( sector, keys ) {
        let status, firstBlock, no_of_blocks, isSectorTrailer;
        let c1, c2, c3, c1_, c2_, c3_;
        let invertedError, g = [], group;
        let firstInGroup;

        if (sector < 32) {
            no_of_blocks = 4;
            firstBlock = sector * no_of_blocks;
        } else if (sector < 40) {
            no_of_blocks = 16;
            firstBlock = 128 + (sector - 32) * no_of_blocks;
        } else {return}

        let byteCount = 18, buffer = [], blockAddr;

        isSectorTrailer = true;
        invertedError   = false;

        let $Sectors = {};

        for (let blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--) {
            blockAddr = firstBlock + blockOffset;

            $Sectors[blockAddr] = {
                bytes     : [],
                accessBits: []
            };

            if (isSectorTrailer) {
                status = this.authenticate("A", firstBlock, keys);
                if (!status) {
                    trace("<error> Authenticate(A) failed\n");
                    return;
                }
            }

            status = this.read(blockAddr, byteCount);
            if ( !status ) {
                trace("<error> Read failed\n");
                continue;
            }

            buffer = new Uint8Array(status);
            buffer = [...buffer];

            for (let index = 0; index < 16; index++)
                $Sectors[blockAddr].bytes.push(buffer[index]);

            if (isSectorTrailer) {
                c1 = buffer[7] >> 4;
                c2 = buffer[8] & 0xF;
                c3 = buffer[8] >> 4;
                c1_ = buffer[6] & 0xF;
                c2_ = buffer[6] >> 4;
                c3_ = buffer[7] & 0xF;
                invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
                g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
                g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
                g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
                g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
                isSectorTrailer = false;
            }

            if (no_of_blocks == 4) {
                group = blockOffset;
                firstInGroup = true;
            } else {
                group = blockOffset / 5;
                firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
            }

            if (firstInGroup) {
                $Sectors[blockAddr].accessBits = [
                    (g[group] >> 2) & 1,
                    (g[group] >> 1) & 1,
                    (g[group] >> 0) & 1
                ]
                if (invertedError) {
                    trace("<error> Inverted access bits did not match!");
                }
            }

            // if (group != 3 && (g[group] == 1 || g[group] == 6)) {
            //     let value = ((buffer[3]) << 24) | ((buffer[2]) << 16) | ((buffer[1]) << 8) | (buffer[0]);
            //     trace(" Value=0x");
            //     trace(value);
            //     trace(" Adr=0x");
            //     trace(buffer[12]);
            // }
            //
            // trace("\n");
        }

        return $Sectors;
    }
    #mifareClassicSectors ( type, keys, callback ) {
        let no_of_sectors = 0;

        switch ( type ) {
            case "MIFARE Mini, 320 bytes":no_of_sectors = 5;break;
            case "MIFARE 1KB":no_of_sectors = 16;break;
            case "MIFARE 4KB":no_of_sectors = 40;break;
            default: /** Should not happen. Ignore */ break;
        }

        if (no_of_sectors) {
            for (let i = no_of_sectors - 1; i >= 0; i--)
                if ( typeof callback === "function" )
                    callback(this.#mifareClassicSectorToSerial(i, keys))
        }

        this.haltA(); this.stopCrypto();
    }
    #mifareUltraLightSectors ( callback ) {
        let byteCount = 18, buffer, i;
        let $Sectors  = {};

        for (let page = 0; page < 16; page += 4) {
            buffer = this.read(page, byteCount);
            if ( !buffer ) {
                trace("<error>Read failed\n");
                continue;
            }

            buffer = new Uint8Array(buffer);
            buffer = [...buffer];

            for (let offset = 0; offset < 4; offset++) {
                i = page + offset;
                $Sectors[i] = [];
                for (let index = 0; index < 4; index++) {
                    $Sectors[i].push(buffer[4 * offset + index]);
                }
            }
        }

        callback($Sectors)
    }

    sectors ( keys = null, callback ) {
        const info = this.info();
        const type = info.type;

        switch ( type ) {
            case "MIFARE Mini, 320 bytes":
            case "MIFARE 1KB":
            case "MIFARE 4KB":
                this.#mifareClassicSectors(type, keys, callback);
                break;
            case "MIFARE Ultralight or Ultralight C":
                this.#mifareUltraLightSectors(callback);
                break;
            case "PICC compliant with ISO/IEC 14443-4":
            case "MIFARE DESFire":
            case "PICC compliant with ISO/IEC 18092 (NFC)":
            case "MIFARE Plus":
            case "MIFARE TNP3XXX":
                trace("<error>Dumping of memory contents is not done for this type of PICC.\n");
                break;
            case "UNKNOWN": case "SAK indicates UID is not complete.": default: break;
        }
    }

    haltA ( ) @ "xs_mfrc522_haltA";

    stopCrypto ( ) @ "xs_mfrc522_stop_crypto";

}