### 1. Import module folder (sd-module) on ModdableSDK
  
  > Location : ModdableSDK\modules\files
  > 
  > Change folder name (sd-module) to (sd)
  
  <img width="768" alt="1" src="https://user-images.githubusercontent.com/67143370/230625906-3e26ca2e-c3a9-40f3-aca2-f8105b36dbad.PNG">

### 2. Import example folder (sd-example) on ModdableSDK

  > Location : ModdableSDK\examples\files
  > 
  > Change folder name (sd-example) to (sd)
  
  <img width="769" alt="2" src="https://user-images.githubusercontent.com/67143370/230625586-bda223ec-3df0-40db-a375-2b704b169a68.PNG">


### 3. And now you can setup SD card in your project Take a complete look at the sample code It's very simple It's a complete sample

### 4. Functions

```
class SD {
    mount ( config )
    unmount ( )
    busFree ( )
    disable ( )
    info ( )
    format ( )
}
```

# Import

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
```

### Format/Erase SDcard

```
$SD.format() // Need ESP-IDF version 5 or higher
```

### Disable mount SDcard

```
$SD.disable()
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
