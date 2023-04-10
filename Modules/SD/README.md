### 1. Wiring

  ![esp32-SDCardAdapter33v_schem-768x336](https://user-images.githubusercontent.com/67143370/230978963-d3bad5d8-2f87-467e-9fbc-013b6b39eac4.jpg)

  > Note : In the image above, the default pins of the SD module are connected, but to show you that you can use your own custom pins, other pins are used in the       examples. (We will talk about it later)

### 2. Import module folder (sd-module) on ModdableSDK
  
  > Location : ModdableSDK\modules\files
  > 
  > Change folder name (sd-module) to (sd)
  
  <img width="768" alt="1" src="https://user-images.githubusercontent.com/67143370/230625906-3e26ca2e-c3a9-40f3-aca2-f8105b36dbad.PNG">

### 3. Import example folder (sd-example) on ModdableSDK

  > Location : ModdableSDK\examples\files
  > 
  > Change folder name (sd-example) to (sd)
  
  <img width="769" alt="2" src="https://user-images.githubusercontent.com/67143370/230625586-bda223ec-3df0-40db-a375-2b704b169a68.PNG">

### Direct import from the github repository in the project manifest :
> This type of import is suggested by [STC1998](https://github.com/Moddable-OpenSource/moddable/discussions/1081#discussioncomment-5558475)

  ```
  "include": [	
    ...
		{ 
			"git":"https://github.com/salarizadi/moddable-sdk.git", 
			"include":"Modules/SD/sd-module/manifest_spi.json"
		}
	]
  ```

### 4. And now you can setup SD card in your project Take a complete look at the sample code It's very simple It's a complete sample

### 5. Setting up the project manifest :
  + If you have a display or SPI host other than the SD card and there is an interference with the setup process, you can change your SD card host via your project       manifest definitions, for example:
  ```
  "defines": {
    ...
    "sd_spi_host": 2 // Note: HSPI_HOST is equal to 1 and VSPI_HOST is equal to 2. Default is HSPI_HOST.
  }
  ```

### Functions

```
class SD {
    mount ( config )
    unmount ( )
    busFree ( )
    disable ( )
    info ( )
    format ( ) // Need ESP-IDF version 5 or higher
}
```

### Import

```
import SD from "sd"

const $SD  = new SD();
const root = "/sdcard/";
```

### Mount SDcard

```
$SD.mount({
    miso: 15, // Default 19
    mosi: 12, // Default 23
    clk: 14, // Default 18
    cs: 13, // Default 5
    ff: true, // Format if mount failed
    transfer_sz: 4000,
    max_files: 5,
    aus: 16 * 1024 // Allocation unit size
});
```

### Get full info of SDcard

```
$SD.info()

This returns a JSON Object :
{
  "name",
  "csd_ver",
  "speed",
  "storage", // Storage unit : MB
  "sector_size",
  "capacity",
  "sd_spec",
  "bus_width",
  "spi_host"
}
```

### Format/Erase SDcard

```
$SD.format() // Need ESP-IDF version 5 or higher
```

### Disable mount SDcard

```
$SD.disable()
```

### It shows this when you run the example on an XS console, I tested with a 2GB and 32GB SD card.

> Notes : When the $SD.info() function is executed, the output of my memory card information is different from yours

```
SD initialize : true

This is a test.
We can write multiple values.
This is the end of the test.

test.txt renamed to test2.txt

name: Brian, city: Del Mar, state: CA

File length: 20
Last five shorts: 5 6 7 8 9 
System Volume Information        directory
test.bin                         file          20 bytes
test2.txt                        file          75 bytes
preferences.json                 file          46 bytes

Info : {"name":1073430784,"csd_ver":0,"speed":25000000,"storage":1898,"sector_size":512,"capacity":3887104,"sd_spec":2,"bus_width":5,"spi_host":2}

Disable : {"unmount":true,"free":true}
```

### Notes
  + If you want to use the (format) function, you must use ESP-IDF version 5 or higher and uncomment the xs_sd_format function in the (sdcardspi.c) file and the (format) function in the (sd.js) file.
  
  > sdcardspi.c
  ```
  void xs_sd_format ( xsMachine *the ) {
      xsmcSetBoolean(xsResult, sdmmc_full_erase(xs_sdmmc_card) == ESP_OK);
  }
  ```
  > sd.js
  ```
  format ( ) @ "xs_sd_format";
  ```
  
### Versions
  + 1.5.0 :
    - Fix bugs
    - Added definition (sd_spi_host) to customize SPI host in manifest definitions
    - (spi_host) was added in the (info) function
  
  + 1.0.0 :
    - spi sd card for esp32
