#ifndef __BASE64_H__
#define __BASE64_H__

#include <string>

namespace Base64 {
    class CodecInterface {
        public:
            virtual size_t feed(uint8_t u8InputBuffer[], size_t inputSize,
                                uint8_t u8OutputBuffer[], size_t outputBufferSize) = 0;
            virtual size_t finish(uint8_t u8OutputBuffer[], size_t outputBufferSize) = 0;
            virtual ~CodecInterface() {};
    };
    class Decoder : public CodecInterface {
        public:
            Decoder();
            virtual size_t feed(uint8_t u8InputBuffer[], size_t inputSize,
                                uint8_t u8OutputBuffer[], size_t outputBufferSize);
            virtual size_t finish(uint8_t u8OutputBuffer[], size_t outputBufferSize);
        private:
            uint8_t symbol2value(char symbol);
            char m_cpRemainingSymbols[4];
            size_t m_remainingSymbolCount;
    };
    class Encoder : public CodecInterface {
        public:
            Encoder();
            virtual size_t feed(uint8_t u8InputBuffer[], size_t inputSize,
                                uint8_t u8OutputBuffer[], size_t outputBufferSize);
            virtual size_t finish(uint8_t u8OutputBuffer[], size_t outputBufferSize);
        private:
            void encode(uint32_t u24, char u8Buffer[]);
            uint8_t m_u8RemainingBuffer[4];
            size_t m_remainingByteCount;
    };
} //namespace
#endif // __BASE64_H__
