## About This Module

This library is written for the RFID-RC522 module. You can easily read and even edit your RFID cards. This library is ported from the [RFID-Arduino](https://github.com/miguelbalboa/rfid) library.

## Help list and setup
  * Config
      * [Move folders](#move-folders)
      * [Wiring](#wiring)
  * [Functions](#functions)
  * [Examples](https://github.com/salarizadi/moddable-sdk/tree/main/Modules/MFRC522/Examples)
  * [Support cards](#support-cards)


## Move folders
<a id="move-folders"></a>

After downloading the library, you should move the folders to their original place :
  
  * Folder MFRC522 (Important) : The exact folder must have this address
      
      ```$(MODULES)/drivers/MFRC522```
      
  * Folder Examples (Optional) : The exact folder must have this address
      
      ```$(MODULES)/examples/MFRC522```
      * Rename folder Examples to MFRC522
     

## Wiring
<a id="wiring"></a>

| Device | MISO | MOSI | CLK | CS | RST |
| --- | :-- | :-- | :-- | :-- | :-- |
| `ESP32-WROOM-32` | 25 | 23 | 19 | 22 | 14

Note : You can do custom wiring yourself, but you must enter your custom pins in the definition section of your project manifest, such as :
```json
...
"defines": {
  ...
  "mfrc522": {
    "miso": 25,
    "mosi": 23,
    "clk": 19,
    "cs": 22,
    "rst": 14
  }
}
```
  
  
## Functions
<a id="functions"></a>

  | Function | Description | Arguments | Return
  | --- | :-- | :-- | :-- |
  | `init` | Complete SPI setup and RFID-RC522 module connection check | `NULL` | `bool`
  | `card` | If the new card is close to the module, it will return true | `NULL` | `bool`
  | `serialCard` | This function reads and saves card information for other operations | `NULL` | `bool`
  | `authenticate` | Used to exit the PCD from its authenticated state. Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start. | string command "A" or "B", int block, array keys | `bool`
  | `read` | Read the block contains | int block, int bufferSize | `NULL` | `ArrayBuffer`
  | `write`| Write on the card | int block, array data | `NULL` | `bool`
  | `info` | Get card information: id, type, sak, version, ... | `NULL` | `object`
  | `toHex` | The returned data is not hexadecimal, so this function helps you to convert a number to hexadecimal if you want. | `NULL` | `hex`
  | `sectors` | Get the complete list of information stored on the card, sector rows and values stored in each row | array keys, function callback | `NULL`
  | `haltA` | Commands a PICC in ACTIVE state to go into HALT state | `NULL` | `bool`
  | `stopCrypto` | Used to exit the PCD from its authenticated state. Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start. | `NULL` | `NULL`
  
## Support cards
<a id="functions"></a>

| Type | Support | Show sectors
| --- | :-- | :-- |
| `MIFARE Mini, 320 bytes` | ✔ | ✔ |
| `MIFARE 1KB` | ✔ | ✔ |
| `MIFARE 4KB` | ✔ | ✔ |
| `MIFARE Ultralight or Ultralight C` | ✔ | ✔ |
| `MIFARE DESFire` | ✔ | ❌
| `PICC compliant with ISO/IEC 18092 (NFC)` | ✔ | ❌
| `MIFARE Plus` | ✔ | ❌
| `MIFARE TNP3XXX` | ✔ | ❌


## Buy coffee for me ☕
  I worked very hard for this project. If you want to support me : [Click me](https://github.com/salarizadi/moddable-sdk/blob/main/README.md#buy-coffee-for-me-)  
