/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

/* This file is derived from the libb64 project which itself was released to public domain.
 * For details, see http://sourceforge.net/projects/libb64. Original code by Chris Venter
 * c++ wrapper for a base64 encoding and decoding algorithm
*/

#include <osgDB/ConvertBase64>

#include <sstream>
#include <string.h>

namespace osgDB
{

    const int CHARS_PER_LINE = 72;

    int base64_decode_value(char value_in)
    {
        static const signed char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
        static const char decoding_size = sizeof(decoding);
        value_in -= 43;
        if (value_in < 0 || value_in >= decoding_size) return -1;
        return decoding[(int)value_in];
    }

    int base64_decode_block(const char* code_in, const int length_in, char* plaintext_out, base64_decodestate* state_in)
    {
        const char* codechar = code_in;
        char* plainchar = plaintext_out;
        char fragment;

        *plainchar = state_in->plainchar;

        switch (state_in->step)
        {
            while (1)
            {
        case step_a:
                do {
                    if (codechar == code_in+length_in)
                    {
                        state_in->step = step_a;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar    = (fragment & 0x03f) << 2;
        case step_b:
                do {
                    if (codechar == code_in+length_in)
                    {
                        state_in->step = step_b;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar++ |= (fragment & 0x030) >> 4;
                *plainchar    = (fragment & 0x00f) << 4;
        case step_c:
                do {
                    if (codechar == code_in+length_in)
                    {
                        state_in->step = step_c;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar++ |= (fragment & 0x03c) >> 2;
                *plainchar    = (fragment & 0x003) << 6;
        case step_d:
                do {
                    if (codechar == code_in+length_in)
                    {
                        state_in->step = step_d;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar++   |= (fragment & 0x03f);
            }
        }
        /* control should not reach here */
        return plainchar - plaintext_out;
    }

    char base64_encode_value(char value_in)
    {
        static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        if (value_in > 63) return '=';
        return encoding[(int)value_in];
    }

    int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in)
    {
        const char* plainchar = plaintext_in;
        const char* const plaintextend = plaintext_in + length_in;
        char* codechar = code_out;
        char result;
        char fragment;

        result = state_in->result;

        switch (state_in->step)
        {
            while (1)
            {
        case step_A:
                if (plainchar == plaintextend)
                {
                    state_in->result = result;
                    state_in->step = step_A;
                    return codechar - code_out;
                }
                fragment = *plainchar++;
                result = (fragment & 0x0fc) >> 2;
                *codechar++ = base64_encode_value(result);
                result = (fragment & 0x003) << 4;
        case step_B:
                if (plainchar == plaintextend)
                {
                    state_in->result = result;
                    state_in->step = step_B;
                    return codechar - code_out;
                }
                fragment = *plainchar++;
                result |= (fragment & 0x0f0) >> 4;
                *codechar++ = base64_encode_value(result);
                result = (fragment & 0x00f) << 2;
        case step_C:
                if (plainchar == plaintextend)
                {
                    state_in->result = result;
                    state_in->step = step_C;
                    return codechar - code_out;
                }
                fragment = *plainchar++;
                result |= (fragment & 0x0c0) >> 6;
                *codechar++ = base64_encode_value(result);
                result  = (fragment & 0x03f) >> 0;
                *codechar++ = base64_encode_value(result);

                ++(state_in->stepcount);
                if (state_in->stepcount == CHARS_PER_LINE/4)
                {
                    *codechar++ = '\n';
                    state_in->stepcount = 0;
                }
            }
        }
        /* control should not reach here */
        return codechar - code_out;
    }

    int base64_encode_blockend(char* code_out, base64_encodestate* state_in)
    {
        char* codechar = code_out;

        switch (state_in->step)
        {
        case step_B:
            *codechar++ = base64_encode_value(state_in->result);
            *codechar++ = '=';
            *codechar++ = '=';
            break;
        case step_C:
            *codechar++ = base64_encode_value(state_in->result);
            *codechar++ = '=';
            break;
        case step_A:
            break;
        }
        *codechar++ = '\n';

        return codechar - code_out;
    }

    int Base64encoder::encode(char value_in)
    {
        return base64_encode_value(value_in);
    }

    int Base64encoder::encode(const char* code_in, const int length_in, char* plaintext_out)
    {
        return base64_encode_block(code_in, length_in, plaintext_out, &_state);
    }

    int Base64encoder::encode_end(char* plaintext_out)
    {
        return base64_encode_blockend(plaintext_out, &_state);
    }

    void Base64encoder::encode(std::istream& istream_in, std::ostream& ostream_in)
    {
        base64_init_encodestate(&_state);

        const int N = _buffersize;
        char* plaintext = new char[N];
        char* code = new char[2*N];
        int plainlength;
        int codelength;

        do
        {
            istream_in.read(plaintext, N);
            plainlength = istream_in.gcount();

            codelength = encode(plaintext, plainlength, code);
            ostream_in.write(code, codelength);
        }
        while (istream_in.good() && plainlength > 0);

        codelength = encode_end(code);
        ostream_in.write(code, codelength);

        base64_init_encodestate(&_state);

        delete [] code;
        delete [] plaintext;
    }

    void Base64encoder::encode(const char* chars_in, int length_in, std::string& code_out)
    {
        std::stringstream stream_out;
        {
            std::stringstream stream_in;
            {
                stream_in<<std::string(chars_in, length_in);
            }
            stream_in.seekg(0, stream_in.beg);
            encode(stream_in, stream_out);
        }
        stream_out.seekg (0, stream_out.beg);
        code_out = stream_out.str();
    }

    int Base64decoder::decode(char value_in)
    {
        return base64_decode_value(value_in);
    }

    int Base64decoder::decode(const char* code_in, const int length_in, char* plaintext_out)
    {
        return base64_decode_block(code_in, length_in, plaintext_out, &_state);
    }

    void Base64decoder::decode(std::istream& istream_in, std::ostream& ostream_in)
    {
        base64_init_decodestate(&_state);

        const int N = _buffersize;
        char* code = new char[N];
        char* plaintext = new char[N];
        int codelength;
        int plainlength;

        do
        {
            istream_in.read((char*)code, N);
            codelength = istream_in.gcount();
            plainlength = decode(code, codelength, plaintext);
            ostream_in.write((const char*)plaintext, plainlength);
        }
        while (istream_in.good() && codelength > 0);

        base64_init_decodestate(&_state);

        delete [] code;
        delete [] plaintext;
    }

    char* Base64decoder::decode(const std::vector<std::string>& str_in, std::vector<unsigned int>& pos_out)
    {
        std::stringstream stream_out;
        {
            std::stringstream stream_in;

            pos_out.resize(str_in.size());

            for (unsigned int i = 0; i < str_in.size(); ++i)
            {
                stream_in.clear();
                stream_in<<str_in.at(i);
                stream_in.seekg(0, stream_in.beg);

                decode(stream_in, stream_out);
                pos_out.at(i) = stream_out.tellp();
            }
        }

        // Allocate memory for use with osg::Image
        const std::string str = stream_out.str();
        char* allocated_out = new char[str.size()];
        memcpy(allocated_out, str.c_str(), str.size());

        return allocated_out;
    }

} // namespace osgDB
