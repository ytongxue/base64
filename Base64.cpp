#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "Base64.h"
#include "utils.h"


using namespace Base64;
using namespace std;

const static char symbolTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

Decoder::Decoder() {
    m_remainingSymbolCount = 0;
    bzero(m_cpRemainingSymbols, sizeof(m_cpRemainingSymbols));
}

uint8_t Decoder::symbol2value(char symbol) {
    uint8_t value = 0xff;
    for (uint8_t i = 0; i < sizeof(symbolTable); i++) {
        if (symbolTable[i] == symbol) {
            value = i;
            break;
        }
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
    for (size_t i = 0; i < inputSize / 4; i++) {
        uint32_t u24 = 0;
        for (size_t j = 0; j < 4; j++) {
            char symbol = u8InputBuffer[i * 4 + j];
            uint8_t u6 = 0;
            if (symbol == '=') {
                padding += 1;
            } else {
                u6 = symbol2value(symbol);
                if (u6 >= 64) {
                    LOG_ERROR("symbol2value error: %c => 0x%02x\n", symbol, u6);
                    u6 = 0;
                }
            }
            u24 = (u24 << 6) + u6;
        }
        for (size_t k = 0; (k < 3) && (outputSize + k < outputBufferSize); k++) {
            u8OutputBuffer[outputSize + k] = (u24 >> (2 - k) * 8) & 0xff;
        }
        outputSize += 3;
        if (padding == 1) {
            outputSize -= 1;
        } else if (padding == 2) {
            outputSize -= 2;
        }
    }
    m_remainingSymbolCount = inputSize % 4;
    if (m_remainingSymbolCount) {
        memcpy(m_cpRemainingSymbols, u8InputBuffer + inputSize - m_remainingSymbolCount, m_remainingSymbolCount);
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
    //printf("m_remainingByteCount: %d\n", m_remainingByteCount);
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
    //printf("u24: 0x%08x\n", u24);
    encode(u24, (char *)u8OutputBuffer);
    for (int i = (m_remainingByteCount == 1 ? 2 : 3); i < 4; i++) {
        u8OutputBuffer[i] = '=';
    }
    return 4;
}
