#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "Base64.h"
#include "utils.h"


using namespace Base64;
using namespace std;

// value to symbol table
const static char symbolTable[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// symbol to value table
const static uint8_t valueTable[] = {
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*'+'*/0x3e,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*'/'*/0x3f,
    /*'0'*/0x34, /*'1'*/0x35, /*'2'*/0x36, /*'3'*/0x37,
    /*'4'*/0x38, /*'5'*/0x39, /*'6'*/0x3a, /*'7'*/0x3b,
    /*'8'*/0x3c, /*'9'*/0x3d, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*'A'*/0x00, /*'B'*/0x01, /*'C'*/0x02,
    /*'D'*/0x03, /*'E'*/0x04, /*'F'*/0x05, /*'G'*/0x06,
    /*'H'*/0x07, /*'I'*/0x08, /*'J'*/0x09, /*'K'*/0x0a,
    /*'L'*/0x0b, /*'M'*/0x0c, /*'N'*/0x0d, /*'O'*/0x0e,
    /*'P'*/0x0f, /*'Q'*/0x10, /*'R'*/0x11, /*'S'*/0x12,
    /*'T'*/0x13, /*'U'*/0x14, /*'V'*/0x15, /*'W'*/0x16,
    /*'X'*/0x17, /*'Y'*/0x18, /*'Z'*/0x19, /*' '*/0xff,
    /*' '*/0xff, /*' '*/0xff, /*' '*/0xff, /*' '*/0xff,
    /*' '*/0xff, /*'a'*/0x1a, /*'b'*/0x1b, /*'c'*/0x1c,
    /*'d'*/0x1d, /*'e'*/0x1e, /*'f'*/0x1f, /*'g'*/0x20,
    /*'h'*/0x21, /*'i'*/0x22, /*'j'*/0x23, /*'k'*/0x24,
    /*'l'*/0x25, /*'m'*/0x26, /*'n'*/0x27, /*'o'*/0x28,
    /*'p'*/0x29, /*'q'*/0x2a, /*'r'*/0x2b, /*'s'*/0x2c,
    /*'t'*/0x2d, /*'u'*/0x2e, /*'v'*/0x2f, /*'w'*/0x30,
    /*'x'*/0x31, /*'y'*/0x32, /*'z'*/0x33,
};

Decoder::Decoder() {
    m_remainingSymbolCount = 0;
    bzero(m_cpRemainingSymbols, sizeof(m_cpRemainingSymbols));
}

uint8_t Decoder::symbol2value(char symbol) {
    uint8_t value = 0xff;
    uint8_t symbolIndex = (uint8_t)symbol;
    //LOG_ERROR("%c => %d", symbol, symbolIndex);
    if (symbolIndex < sizeof(valueTable)) {
        value = valueTable[symbolIndex];
    }
    return value;
}

size_t Decoder::feed(uint8_t u8InputBuffer[], size_t inputSize,
                     uint8_t u8OutputBuffer[], size_t outputBufferSize) {
    size_t outputSize = 0;
    size_t padding = 0;
    if (inputSize / 4 * 3 > outputBufferSize) {
        LOG_ERROR("outputBufferSize too small\n");
        return 0;
    }
    size_t validSymbolCount = 0;
    uint32_t u24 = 0;
    int remainingOffset = 0;
    size_t totalSymbolCount = m_remainingSymbolCount + inputSize;
    if (totalSymbolCount < 4) {
        memcpy(m_cpRemainingSymbols + m_remainingSymbolCount, u8InputBuffer, inputSize);
        m_remainingSymbolCount += inputSize;
        return 0;
    }
    for (size_t i = 0; i < totalSymbolCount; i++) {
        char symbol = '*';
        if (i < m_remainingSymbolCount) {
            symbol = m_cpRemainingSymbols[i];
        } else {
            symbol = u8InputBuffer[i - m_remainingSymbolCount];
        }
        if (validSymbolCount == 0) {
            remainingOffset = i - m_remainingSymbolCount;
        }
        uint8_t u6 = 0;
        if (symbol == '=') {
            padding++;
        } else {
            u6 = symbol2value(symbol);
            if (u6 == 0xff) { //invalid symbol
                if (symbol != '\n') {
                    LOG_ERROR("Invalid symbol: '%c'", symbol);
                }
                continue;
            }
        }
        u24 = (u24 << 6) + u6;
        validSymbolCount++;
        if (validSymbolCount == 4) {
            uint8_t *pu8 = (uint8_t *)&u24;
            for (size_t k = 0; (k < 3) && (outputSize + k < outputBufferSize); k++) {
                u8OutputBuffer[outputSize + k] = pu8[2 - k]; // little-endian, read from the higher address
            }
            outputSize += 3;
            u24 = 0;
            validSymbolCount = 0;
            remainingOffset = 0;
        }
    }
    if (padding == 1) {
        outputSize -= 1;
    } else if (padding == 2) {
        outputSize -= 2;
    }
    if (remainingOffset > 0) {
        m_remainingSymbolCount = inputSize - remainingOffset;
    } else {
        m_remainingSymbolCount = 0;
    }
    if (m_remainingSymbolCount) {
        memcpy(m_cpRemainingSymbols,
               u8InputBuffer + remainingOffset,
               m_remainingSymbolCount);
    }
    return outputSize;
}

size_t Decoder::finish(uint8_t u8OutputBuffer[], size_t outputBufferSize) {
    return 0;
}

// Encoder

Encoder::Encoder() {
    bzero(&m_u8RemainingBuffer, sizeof(m_u8RemainingBuffer));
    m_remainingByteCount = 0;
}

void Encoder::encode(uint32_t u24, char u8Buffer[]) {
    for (int k = 0; k < 4; k++) {
        uint8_t u6 = (u24 >> ((3 - k) * 6)) & 0x3f;
        char ch = symbolTable[u6];
        u8Buffer[k] = ch;
    }
}

size_t Encoder::feed(uint8_t u8InputBuffer[], size_t inputSize,
                     uint8_t u8OutputBuffer[], size_t outputBufferSize) {
    size_t dataLen = inputSize + m_remainingByteCount;
    size_t outputSize = 0;
    for (size_t i = 0; i < dataLen / 3; i++) {
        uint32_t u24 = 0;
        for (size_t j = 0; j < 3; j++) {
            uint8_t u8 = 0;
            if (i == 0 && j < m_remainingByteCount) {
                u8 = m_u8RemainingBuffer[j];
            } else {
                u8 = u8InputBuffer[i * 3 + j - m_remainingByteCount];
            }
            u24 = (u24 << 8) + u8;
        }
        encode(u24, (char *)u8OutputBuffer + outputSize);
        outputSize += 4;
    }
    m_remainingByteCount = dataLen % 3;
    if (m_remainingByteCount >= 3) {
        m_remainingByteCount = 0;
    }
    // save the remaining bytes
    if (m_remainingByteCount) {
        memcpy(m_u8RemainingBuffer, u8InputBuffer + dataLen - m_remainingByteCount, m_remainingByteCount);
    }
    return outputSize;
}

size_t Encoder::finish(uint8_t u8OutputBuffer[], size_t outputBufferSize) {
    if (m_remainingByteCount == 0) {
        return 0;
    }
    uint32_t u24 = 0;
    for (size_t i = 0; i < m_remainingByteCount; i++) {
        u24 = (m_u8RemainingBuffer[i] << ((2 - i) * 8)) + u24;
    }
    encode(u24, (char *)u8OutputBuffer);
    for (int i = (m_remainingByteCount == 1 ? 2 : 3); i < 4; i++) {
        u8OutputBuffer[i] = '=';
    }
    return 4;
}
