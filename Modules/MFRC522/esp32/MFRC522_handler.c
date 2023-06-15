/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/MFRC522
 * @Author     : https://salarizadi.github.io
 */

bool PCD_Version ( spi_device_handle_t spi, xsMachine *the ) {
    uint8_t ver = PCD_ReadRegister(spi, VersionReg);
    if (ver != 0x92 && ver != 0x91) {
        xsTrace("<error>Is the MFRC522 connected? If yes, recheck the wiring.\n");
        return false;
    }
    return true;
}

void PCD_WriteRegister ( spi_device_handle_t spi, uint8_t Register, uint8_t value ) {
    esp_err_t ret;
    uint8_t reg = Register;
    uint8_t val = value;
    static spi_transaction_t t;
    memset( & t, 0, sizeof(t)); //Zero out the transaction
    t.flags = SPI_TRANS_USE_TXDATA;
    t.length = 16;
    t.tx_data[0] = reg;
    t.tx_data[1] = val;
    ret = spi_device_queue_trans(spi, & t, 10);
    assert(ret == ESP_OK);
    spi_transaction_t * rtrans;
    ret = spi_device_get_trans_result(spi, & rtrans, 10);
    assert(ret == ESP_OK);
}

void PCD_WriteRegisterMany ( spi_device_handle_t spi, uint8_t Register, uint8_t count, uint8_t * values ) {
    esp_err_t ret;
    uint8_t total[count + 1];
    total[0] = Register;

    for (int i = 1; i <= count; ++i)
        total[i] = values[i - 1];

    static spi_transaction_t t1;
    memset( & t1, 0, sizeof(t1)); //Zero out the transaction
    t1.length = 8 * (count + 1);
    t1.tx_buffer = total;

    ret = spi_device_transmit(spi, & t1);
    assert(ret == ESP_OK);
}

uint8_t PCD_ReadRegister ( spi_device_handle_t spi, uint8_t Register ) {
    esp_err_t ret;
    uint8_t reg = Register | 0x80;
    uint8_t val;
    spi_transaction_t t;
    memset( & t, 0, sizeof(t)); //Zero out the transaction
    t.length = 8;
    t.tx_buffer = & reg;
    t.rx_buffer = & val;
    gpio_set_level(MFRC522_CS, 0);
    ret = spi_device_transmit(spi, & t);
    assert(ret == ESP_OK);
    t.tx_buffer = (uint8_t * ) 0;
    ret = spi_device_transmit(spi, & t);
    gpio_set_level(MFRC522_CS, 1);

    assert(ret == ESP_OK);

    return val;
}

void PCD_ReadRegisterMany ( spi_device_handle_t spi, uint8_t Register, uint8_t count, uint8_t * values, uint8_t rxAlign ) {
    if (count == 0)
        return;

    esp_err_t ret;
    uint8_t reg = 0x80 | Register; // MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
    spi_transaction_t t;
    memset( & t, 0, sizeof(t)); //Zero out the transaction
    t.length = 128; //8
    t.rxlength = 8 * count;
    t.tx_buffer = & reg;
    t.rx_buffer = values;
    gpio_set_level(MFRC522_CS, 0);
    ret = spi_device_transmit(spi, & t);
    assert(ret == ESP_OK);
    t.tx_buffer = (uint8_t * ) 0;
    ret = spi_device_transmit(spi, & t);
    gpio_set_level(MFRC522_CS, 1);
    assert(ret == ESP_OK);
}

void PCD_ClearRegisterBitMask ( spi_device_handle_t spi, uint8_t reg, uint8_t mask ) {
    uint8_t tmp;
    tmp = PCD_ReadRegister(spi, reg);
    PCD_WriteRegister(spi, reg, tmp & (~mask)); // clear bit mask
}

void PCD_SetRegisterBitMask ( spi_device_handle_t spi, uint8_t reg, uint8_t mask ) {
    uint8_t tmp;
    tmp = PCD_ReadRegister(spi, reg);
    PCD_WriteRegister(spi, reg, tmp | mask);
}

uint8_t PICC_HaltA ( spi_device_handle_t spi ) {
    uint8_t result;
    uint8_t buffer[4];

    // Build command buffer
    buffer[0] = PICC_CMD_HLTA;
    buffer[1] = 0;

    // Calculate CRC_A
    result = PCD_CalculateCRC(spi, buffer, 2, & buffer[2]);
    if (result != STATUS_OK)
        return result;

    result = PCD_TransceiveData(spi, buffer, sizeof(buffer), NULL, 0, NULL, 0, false);
    if (result == STATUS_TIMEOUT)
        return STATUS_OK;

    if (result == STATUS_OK) // That is ironically NOT ok in this case ;-)
        return STATUS_ERROR;

    return result;
}

void PCD_StopCrypto1 ( spi_device_handle_t spi ) {
    PCD_ClearRegisterBitMask(spi, Status2Reg, 0x08);
}

bool PCD_Init ( spi_device_handle_t spi, xsMachine *the ) {
    bool status = PCD_Version(spi, the);
    if ( status ) {
        gpio_pad_select_gpio(MFRC522_RST);
        gpio_set_direction(MFRC522_RST, GPIO_MODE_OUTPUT);

        // Hard Reset RFID
        gpio_set_level(MFRC522_RST, 0);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_set_level(MFRC522_RST, 1);
        vTaskDelay(50 / portTICK_PERIOD_MS);

        // Reset baud rates
        PCD_WriteRegister(spi, TxModeReg, 0x00);
        PCD_WriteRegister(spi, RxModeReg, 0x00);

        // Reset ModWidthReg
        PCD_WriteRegister(spi, ModWidthReg, 0x26);

        PCD_WriteRegister(spi, TModeReg, 0x80);
        PCD_WriteRegister(spi, TPrescalerReg, 0xA9);
        PCD_WriteRegister(spi, TReloadRegH, 0x03);
        PCD_WriteRegister(spi, TReloadRegL, 0xE8);
        PCD_WriteRegister(spi, TxASKReg, 0x40);
        PCD_WriteRegister(spi, ModeReg, 0x3D);

        PCD_AntennaOn(spi);
    }
    return status;
}

void PCD_AntennaOn ( spi_device_handle_t spi ) {
    uint8_t value = PCD_ReadRegister(spi, TxControlReg);
    if ((value & 0x03) != 0x03) {
        PCD_WriteRegister(spi, TxControlReg, value | 0x03);
    }
}

bool PICC_IsNewCardPresent ( spi_device_handle_t spi ) {
    static uint8_t bufferATQA[4] = {
        0, 0, 0, 0
    };

    uint8_t bufferSize = sizeof(bufferATQA);

    // Reset baud rates
    PCD_WriteRegister(spi, TxModeReg, 0x00);
    PCD_WriteRegister(spi, RxModeReg, 0x00);

    // Reset ModWidthReg
    PCD_WriteRegister(spi, ModWidthReg, 0x26);

    uint8_t result = PICC_RequestA(spi, bufferATQA, & bufferSize);
    return (result == STATUS_OK || result == STATUS_COLLISION);
}

PICC_Type PICC_GetType ( uint8_t sak ) {
    sak &= 0x7F;
    switch (sak) {
        case 0x04:
            return PICC_TYPE_NOT_COMPLETE; // UID not complete
        case 0x09:
            return PICC_TYPE_MIFARE_MINI;
        case 0x08:
            return PICC_TYPE_MIFARE_1K;
        case 0x18:
            return PICC_TYPE_MIFARE_4K;
        case 0x00:
            return PICC_TYPE_MIFARE_UL;
        case 0x10:
        case 0x11:
            return PICC_TYPE_MIFARE_PLUS;
        case 0x01:
            return PICC_TYPE_TNP3XXX;
        case 0x20:
            return PICC_TYPE_ISO_14443_4;
        case 0x40:
            return PICC_TYPE_ISO_18092;
        default:
            return PICC_TYPE_UNKNOWN;
    }
}

uint8_t PICC_RequestA ( spi_device_handle_t spi, uint8_t * bufferATQA, uint8_t * bufferSize ) {
    return PICC_REQA_or_WUPA(spi, PICC_CMD_REQA, bufferATQA, bufferSize);
}

uint8_t PICC_REQA_or_WUPA ( spi_device_handle_t spi, uint8_t command, uint8_t * bufferATQA, uint8_t * bufferSize ) {
    uint8_t validBits, status;

    if (bufferATQA == NULL || * bufferSize < 2)
        return STATUS_NO_ROOM;

    PCD_ClearRegisterBitMask(spi, CollReg, 0x80);
    validBits = 7;
    status = PCD_TransceiveData(spi, & command, 1, bufferATQA, bufferSize, & validBits, 0, false);

    if (status != STATUS_OK)
        return status;

    if ( * bufferSize != 2 || validBits != 0)
        return STATUS_ERROR;

    return STATUS_OK;
}

uint8_t PCD_TransceiveData ( spi_device_handle_t spi,
    uint8_t * sendData,  uint8_t sendLen,
    uint8_t * backData, uint8_t * backLen,
    uint8_t * validBits, uint8_t rxAlign,
    bool checkCRC
) {
    uint8_t waitIRq = 0x30;
    return PCD_CommunicateWithPICC(spi, PCD_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
}

uint8_t PCD_CommunicateWithPICC (spi_device_handle_t spi,
    uint8_t command, uint8_t waitIRq,
    uint8_t * sendData, uint8_t sendLen,
    uint8_t * backData, uint8_t * backLen,
    uint8_t * validBits, uint8_t rxAlign,
    bool checkCRC
) {
    uint8_t txLastBits = validBits ? * validBits : 0;
    uint8_t bitFraming = (rxAlign << 4) + txLastBits;

    PCD_WriteRegister(spi, CommandReg, PCD_Idle);
    PCD_WriteRegister(spi, ComIrqReg, 0x7F);
    PCD_WriteRegister(spi, FIFOLevelReg, 0x80);
    int sendData_l = 0;

    for (sendData_l = 0; sendData_l < sendLen; sendData_l++)
        PCD_WriteRegister(spi, FIFODataReg, sendData[sendData_l]);

    PCD_WriteRegister(spi, BitFramingReg, bitFraming);
    PCD_WriteRegister(spi, CommandReg, command);
    if (command == PCD_Transceive)
        PCD_SetRegisterBitMask(spi, BitFramingReg, 0x80);

    int i;
    uint8_t n, data;
    for (i = 20000; i > 0; i--) {
        n = PCD_ReadRegister(spi, ComIrqReg);
        if (n & waitIRq)
            break;
        if (n & 0x01)
            return STATUS_TIMEOUT;
    }

    if (i == 0)
        return STATUS_TIMEOUT;

    uint8_t errorRegValue = PCD_ReadRegister(spi, ErrorReg);
    if (errorRegValue & 0x13)
        return STATUS_ERROR;

    uint8_t _validBits = 0, h;

    if (backData && backLen) {
        h = PCD_ReadRegister(spi, FIFOLevelReg);
        if (h > * backLen)
            return STATUS_NO_ROOM;

        *backLen = h;

        int k;
        for (k = 0; k < h; k++)
            *(backData + k) = PCD_ReadRegister(spi, FIFODataReg);

        _validBits = PCD_ReadRegister(spi, ControlReg) & 0x07;
        if (validBits)
            *validBits = _validBits;

        int i;
    }

    if (errorRegValue & 0x08)
        return STATUS_COLLISION;

    return STATUS_OK;
}

uint8_t PICC_ReadCardSerial ( spi_device_handle_t spi ) {
    uint8_t result = PICC_Select(spi, &uid, 0);
    return (result == STATUS_OK);
}

uint8_t PICC_Select ( spi_device_handle_t spi, Uid * uid, uint8_t validBits ) {
    uint8_t uidComplete;
    uint8_t selectDone;
    uint8_t useCascadeTag;
    uint8_t cascadeLevel = 1;
    uint8_t result;
    uint8_t count;
    uint8_t checkBit;
    uint8_t index;
    uint8_t uidIndex;
    int8_t currentLevelKnownBits;
    uint8_t buffer[9];
    uint8_t bufferUsed;
    uint8_t rxAlign;
    uint8_t txLastBits;
    uint8_t * responseBuffer;
    uint8_t responseLength;

    if (validBits > 80)
        return STATUS_INVALID;

    PCD_ClearRegisterBitMask(spi, CollReg, 0x80);

    uidComplete = false;
    while (!uidComplete) {
        switch (cascadeLevel) {
            case 1:
                buffer[0] = PICC_CMD_SEL_CL1;
                uidIndex = 0;
                useCascadeTag = validBits && uid -> size > 4;
                break;
            case 2:
                buffer[0] = PICC_CMD_SEL_CL2;
                uidIndex = 3;
                useCascadeTag = validBits && uid -> size > 7;
                break;
            case 3:
                buffer[0] = PICC_CMD_SEL_CL3;
                uidIndex = 6;
                useCascadeTag = false;
                break;
            default:
                return STATUS_INTERNAL_ERROR;
                break;
        }

        currentLevelKnownBits = validBits - (8 * uidIndex);
        if (currentLevelKnownBits < 0) {
            currentLevelKnownBits = 0;
        }

        if (useCascadeTag) {
            buffer[index++] = PICC_CMD_CT;
        }
        uint8_t bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0);

        if (bytesToCopy) {
            uint8_t maxBytes = useCascadeTag ? 3 : 4;
            if (bytesToCopy > maxBytes)
                bytesToCopy = maxBytes;
            for (count = 0; count < bytesToCopy; count++)
                buffer[index] = uid -> uidByte[uidIndex + count];
        }

        if (useCascadeTag)
            currentLevelKnownBits += 8;

        selectDone = false;
        while (!selectDone) {
            if (currentLevelKnownBits >= 32) {
                buffer[1] = 0x70;
                buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
                result    = PCD_CalculateCRC(spi, buffer, 7, & buffer[7]);

                if (result != STATUS_OK)
                    return result;

                txLastBits = 0;
                bufferUsed = 9;
                responseBuffer = & buffer[6];
                responseLength = 3;
            } else {
                txLastBits = currentLevelKnownBits % 8;
                count = currentLevelKnownBits / 8;
                index = 2 + count;
                buffer[1] = (index << 4) + txLastBits;
                bufferUsed = index + (txLastBits ? 1 : 0);
                responseBuffer = & buffer[index];
                responseLength = sizeof(buffer) - index;
            }

            rxAlign = txLastBits;
            PCD_WriteRegister(spi, BitFramingReg, (rxAlign << 4) + txLastBits);

            result = PCD_TransceiveData(spi, buffer, bufferUsed, responseBuffer, & responseLength, & txLastBits, rxAlign, 0);

            if (result == STATUS_COLLISION) {
                uint8_t valueOfCollReg = PCD_ReadRegister(spi, CollReg);
                if (valueOfCollReg & 0x20)
                    return STATUS_COLLISION;

                uint8_t collisionPos = valueOfCollReg & 0x1F;
                if (collisionPos == 0)
                    collisionPos = 32;

                if (collisionPos <= currentLevelKnownBits)
                    return STATUS_INTERNAL_ERROR;

                currentLevelKnownBits = collisionPos;
                count = currentLevelKnownBits % 8;
                checkBit = (currentLevelKnownBits - 1) % 8;
                index = 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0);
                buffer[index] |= (1 << checkBit);
            } else if (result != STATUS_OK) {
                return result;
            } else {
                if (currentLevelKnownBits >= 32) {
                    selectDone = true;
                } else {
                    currentLevelKnownBits = 32;
                }
            }
        }

        index = (buffer[2] == PICC_CMD_CT) ? 3 : 2;
        bytesToCopy = (buffer[2] == PICC_CMD_CT) ? 3 : 4;

        for (count = 0; count < bytesToCopy; count++) {
            uid -> uidByte[uidIndex + count] = buffer[index++];
        }

        if (responseLength != 3 || txLastBits != 0)
            return STATUS_ERROR;

        result = PCD_CalculateCRC(spi, responseBuffer, 1, & buffer[2]);
        if (result != STATUS_OK)
            return result;

        if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2]))
            return STATUS_CRC_WRONG;

        if (responseBuffer[0] & 0x04) {
            cascadeLevel++;
        } else {
            uidComplete = true;
            uid->sak = responseBuffer[0];
        }
    }

    uid->size = 3 * cascadeLevel + 1;

    return STATUS_OK;
}

bool PCD_CalculateCRC ( spi_device_handle_t spi, uint8_t * data, uint8_t length, uint8_t * result ) {
    PCD_WriteRegister(spi, CommandReg, PCD_Idle);
    PCD_WriteRegister(spi, DivIrqReg, 0x04);
    PCD_WriteRegister(spi, FIFOLevelReg, 0x80);
    PCD_WriteRegisterMany(spi, FIFODataReg, length, data);
    PCD_WriteRegister(spi, CommandReg, PCD_CalcCRC);

    for (uint16_t i = 5000; i > 0; i--) {
        uint8_t n = PCD_ReadRegister(spi, DivIrqReg);
        if (n & 0x04) {
            PCD_WriteRegister(spi, CommandReg, PCD_Idle);
            result[0] = PCD_ReadRegister(spi, CRCResultRegL);
            result[1] = PCD_ReadRegister(spi, CRCResultRegH);
            return STATUS_OK;
        }
    }
    return STATUS_TIMEOUT;
}

uint8_t PCD_Authenticate ( spi_device_handle_t spi, uint8_t command, uint8_t blockAddr, MIFARE_Key * key, Uid * uid ) {
  uint8_t waitIRq = 0x10;
  uint8_t status;

  uint8_t sendData[12];

  sendData[0] = command;
  sendData[1] = blockAddr;

  for (uint8_t i = 0; i < MF_KEY_SIZE; i++) {
    sendData[2 + i] = key->keyByte[i];
  }

  for (uint8_t i = 0; i < 4; i++) {
    sendData[8 + i] = uid->uidByte[i + uid->size - 4];
  }

  status = PCD_CommunicateWithPICC(spi, PCD_MFAuthent, waitIRq, & sendData[0], sizeof(sendData), NULL, 0, NULL, 0, false);

  return status;
}

bool MIFARE_Read ( spi_device_handle_t spi, uint8_t blockAddr, uint8_t *buffer, uint8_t *bufferSize ) {
  uint8_t result;

  if (buffer == NULL || *bufferSize < 18) {
    printf("MFRC522 MIFARE_READ : Status no room\n");
    return STATUS_NO_ROOM;
  }

  buffer[0] = PICC_CMD_MF_READ;
  buffer[1] = blockAddr;

  result = PCD_CalculateCRC(spi, buffer, 2, & buffer[2]);

  if (result != STATUS_OK)
    return result;

  return PCD_TransceiveData(spi, buffer, 4, buffer, bufferSize, NULL, 0, true);
}

uint8_t MIFARE_Write ( spi_device_handle_t spi, uint8_t blockAddr, uint8_t * buffer, uint8_t bufferSize ) {
  uint8_t result;

  if (buffer == NULL || bufferSize < 16)
    return STATUS_INVALID;

  uint8_t cmdBuffer[2];

  cmdBuffer[0] = PICC_CMD_MF_WRITE;
  cmdBuffer[1] = blockAddr;

  result = PCD_MIFARE_Transceive(spi, cmdBuffer, 2, false);
  if (result != STATUS_OK)
    return result;

  result = PCD_MIFARE_Transceive(spi, buffer, bufferSize, false);
  if (result != STATUS_OK)
    return result;

  return STATUS_OK;
}

uint8_t PCD_MIFARE_Transceive ( spi_device_handle_t spi, uint8_t * sendData, uint8_t sendLen, bool acceptTimeout ) {
  uint8_t result;

  uint8_t cmdBuffer[18];
  if (sendData == NULL || sendLen > 16)
    return STATUS_INVALID;

  memcpy(cmdBuffer, sendData, sendLen);

  result = PCD_CalculateCRC(spi, cmdBuffer, sendLen, & cmdBuffer[sendLen]);

  if (result != STATUS_OK)
    return result;

  sendLen += 2;

  uint8_t waitIRq = 0x30;
  uint8_t cmdBufferSize = sizeof(cmdBuffer);

  uint8_t validBits = 0;

  result = PCD_CommunicateWithPICC(spi, PCD_Transceive, waitIRq, cmdBuffer, sendLen, cmdBuffer, & cmdBufferSize, & validBits, 0, false);

  if (acceptTimeout && result == STATUS_TIMEOUT)
    return STATUS_OK;

  if (result != STATUS_OK)
    return result;

  if (cmdBufferSize != 1 || validBits != 4)
    return STATUS_ERROR;

  if (cmdBuffer[0] != MF_ACK)
    return STATUS_MIFARE_NACK;

  return STATUS_OK;
}

void GetStatusCodeName ( uint8_t code ) {
  switch (code) {
      case STATUS_OK:
        printf("Success.\n");
        break;
      case STATUS_ERROR:
        printf("Error in communication.\n");
        break;
      case STATUS_COLLISION:
        printf("Collission detected.\n");
        break;
      case STATUS_TIMEOUT:
        printf("Timeout in communication.\n");
        break;
      case STATUS_NO_ROOM:
        printf("A buffer is not big enough.\n");
        break;
      case STATUS_INTERNAL_ERROR:
        printf("Internal error in the code. Should not happen.\n");
        break;
      case STATUS_INVALID:
        printf("Invalid argument.\n");
        break;
      case STATUS_CRC_WRONG:
        printf("The CRC_A does not match.\n");
        break;
      case STATUS_MIFARE_NACK:
        printf("A MIFARE PICC responded with NAK.\n");
        break;
      default:
        printf("Unknown error\n");
  }
}