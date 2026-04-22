#include "langdetect/utf8.hpp"


// Decode a UTF-8 string into Unicode code points
std::vector<uint32_t> decode(std::string_view data){ // string_view keeps the input read-only
    std::vector<uint32_t> decoded_data;
    uint8_t byte;
    size_t i = 0;
    while(i < data.size()){
        byte = static_cast<uint8_t>(data[i]);
        if((byte & 0x80) == 0x00){ // One byte ASCII path
            decoded_data.push_back(byte & 0x7F);
            i++;
        }
        else if(((byte & 0xE0) == 0xC0) && data.size() - i >= 2){
            // Two byte sequence, five bits from the first byte and six from the next
            decoded_data.push_back((byte & 0x1F)<<6 | (static_cast<uint8_t>(data[i+1]) & 0x3F));
            i += 2;
        }
        else if(((byte & 0xF0) == 0xE0) && data.size() - i >= 3){
            // Three byte sequence with one more continuation byte
            decoded_data.push_back(((byte & 0x0F) << 12) |((static_cast<uint8_t>(data[i+1]) & 0x3F) << 6) |(static_cast<uint8_t>(data[i+2]) & 0x3F));
            i += 3;
        }
        else if(((byte & 0xF8) == 0xF0) && data.size() - i >= 4){
            decoded_data.push_back(((byte & 0x07) << 18) |((static_cast<uint8_t>(data[i+1]) & 0x3F) << 12) |((static_cast<uint8_t>(data[i+2]) & 0x3F) << 6) |(static_cast<uint8_t>(data[i+3]) & 0x3F));  
            i += 4;
        }
        else
            i++;
    }
    return decoded_data;
}
