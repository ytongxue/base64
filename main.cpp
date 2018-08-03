#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "Base64.h"
#include "utils.h"

using namespace Base64;
using namespace std;

int main(int argc, char **argv) {
    FILE *fp = stdin;
    uint8_t u8Buffer[1024] = {0};
    Encoder encoder;
    Decoder decoder;
    string strEncoded;
    while (1) {
        int readLen = fread(u8Buffer, 1, sizeof(u8Buffer), fp);
        //printf("len: %d\n", len);
        if (readLen < 0) {
            LOG_ERROR("Failed to read, errno: %d, str: %s\n",
                    errno, strerror(errno));
            exit(1);
        }
        if (readLen == 0) {
            strEncoded += encoder.finish();
            break;
        } else {
            strEncoded += encoder.feed(u8Buffer, readLen);
        }
    }
    cout << "encoded: " << strEncoded << endl;
    uint8_t u8DecodedBuffer[2048];
    size_t decodedSize = decoder.feed(strEncoded, u8DecodedBuffer, sizeof(u8DecodedBuffer));
    if (decodedSize) {
        cout << "decoding finished" << endl;
        for (size_t i = 0; i < decodedSize; i++) {
            if (i > 0 && i % 16 == 0) {
                printf("\n");
            }
            printf("%02x ", u8DecodedBuffer[i]);
        }
        printf("\n");
    } else {
        cout << "decoding failed" << endl;
    }
    return 0;
}
