/**
 * Copyright (c) 2023
 * @Version    : 1.5.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/SD
 * @Author     : https://salarizadi.github.io
 */

import SD from "sd"
import {File, Iterator, System} from "file";

const $SD = new SD();
const root = "/sdcard/";

let file;

// Setup SD card
trace("SD initialize : " + $SD.mount({
    miso: 15, // Default 19
    mosi: 12, // Default 23
    clk: 14, // Default 18
    cs: 13, // Default 5
    ff: true, // Format if mount failed
    transfer_sz: 4000,
    max_files: 5,
    aus: 16 * 1024 // Allocation unit size
}) + "\n\n");

// writing/reading strings
file = new File(root + "test.txt", true);
file.write("This is a test.\n");
file.write("We can write ", "multiple", " values.\n");
file.write("This is the end of the test.\n");
file.position = 0;
let content = file.read(String);
trace(content);
file.close();
trace("\n");

// renaming file
let from = "test.txt";
let to = "test2.txt";
File.rename(root + from, to);
if (File.exists(root + to))
    trace(`${from} renamed to ${to}\n`);
trace("\n");

// writing/reading JSON
let preferences = {name: "Brian", city: "Del Mar", state: "CA"};
file = new File(root + "preferences.json", true);
file.write(JSON.stringify(preferences));
file.close();
file = new File(root + "preferences.json");
preferences = JSON.parse(file.read(String));
trace(`name: ${preferences.name}, city: ${preferences.city}, state: ${preferences.state}\n`);
file.close(file)
trace("\n");

// writing/reading ArrayBuffers
let length = 10;
let buffer = new ArrayBuffer(length * 2);
let shorts = new Uint16Array(buffer);
for (let i = 0; i < length; ++i)
    shorts[i] = i;
file = new File(root + "test.bin", true);
file.write(buffer);
trace(`File length: ${file.length}\n`);
file.position = 10;
shorts = new Uint16Array(file.read(ArrayBuffer, 10));
length = shorts.length;
trace("Last five shorts: ");
for (let i = 0; i < length; ++i)
    trace(shorts[i] + ' ');
file.close(file)
trace("\n");

// directory iterator
for (const item of (new Iterator(root))) {
    if (undefined === item.length)
        trace(`${item.name.padEnd(32)} directory\n`);
    else
        trace(`${item.name.padEnd(32)} file          ${item.length} bytes\n`);
}
trace("\n");

trace("Info : " + JSON.stringify($SD.info()) + "\n");

File.delete(root + "test2.txt");
File.delete(root + "preferences.json");
File.delete(root + "test.bin");

// trace("Format SD Card : " + $SD.format()); // Need ESP-IDF 5 

trace("Disable : " + JSON.stringify($SD.disable()));
