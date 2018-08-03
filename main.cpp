#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "Base64.h"
#include "utils.h"


using namespace Base64;
using namespace std;

enum WorkingMode {
    MODE_ENCODE,
    MODE_DECODE,
};

int main(int argc, char **argv) {
    uint8_t u8InputBuffer[1024] = {0};
    uint8_t u8OutputBuffer[1024 * 2] = {0};
    enum WorkingMode  mode = MODE_DECODE;
    CodecInterface *pCodec = NULL;
    size_t totalOutputLength = 0;
    size_t lineWidth = 76;
    if (mode == MODE_DECODE) {
        lineWidth = 0;
        pCodec = new Decoder();
    } else {
        pCodec = new Encoder();
    }
    while (1) {
        size_t outputSize = 0;
        int readLen = fread(u8InputBuffer, 1, sizeof(u8InputBuffer), stdin);
        if (readLen < 0) {
            LOG_ERROR("Failed to read, errno: %d, str: %s\n",
                    errno, strerror(errno));
            exit(1);
        }
        if (readLen == 0) { // EOF
            outputSize = pCodec->finish(u8OutputBuffer, sizeof(u8OutputBuffer));
        } else {
            outputSize = pCodec->feed(u8InputBuffer, readLen,
                                      u8OutputBuffer, sizeof(u8OutputBuffer));
        }
        for (size_t i = 0; i < outputSize; i++) {
            if ((totalOutputLength > 0)
                && (lineWidth > 0)
                && (totalOutputLength % lineWidth == 0)) {
                printf("\n");
            }
            printf("%c", u8OutputBuffer[i]);
            totalOutputLength++;
        }
        if (readLen == 0) { // EOF
            if (mode == MODE_ENCODE) {
                printf("\n");
            }
            break;
        }
    }
    if (pCodec) delete pCodec;
    return 0;
}
