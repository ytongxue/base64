#ifndef __BASE64_H__
#define __BASE64_H__

#include <string>

namespace Base64 {
    class Decoder {
        public:
            Decoder();
            size_t feed(const std::string &str, uint8_t u8OutputBuffer[], size_t outputBufferSize);
        private:
            uint8_t symbol2value(char symbol);
            std::string m_strRemaining;
    };
    class Encoder {
        public:
            Encoder();
            std::string feed(uint8_t u8Buffer[], size_t size);
            std::string finish();
        private:
            std::string encode(uint32_t u24);
            uint8_t m_u8RemainingBuffer[4];
            size_t m_remainingByteCount;
    };
} //namespace
#endif // __BASE64_H__
