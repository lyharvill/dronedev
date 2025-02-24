#pragma once

#include <math.h>
#include "Connection.h"
#include "messageout.h"

/**
* Utilities to pack and unpack variables from a byte stream.
*/
class PackUtils
{
public:

    /**
    * Unpacks a float32 
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned float32 unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, float &value) {
        union {
            float value;
            unsigned char bytes[4];
        } converter;

        converter.bytes[0] = buffer[index + 0];
        converter.bytes[1] = buffer[index + 1];
        converter.bytes[2] = buffer[index + 2];
        converter.bytes[3] = buffer[index + 3];

        value = converter.value;
        return 4;
    }

    /**
    * Unpacks a uint32_t
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned uint32_t unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, uint32_t& value) {
        union {
            uint32_t value;
            unsigned char bytes[4];
        } converter;

        converter.bytes[0] = buffer[index + 0];
        converter.bytes[1] = buffer[index + 1];
        converter.bytes[2] = buffer[index + 2];
        converter.bytes[3] = buffer[index + 3];

        value = converter.value;
        return 4;
    }

    /**
    * Unpacks a uint16_t
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned uint16_t unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, uint16_t& value) {
        union {
            uint16_t value;
            unsigned char bytes[2];
        } converter;

        converter.bytes[0] = buffer[index + 0];
        converter.bytes[1] = buffer[index + 1];

        value = converter.value;
        return 2;
    }

    /**
    * Unpacks an int16_t
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned int16_t unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, int16_t& value) {
        union {
            int16_t value;
            unsigned char bytes[2];
        } converter;

        converter.bytes[0] = buffer[index + 0];
        converter.bytes[1] = buffer[index + 1];

        value = converter.value;
        return 2;
    }

    /**
    * Unpacks an int32_t
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned int32_t unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, int32_t& value) {
        union {
            int32_t value;
            unsigned char bytes[4];
        } converter;

        converter.bytes[0] = buffer[index + 0];
        converter.bytes[1] = buffer[index + 1];
        converter.bytes[2] = buffer[index + 2];
        converter.bytes[3] = buffer[index + 3];

        value = converter.value;
        return 4;
    }

    /**
    * Unpacks a uint8_t
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned uint8_t unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, uint8_t& value) {
        union {
            uint8_t value;
            unsigned char bytes[1];
        } converter;

        converter.bytes[0] = buffer[index + 0];

        value = converter.value;
        return 1;
    }

    /**
    * Unpacks a bool
    * @param The buffer holding the bytes to unpack
    * @param The index to the start of the bytes to unpack
    * @param The returned bool unpacked value
    * @returns The number of bytes unpacked.
    */
    static uint8_t unpack(uint8_t* buffer, uint8_t index, bool& value) {
        value = buffer[index] != 0;
        return 1;
    }

    /**
    * Packs a float32 value into a byte buffer.
    * @param Pack the value into this buffer. 
    * @param The index to the start of the buffer for packing.
    * @param The float32 value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t pack(uint8_t* buffer, uint8_t index, float value) {
        union {
            float value;
            unsigned char bytes[4];
        } converter;
        converter.value = value;
        buffer[index + 0] = converter.bytes[0];
        buffer[index + 1] = converter.bytes[1];
        buffer[index + 2] = converter.bytes[2];
        buffer[index + 3] = converter.bytes[3];
        return 4;
    }

    /**
    * Packs a uint32_t value into a byte buffer.
    * @param Pack the value into this buffer.
    * @param The index to the start of the buffer for packing.
    * @param The uint32_t value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t pack(uint8_t* buffer, uint8_t index, uint32_t value) {
        union {
            uint32_t value;
            unsigned char bytes[4];
        } converter;
        converter.value = value;
        buffer[index + 0] = converter.bytes[0];
        buffer[index + 1] = converter.bytes[1];
        buffer[index + 2] = converter.bytes[2];
        buffer[index + 3] = converter.bytes[3];
        return 4;
    }

    /**
    * Packs a uint16_t value into a byte buffer.
    * @param Pack the value into this buffer.
    * @param The index to the start of the buffer for packing.
    * @param The uint16_t value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t pack(uint8_t* buffer, uint8_t index, uint16_t value) {
        union {
            uint16_t value;
            unsigned char bytes[2];
        } converter;
        converter.value = value;
        buffer[index + 0] = converter.bytes[0];
        buffer[index + 1] = converter.bytes[1];
        return 2;
    }

    /**
    * Packs an int16_t value into a byte buffer.
    * @param Pack the value into this buffer.
    * @param The index to the start of the buffer for packing.
    * @param The int16_t value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t pack(uint8_t* buffer, uint8_t index, int16_t value) {
        union {
            int16_t value;
            unsigned char bytes[2];
        } converter;
        converter.value = value;
        buffer[index + 0] = converter.bytes[0];
        buffer[index + 1] = converter.bytes[1];
        return 2;
    }

    /**
    * Packs an uint8_t value into a byte buffer.
    * @param Pack the value into this buffer.
    * @param The index to the start of the buffer for packing.
    * @param The int16_t value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t pack(uint8_t* buffer, uint8_t index, uint8_t value) {
        buffer[index] = value;
        return 1;
    }

    /**
    * Packs a bool value into a byte buffer.
    * @param Pack the value into this buffer.
    * @param The index to the start of the buffer for packing.
    * @param The bool value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t pack(uint8_t* buffer, uint8_t index, bool value) {
        uint8_t byteVal = (value) ? 1 : 0;
        buffer[index] = byteVal;
        return 1;
    }



    /**
    * Conversion to and from float16
    * https://stackoverflow.com/questions/6162651/half-precision-floating-point-in-java/6162687#6162687
    */

    /**
    * Converts a float16 to a float32
    * @param A uint32_t holding the float16.
    * @returns The converted float32.
    */
    static float unPackFloat16(uint32_t hbits)
    {
        int mant = hbits & 0x03ff;            // 10 bits mantissa
        int exp = hbits & 0x7c00;            // 5 bits exponent
        if (exp == 0x7c00)                   // NaN/Inf
            exp = 0x3fc00;                    // -> NaN/Inf
        else if (exp != 0)                   // normalized value
        {
            exp += 0x1c000;                   // exp - 15 + 127
            if (mant == 0 && exp > 0x1c400)  // smooth transition
            {
                uint32_t fBits = ((hbits & 0x8000) << 16) | exp << 13 | 0x3ff;
                float n = *(float*)&fBits;
                return(n);
            }
        }
        else if (mant != 0)                  // && exp==0 -> subnormal
        {
            exp = 0x1c400;                    // make it normal
            do {
                mant <<= 1;                   // mantissa * 2
                exp -= 0x400;                 // decrease exp by 1
            } while ((mant & 0x400) == 0); // while not normal
            mant &= 0x3ff;                    // discard subnormal bit
        }                                     // else +/-0 -> +/-0
        {
            uint32_t fBits = ((hbits & 0x8000) << 16          // sign  << ( 31 - 15 )
                | (exp | mant) << 13);
            float n = *(float*)&fBits;
            return(n);
        }
    }
    static float unPackFloat16(uint8_t *buffer)
    {
        uint16_t _hbits = 0;
        PackUtils::unpack(buffer, 0, _hbits);
        uint32_t hbits = _hbits;
    
        return(unPackFloat16(hbits));
    }


    /**
    * Converts a float32 to a float16
    * @param A float32
    * @returns The converted float16.
    */
    static int packFloat16(float fval)
    {

        uint32_t fbits = *((uint32_t*)&fval);
        int sign = (fbits >> 16) & 0x8000;              // sign only
        int val = (fbits & 0x7fffffff) + 0x1000;        // rounded value

        if (val >= 0x47800000)                          // might be or become NaN/Inf
        {                                               // avoid Inf due to rounding
            if ((fbits & 0x7fffffff) >= 0x47800000)
            {                                           // is or must become NaN/Inf
                if (val < 0x7f800000)                   // was value but too large
                    return sign | 0x7c00;               // make it +/-Inf
                return sign | 0x7c00 |                  // remains +/-Inf or NaN
                    ((fbits & 0x007fffff) >> 13);       // keep NaN (and Inf) bits
            }
            return sign | 0x7bff;                       // unrounded not quite Inf
        }
        if (val >= 0x38800000)                          // remains normalized value
            return sign | ((val - 0x38000000) >> 13);     // exp - 127 + 15
        if (val < 0x33000000)                           // too small for subnormal
            return sign;                                // becomes +/-0
        val = (fbits & 0x7fffffff) >> 23;               // val is exp for subnormal calc

        int n = (fbits & 0x7fffff | 0x800000);          // add subnormal bit
        n = n + (0x800000 >> (val - 102));              // round depending on cut off
        n = n >> (126 - val);                           // div by 2^(1-(exp-127+15)) and >> 13 | exp=0
        n = n | sign;                                   // set the sign;
        return(n);
    }

    /**
    * Packs a float32 as a float16 value into a byte buffer.
    * @param Pack the value into this buffer.
    * @param The index to the start of the buffer for packing.
    * @param The float32 value to pack.
    * @returns The number of bytes packed.
    */
    static uint8_t packFloat16(uint8_t* buffer, uint8_t index, float value)
    {
        union {
            int32_t packedInt; 
            uint8_t buff[4];
        };
        packedInt = packFloat16(value);
        buffer[index] = buff[0];
        buffer[index + 1] = buff[1];
        return(2);
    }

    /**
    * Test for Float16 conversion
    */
    static void testFloat16()
    {
        float n = -180.00; 
        while (n < 180.00)
        {
            n += 0.01f;
            int32_t intBuff; 
            intBuff = packFloat16(n);
            uint8_t* buffer = (uint8_t*)&intBuff;
            float out = unPackFloat16(buffer);

            messageOut << n; 
            messageOut << ", ";
            messageOut << out;
            messageOut << "\n\r";
        }
    }

    // compress a unit quaternion into 32 bits.
    // assumes input quaternion is normalized. will fail if not.
    static inline uint32_t quatcompress(float const q[4])
    {
        // we send the values of the quaternion's smallest 3 elements.
        unsigned i_largest = 0;
        for (unsigned i = 1; i < 4; ++i) {
            if (fabsf(q[i]) > fabsf(q[i_largest])) {
                i_largest = i;
            }
        }

        // since -q represents the same rotation as q,
        // transform the quaternion so the largest element is positive.
        // this avoids having to send its sign bit.
        unsigned negate = q[i_largest] < 0;

        // 1/sqrt(2) is the largest possible value
        // of the second-largest element in a unit quaternion.
        float const SMALL_MAX = (float)(1.0 / sqrt(2));

        // do compression using sign bit and 9-bit precision per element.
        uint32_t comp = i_largest;
        for (unsigned i = 0; i < 4; ++i) {
            if (i != i_largest) {
                unsigned negbit = (q[i] < 0) ^ negate;
                unsigned mag = (unsigned)(((1 << 9) - 1) * (fabsf(q[i]) / SMALL_MAX) + 0.5f);
                comp = (comp << 10) | (negbit << 9) | mag;
            }
        }

        return comp;
    }


    // This is the matching function to decompress
    // decompress a quaternion from 32 bit compressed representation.
    static void quatdecompress(uint32_t comp, float q[4])
    {
        float const SMALL_MAX = (float)(1.0 / sqrt(2));
        unsigned const mask = (1 << 9) - 1;

        int const i_largest = comp >> 30;
        float sum_squares = 0;
        for (int i = 3; i >= 0; --i) {
            if (i != i_largest) {
                unsigned mag = comp & mask;
                unsigned negbit = (comp >> 9) & 0x1;
                comp = comp >> 10;
                q[i] = SMALL_MAX * ((float)mag) / mask;
                if (negbit == 1) {
                    q[i] = -q[i];
                }
                sum_squares += q[i] * q[i];
            }
        }
        q[i_largest] = sqrtf(1.0f - sum_squares);
    }
};

