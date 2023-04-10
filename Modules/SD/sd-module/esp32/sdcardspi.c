/**
 * Copyright (c) 2023
 * @Version    : 1.5.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/SD
 * @Author     : https://salarizadi.github.io
 */

#include "xsmc.h"
#include "mc.xs.h"
#include "mc.defines.h"
#include "xsHost.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#ifdef MODDEF_SD_SPI_HOST
    const int xs_sd_spi_host = MODDEF_SD_SPI_HOST;
#else
    const int xs_sd_spi_host = 1; // Default HSPI_HOST
#endif
#define xs_SDSpiHost() {\
    .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG, \
    .slot = xs_sd_spi_host, \
    .max_freq_khz = SDMMC_FREQ_DEFAULT, \
    .io_voltage = 3.3f, \
    .init = &sdspi_host_init, \
    .set_bus_width = NULL, \
    .get_bus_width = NULL, \
    .set_bus_ddr_mode = NULL, \
    .set_card_clk = &sdspi_host_set_card_clk, \
    .do_transaction = &sdspi_host_do_transaction, \
    .deinit_p = &sdspi_host_remove_device, \
    .io_int_enable = &sdspi_host_io_int_enable, \
    .io_int_wait = &sdspi_host_io_int_wait, \
    .command_timeout_ms = 0, \
}

#define XS_SD_MOUNT_POINT "/sdcard"
const char xs_sd_mount_point[] = XS_SD_MOUNT_POINT;

// Default pins
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS	 5
#define XS_SD_FF	 true // format_if_mount_failed
#define XS_SD_TRANSFER_SZ 4000
#define XS_SD_MAX_FILES 5
#define XS_SD_AUS (16 * 1024) // allocation_unit_size

sdmmc_card_t *xs_sdmmc_card;
sdmmc_host_t xs_sd_host = xs_SDSpiHost();

void xs_sd_destructor ( void *data ) { }

void xs_sd_mount ( xsMachine *the ) {
    esp_err_t ret; xsmcVars(1);

    int miso = PIN_NUM_MISO;
    int mosi = PIN_NUM_MOSI;
    int clk  = PIN_NUM_CLK;
    int cs   = PIN_NUM_CS;
    int aus  = XS_SD_AUS;
    bool ff  = XS_SD_FF;
    int transfer_sz = XS_SD_TRANSFER_SZ;
    int max_files   = XS_SD_MAX_FILES;

    if (xsmcHas(xsArg(0), xsID_miso)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_miso);
    	miso = xsmcToInteger(xsVar(0));
    }

    if (xsmcHas(xsArg(0), xsID_mosi)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_mosi);
    	mosi = xsmcToInteger(xsVar(0));
    }

    if (xsmcHas(xsArg(0), xsID_clk)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_clk);
    	clk = xsmcToInteger(xsVar(0));
    }

    if (xsmcHas(xsArg(0), xsID_cs)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_cs);
    	cs = xsmcToInteger(xsVar(0));
    }

    if (xsmcHas(xsArg(0), xsID_ff)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_ff);
    	ff = xsmcToBoolean(xsVar(0));
    }

    if (xsmcHas(xsArg(0), xsID_transfer_sz)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_transfer_sz);
    	transfer_sz = xsmcToInteger(xsVar(0));
    }

    if (xsmcHas(xsArg(0), xsID_max_files)) {
    	xsmcGet(xsVar(0), xsArg(0), xsID_max_files);
    	max_files = xsmcToInteger(xsVar(0));
    }

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = ff,
        .max_files = max_files,
        .allocation_unit_size = aus
    };

    // Initializing SD card & Using SPI peripheral
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = transfer_sz
    };

    ret = spi_bus_initialize(xs_sd_host.slot, &bus_cfg, 2);
    if (ret != ESP_OK) {
        xsTypeError("Failed to initialize bus");
        xsmcSetBoolean(xsResult, false);
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = cs;
    slot_config.host_id = xs_sd_host.slot;

    // Mounting filesystem
    ret = esp_vfs_fat_sdspi_mount(xs_sd_mount_point, &xs_sd_host, &slot_config, &mount_config, &xs_sdmmc_card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            xsTypeError("Failed to mount filesystem. If you want the card to be formatted, set the ff");
        } else {
            xsTypeError("Failed to initialize the card. Make sure you enter the PINs correctly and if you haven't inserted a memory card, insert it");
        }
        xsmcSetBoolean(xsResult, false); return;
    }

    // sdmmc_card_print_info(stdout, &xs_sdmmc_card);

    xsmcSetBoolean(xsResult, true);
}

void xs_sd_unmount ( xsMachine *the ) {
    xsmcSetBoolean(xsResult, esp_vfs_fat_sdcard_unmount(xs_sd_mount_point, xs_sdmmc_card) == ESP_OK);
}

void xs_sd_bus_free ( xsMachine *the ) {
    xsmcSetBoolean(xsResult, spi_bus_free(xs_sd_host.slot) == ESP_OK);
}

void xs_sd_info ( xsMachine *the ) {
    xsResult = xsmcNewObject();

    xsmcVars(1);

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->cid.name);
    xsmcSet(xsResult, xsID_name, xsVar(0));

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->csd.csd_ver);
    xsmcSet(xsResult, xsID_csd_ver, xsVar(0));

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->csd.tr_speed);
    xsmcSet(xsResult, xsID_speed, xsVar(0));

    xsmcSetInteger(xsVar(0), ((uint64_t) xs_sdmmc_card->csd.capacity) * xs_sdmmc_card->csd.sector_size / (1024 * 1024));
    xsmcSet(xsResult, xsID_storage, xsVar(0)); // Storage unit : MB

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->csd.sector_size);
    xsmcSet(xsResult, xsID_sector_size, xsVar(0));

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->csd.capacity);
    xsmcSet(xsResult, xsID_capacity, xsVar(0));

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->scr.sd_spec);
    xsmcSet(xsResult, xsID_sd_spec, xsVar(0));

    xsmcSetInteger(xsVar(0), xs_sdmmc_card->scr.bus_width);
    xsmcSet(xsResult, xsID_bus_width, xsVar(0));

    xsmcSetInteger(xsVar(0), xs_sd_host.slot);
    xsmcSet(xsResult, xsID_spi_host, xsVar(0));
}

// Need ESP-IDF 5
//void xs_sd_format ( xsMachine *the ) {
//    xsmcSetBoolean(xsResult, sdmmc_full_erase(xs_sdmmc_card) == ESP_OK);
//}
