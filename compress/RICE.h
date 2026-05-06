
#ifndef RICE_H
#define RICE_H

#define RICE_VER_MAJOR  1
#define RICE_VER_MINOR  1
#define RICE_VER_BUGFIX 0
#define RICE_VER_HOTFIX 0

#define RICE_VER ((RICE_VER_MAJOR << 24) | (RICE_VER_MINOR << 16) | (RICE_VER_BUGFIX << 8) | (RICE_VER_HOTFIX))

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <cstdint>

	typedef enum {
		RICE_OK = 0,
		RICE_ERROR_INVALID_PARAM = 1,
		RICE_ERROR_DATA = 2

	} RICE_RESULT;

	extern uint32_t     RICE_VERSION		();

	extern RICE_RESULT  RICE_ENCODE		    (uint8_t* buf, uint32_t size, uint8_t* bufc, uint32_t* sizec);
	extern RICE_RESULT  RICE_DECODE		    (uint8_t* buf, uint32_t size, uint8_t* bufd, uint32_t sized);


#ifdef __cplusplus
}
#endif

#ifdef RICE_IMP

uint32_t RICE_VERSION(){ return RICE_VER; }


RICE_RESULT RICE_ENCODE(uint8_t* buf, uint32_t size, uint8_t* bufc, uint32_t* sizec)
{
    if (buf == NULL || bufc == NULL  || size <= 0) { return RICE_RESULT::RICE_ERROR_INVALID_PARAM; }

    double avg = 0.0;
    for (uint32_t i = 0; i < size; ++i){
        avg += buf[i];
    }
    avg /= double(size);

    uint8_t k = 0;

    if (avg >= 1.0)
    {
        if      (avg >= 128.0){k = 7;}
        else if (avg >= 64.0) {k = 6;}
        else if (avg >= 32.0) {k = 5;}
        else if (avg >= 16.0) {k = 4;}
        else if (avg >= 8.0)  {k = 3;}
        else if (avg >= 4.0)  {k = 2;}
        else if (avg >= 2.0)  {k = 1;}
    }

    bufc[0] = k;
    uint32_t bitPos = 8;

    for (uint32_t i = 0; i < size; ++i)
    {
        uint32_t v = buf[i];
        uint32_t q = v >> k;
        uint32_t r = v & ((1u << k) - 1);


        for (uint32_t j = 0; j < q; ++j)
        {
            uint32_t bytePos = bitPos >> 3;
            bufc[bytePos] |= (1u << (7 - (bitPos & 7)));
            bitPos++;
        }


        ++bitPos;


        for (int b = (int)k - 1; b >= 0; --b)
        {
            uint32_t bytePos = bitPos >> 3;
            uint32_t bit = (r >> b) & 1u;
            if (bit)
                bufc[bytePos] |= (1u << (7 - (bitPos & 7)));
            bitPos++;
        }
    }

    *sizec = (bitPos + 7) / 8;

	return RICE_RESULT::RICE_OK;
}


RICE_RESULT RICE_DECODE(uint8_t* buf, uint32_t size, uint8_t* bufd, uint32_t sized)
{
    if (buf == NULL || bufd == NULL || size == 0 || sized == 0) {return RICE_RESULT::RICE_ERROR_INVALID_PARAM; }

    uint32_t bitPos = 8;
    uint8_t k = buf[0];
    const uint32_t totalBits = size * 8u;

    bool endOfData = false;

    for (uint32_t i = 0; i < sized; ++i)
    {
        if (bitPos >= totalBits) {
            break;
        }

        uint32_t q = 0;


        while (true)
        {
            if (bitPos >= totalBits) {
                endOfData = true;
                break;
            }

            uint32_t bytePos = bitPos >> 3;
            uint32_t bit = (buf[bytePos] >> (7 - (bitPos & 7))) & 1u;
            ++bitPos;

            if (bit == 0) {break;}
            ++q;
        }

        if (endOfData) {break;}

        uint32_t r = 0;
        for (uint32_t b = 0; b < k; ++b)
        {
            if (bitPos >= totalBits) {
                endOfData = true;
                break;
            }

            uint32_t bytePos = bitPos >> 3;
            uint32_t bit = (buf[bytePos] >> (7 - (bitPos & 7))) & 1u;
            r = (r << 1) | bit;
            ++bitPos;
        }

        if (endOfData){break;}

        bufd[i] = uint8_t((q << k) | r);
    }

    return RICE_RESULT::RICE_OK;
}



#endif // RICE_IMP 
#endif // RICE_H