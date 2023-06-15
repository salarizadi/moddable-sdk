/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/MFRC522
 * @Author     : https://salarizadi.github.io
 */

import MFRC522 from "MFRC522";
import Timer from "timer";

const mfrc522 = new MFRC522();
const mfrc522_init = mfrc522.init();

trace(`<info>Init MFRC522 : ${mfrc522_init}\n\n`)

const block        = 4;
const trailerBlock = 7;
const keys = [
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF
];
const writeBuffers = [
    0x01, 0x02, 0x03, 0x04, // 1,  2,   3,  4,
    0x05, 0x06, 0x07, 0x08, // 5,  6,   7,  8,
    0x09, 0x0a, 0xff, 0x0b, // 9, 10, 255, 11,
    0x0c, 0x0d, 0x0e, 0x0f  // 12, 13, 14, 15
];
let scanning = false;

if ( mfrc522_init ) {
    Timer.repeat(function () {
        if ( mfrc522.card() && !scanning ) {
            scanning = true

            trace("\n***************** CARD PRESENT *****************\r\n")

            if ( !mfrc522.serialCard() ) {
                trace("<error>Failed to read serial card\n")
                return false
            }

            let authenticateA = mfrc522.authenticate("A", trailerBlock, keys);
            if ( !authenticateA )
                trace("<error>Authenticate (A) failed\n")

            let authenticateB = mfrc522.authenticate("B", trailerBlock, keys);
            if ( !authenticateB )
                trace("<error>Authenticate (B) failed\n")

            if ( mfrc522.write(block, writeBuffers) )
                trace("<info>Success write\n");
            else
                trace("<error>Failed write\n")

            if ( authenticateA ) {
                trace(`\nInfo : ${JSON.stringify(mfrc522.info())}\n`)

                trace(`\n<warn>Sectors : \n`);
                mfrc522.sectors(keys, sectors => {
                    trace(`${JSON.stringify(sectors)}\n`)
                })

                scanning = false
            }

            Timer.delay(1000)
        }
    }, 100)
}