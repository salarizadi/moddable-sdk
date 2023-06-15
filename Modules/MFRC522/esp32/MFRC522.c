/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/MFRC522
 * @Author     : https://salarizadi.github.io
 */

#include "MFRC522.h"
#include "MFRC522_handler.c"

void xs_mfrc522_card ( xsMachine *the ) {xsmcSetBoolean(xsResult, PICC_IsNewCardPresent(MFRC522_SPI));}

void xs_mfrc522_serial_card ( xsMachine *the ) {xsmcSetBoolean(xsResult, PICC_ReadCardSerial(MFRC522_SPI));}

void xs_mfrc522_authenticate ( xsMachine *the ) {
    MIFARE_Key key;

    enum PICC_Command cmd_k;
    enum StatusCode status;

    const char *cmd = xsmcToString(xsArg(0)); // Cmd
    uint8_t block   = xsmcToInteger(xsArg(1)); // Block

    if ( strcmp(cmd, "A") == 0 )
        cmd_k = PICC_CMD_MF_AUTH_KEY_A;
    else if ( strcmp(cmd, "B") == 0 )
        cmd_k = PICC_CMD_MF_AUTH_KEY_B;
    else
        xsRangeError("\nAuthenticate command invalid (A OR B)\n");

    xsmcVars(1);
    xsmcGet(xsVar(0), xsArg(2), xsID_length);

    int length = xsmcToInteger(xsVar(0));
    for ( int i = 0; i < length; i++ ) {
       xsmcGetIndex(xsVar(0), xsArg(2), i);
       key.keyByte[i] = (uint8_t) xsmcToInteger(xsVar(0));
    }

    status = PCD_Authenticate(MFRC522_SPI, cmd_k, block, &key, &uid);
    if (status != STATUS_OK)
         xsTrace("<error>MFRC522 Authenticate failed\n");

    xsmcSetInteger(xsResult, status == STATUS_OK);
}

void xs_mfrc522_write ( xsMachine *the ) {
    enum StatusCode status;

    int block = xsmcToInteger(xsArg(0)); // Block

    xsmcVars(1);
    xsmcGet(xsVar(0), xsArg(1), xsID_length);
    int length = xsmcToInteger(xsVar(0));

    uint8_t data[length]; // Data

    for ( int i = 0; i < length; i++ ) {
       xsmcGetIndex(xsVar(0), xsArg(1), i);
       data[i] = (uint8_t) xsmcToInteger(xsVar(0));
    }

    status = MIFARE_Write(MFRC522_SPI, block, data, sizeof(data));
    if (status != STATUS_OK)
         xsTrace("<error>MFRC522 Write failed\n");

    xsmcSetBoolean(xsResult, status == STATUS_OK);
}

void xs_mfrc522_info ( xsMachine *the ) {
    xsmcVars(1);

    MIFARE_Key key;
    enum PICC_Command cmd_k;
    enum StatusCode status;

    Uid *cardID = &uid;

    xsSlot arrayCardID = xsmcNewArray(0);
    for (int i = 0; i < cardID->size; i++) {
        xsVar(0) = xsInteger(cardID->uidByte[i]);
        xsmcSetIndex(arrayCardID, i, xsVar(0));
    }

    xsResult = xsmcNewObject();

    xsmcSetArrayBuffer(xsVar(0), NULL, cardID->size);
    xsmcSet(xsResult, xsID_id, arrayCardID);

    xsmcSetInteger(xsVar(0), cardID->sak);
    xsmcSet(xsResult, xsID_sak, xsVar(0));

    // Start get version
    uint8_t ver = PCD_ReadRegister(MFRC522_SPI, VersionReg);
    if (ver == 0x92) {
        xsmcSetInteger(xsVar(0), 2);
    } else if (ver == 0x91) {
        xsmcSetInteger(xsVar(0), 1);
    } else {
        xsmcSetInteger(xsVar(0), 0);
    }
    xsmcSet(xsResult, xsID_version, xsVar(0));
    // Start get version

    PICC_Type piccType = PICC_GetType(cardID->sak);

    // Start get type
    switch (piccType) {
		case PICC_TYPE_ISO_14443_4:	  xsmcSetString(xsVar(0),"PICC compliant with ISO/IEC 14443-4"); break;
		case PICC_TYPE_ISO_18092:	  xsmcSetString(xsVar(0),"PICC compliant with ISO/IEC 18092 (NFC)"); break;
		case PICC_TYPE_MIFARE_MINI:	  xsmcSetString(xsVar(0),"MIFARE Mini, 320 bytes"); break;
		case PICC_TYPE_MIFARE_1K:	  xsmcSetString(xsVar(0),"MIFARE 1KB"); break;
		case PICC_TYPE_MIFARE_4K:	  xsmcSetString(xsVar(0),"MIFARE 4KB"); break;
		case PICC_TYPE_MIFARE_UL:	  xsmcSetString(xsVar(0),"MIFARE Ultralight or Ultralight C"); break;
		case PICC_TYPE_MIFARE_PLUS:	  xsmcSetString(xsVar(0),"MIFARE Plus"); break;
		case PICC_TYPE_MIFARE_DESFIRE:xsmcSetString(xsVar(0),"MIFARE DESFire");break;
		case PICC_TYPE_TNP3XXX:		  xsmcSetString(xsVar(0),"MIFARE TNP3XXX");break;
		case PICC_TYPE_NOT_COMPLETE:  xsmcSetString(xsVar(0),"SAK indicates UID is not complete.");break;
		case PICC_TYPE_UNKNOWN:
		default: xsmcSetString(xsVar(0),"UNKNOWN");
	}
    xsmcSet(xsResult, xsID_type, xsVar(0));
    // End get type
}

void xs_mfrc522_haltA ( xsMachine *the ) {
    uint8_t result, buffer[4];

    // Build command buffer
    buffer[0] = PICC_CMD_HLTA;
    buffer[1] = 0;

    // Calculate CRC_A
    result = PCD_CalculateCRC(MFRC522_SPI, buffer, 2, & buffer[2]);
    if (result == STATUS_OK) {
        result = PCD_TransceiveData(MFRC522_SPI, buffer, sizeof(buffer), NULL, 0, NULL, 0, false);
    }

    xsmcSetBoolean(xsResult, result == STATUS_OK);
}

void xs_mfrc522_stop_crypto ( xsMachine *the ) {
    PCD_ClearRegisterBitMask(MFRC522_SPI, Status2Reg, 0x08);
}

void xs_mfrc522_read ( xsMachine *the ) {
    xsmcVars(1);

    uint8_t block = xsmcToInteger(xsArg(0));
    uint8_t size  = xsmcToInteger(xsArg(1));

    uint8_t buffer[size], status;

    status = MIFARE_Read(MFRC522_SPI, block, &buffer, &size);
    if (status != STATUS_OK) {
        xsmcSetBoolean(xsResult, false);
        return;
    }

    xsmcSetArrayBuffer(xsResult, buffer, sizeof(buffer));
}

void xs_mfrc522 ( xsMachine *the ) {
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .miso_io_num   = MFRC522_MISO,
        .mosi_io_num   = MFRC522_MOSI,
        .sclk_io_num   = MFRC522_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 5000000,
        .mode           = 0,
        .spics_io_num   = MFRC522_CS,
        .queue_size     = 7
    };

    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ret == ESP_OK);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &MFRC522_SPI);
    assert(ret == ESP_OK);

    xsmcSetBoolean(xsResult, PCD_Init(MFRC522_SPI, the));
}