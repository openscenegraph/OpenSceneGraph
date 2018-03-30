#ifndef UTF8_STRING
#define UTF8_STRING

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>  // control characters


namespace utf8_string {
    inline std::string encode_control_char(unsigned int ctrl) {
        // see http://json.org
        std::ostringstream oss;

        if(ctrl == 8  || // \b
           ctrl == 9  || // \t
           ctrl == 10 || // \n
           ctrl == 12 || // \f
           ctrl == 13 || // \r
           ctrl == 27 || //
           ctrl == 34 || // \"
           ctrl == 47 // \/
           ) {
            oss << static_cast<unsigned char>(ctrl);
        }
        else {
            oss.fill('0');
            oss << "\\u" << std::setw(4) << std::hex << ctrl;
        }

        return oss.str();
    }


    inline bool is_valid_continuation_byte(unsigned int byte) {
        return ((byte & 0xC0) == 0x80);
    }


    inline int get_next_byte(std::string::const_iterator& iterator, std::string::const_iterator end_iterator) {
        if(iterator != end_iterator) {
            return *(++ iterator);
        }
        else {
            return 0; // invalid continuation byte
        }
    }


    // from http://en.wikipedia.org/wiki/UTF-8#Invalid_byte_sequences
    inline std::string encode_codepoint(unsigned code_point)
    {
        std::string output;

        if(code_point > 0x590 && code_point < 0x5F4) {
            return output;
        }

        // out of range
        if(code_point > 1114112) {
            return encode_codepoint(0xfffd);
        }

        if (code_point < 0x80) {
            output.push_back(code_point);
        }
        else if (code_point <= 0x7FF) {
            output.push_back((code_point >> 6) + 0xC0);
            output.push_back((code_point & 0x3F) + 0x80);
        }
        else if (code_point <= 0xFFFF) {
            output.push_back((code_point >> 12) + 0xE0);
            output.push_back(((code_point >> 6) & 0x3F) + 0x80);
            output.push_back((code_point & 0x3F) + 0x80);
        }
        else if (code_point <= 0x10FFFF) {
            output.push_back((code_point >> 18) + 0xF0);
            output.push_back(((code_point >> 12) & 0x3F) + 0x80);
            output.push_back(((code_point >> 6) & 0x3F) + 0x80);
            output.push_back((code_point & 0x3F) + 0x80);
        }
        return output;
    }

    // from http://en.wikipedia.org/wiki/UTF-8#Invalid_byte_sequences
    inline std::string clean_invalid(const std::string& input, const int replacement_codepoint=0xfffd) {
        unsigned int code_unit1, code_unit2, code_unit3, code_unit4;
        std::string output, replacement = encode_codepoint(replacement_codepoint);

        for(std::string::const_iterator iterator = input.begin() ; iterator != input.end() ; ++ iterator) {
            code_unit1 = *iterator;
            if (code_unit1 < 0x80) {
                if(std::iscntrl(code_unit1)) {
                    output += encode_control_char(code_unit1);
                }
                else {
                    output.push_back(code_unit1);
                }
            }
            else if (code_unit1 < 0xC2) {
                // continuation or overlong 2-byte sequence
                output += replacement;
            }
            else if (code_unit1 < 0xE0) {
                // 2-byte sequence
                code_unit2 = get_next_byte(iterator, input.end());

                if (!is_valid_continuation_byte(code_unit2)) {
                    output += replacement;
                    output += replacement;
                }
                else {
                    output += encode_codepoint((code_unit1 << 6) + code_unit2 - 0x3080);
                }
            }
            else if (code_unit1 < 0xF0) {
                // 3-byte sequence
                code_unit2 = get_next_byte(iterator, input.end());

                if (!is_valid_continuation_byte(code_unit2) ||
                    (code_unit1 == 0xE0 && code_unit2 < 0xA0)) /* overlong */ {
                    output += replacement;
                    output += replacement;
                }
                else {
                    code_unit3 = get_next_byte(iterator, input.end());

                    if (!is_valid_continuation_byte(code_unit3)) {
                        output += replacement;
                        output += replacement;
                        output += replacement;
                    }
                    else {
                        output += encode_codepoint((code_unit1 << 12) +
                                                   (code_unit2 << 6) +
                                                   code_unit3 - 0xE2080);
                    }
                }
            }
            else if (code_unit1 < 0xF5) {
                // 4-byte sequence
                code_unit2 = get_next_byte(iterator, input.end());
                if(!is_valid_continuation_byte(code_unit2) ||
                    (code_unit1 == 0xF0 && code_unit2 < 0x90) || /* overlong */
                    (code_unit1 == 0xF4 && code_unit2 >= 0x90)) {  /* > U+10FFFF */
                    output += replacement;
                    output += replacement;
                }
                else {
                    code_unit3 = get_next_byte(iterator, input.end());
                    if(!is_valid_continuation_byte(code_unit3)) {
                            output += replacement;
                            output += replacement;
                            output += replacement;
                    }
                    else {
                        code_unit4 = get_next_byte(iterator, input.end());
                        if(!is_valid_continuation_byte(code_unit4)) {
                            output += replacement;
                            output += replacement;
                            output += replacement;
                            output += replacement;
                        }
                        else {
                            output += encode_codepoint((code_unit1 << 18) +
                                                       (code_unit2 << 12) +
                                                       (code_unit3 << 6) +
                                                       code_unit4 - 0x3C82080);
                        }
                    }
                }
            }
            else {
                /* > U+10FFFF */
                output += replacement;
            }
        }
        return output;
    }


    inline std::string sanitize(const std::string& input) {
        return clean_invalid(input);
    }
}
#endif
