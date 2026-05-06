//   /$$$$$$  /$$                       /$$                                              
//  /$$__  $$| $$                      | $$                                              
// | $$  \__/| $$  /$$$$$$   /$$$$$$  /$$$$$$   /$$$$$$$  /$$$$$$  /$$    /$$            
// |  $$$$$$ | $$ /$$__  $$ /$$__  $$|_  $$_/  /$$_____/ /$$__  $$|  $$  /$$/            
//  \____  $$| $$| $$$$$$$$| $$  \ $$  | $$   |  $$$$$$ | $$  \ $$ \  $$/$$/             
//  /$$  \ $$| $$| $$_____/| $$  | $$  | $$ /$$\____  $$| $$  | $$  \  $$$/              
// |  $$$$$$/| $$|  $$$$$$$| $$$$$$$/  |  $$$$//$$$$$$$/|  $$$$$$/   \  $/               
//  \______/ |__/ \_______/| $$____/    \___/ |_______/  \______/     \_/                
//                         | $$                                                          
//                         | $$                                                          
//                         |__/                                                          
//  /$$$$$$$                                      /$$$$$$$              /$$              
// | $$__  $$                                    | $$__  $$            | $$              
// | $$  \ $$  /$$$$$$   /$$$$$$   /$$$$$$       | $$  \ $$  /$$$$$$  /$$$$$$    /$$$$$$ 
// | $$  | $$ /$$__  $$ /$$__  $$ /$$__  $$      | $$  | $$ |____  $$|_  $$_/   |____  $$
// | $$  | $$| $$  \__/| $$  \ $$| $$  \ $$      | $$  | $$  /$$$$$$$  | $$      /$$$$$$$
// | $$  | $$| $$      | $$  | $$| $$  | $$      | $$  | $$ /$$__  $$  | $$ /$$ /$$__  $$
// | $$$$$$$/| $$      |  $$$$$$/| $$$$$$$/      | $$$$$$$/|  $$$$$$$  |  $$$$/|  $$$$$$$
// |_______/ |__/       \______/ | $$____/       |_______/  \_______/   \___/   \_______/
//                               | $$                                                    
//                               | $$                                                    
//                               |__/                                                    
//
// SPDX-License-Identifier: MIT
// SLDD compression algorithm for C/C++
// Version: 2.1.0.0
// Copyright (C) 2026 Sleptsov Vladimir 
// https://github.com/VERTEXSoftware

#ifndef SLEP_SLDD_H
#define SLEP_SLDD_H

#define SLDD_VER_MAJOR  2
#define SLDD_VER_MINOR  1
#define SLDD_VER_BUGFIX 0
#define SLDD_VER_HOTFIX 0

#define SLDD_VER ((SLDD_VER_MAJOR << 24) | (SLDD_VER_MINOR << 16) | (SLDD_VER_BUGFIX << 8) | (SLDD_VER_HOTFIX))

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <cstdint>


	typedef enum {
		SLDD_OK = 0,
		SLDD_ERROR_INVALID_PARAM = 1,
		SLDD_ERROR_DATA = 2

	} SLDD_RESULT;

	extern uint32_t     SLDD_VERSION	();

	extern SLDD_RESULT  SLDD_ENCODE		(uint8_t* buffer, uint32_t size, uint8_t* buffercomp, uint32_t* sizecomp);
	extern SLDD_RESULT  SLDD_DECODE		(uint8_t* buffer, uint32_t size, uint8_t* bufferde, uint32_t sizede);



#ifdef __cplusplus
}
#endif


#ifdef SLEP_SLDD_IMP

uint32_t SLDD_VERSION(){ return SLDD_VER; }


SLDD_RESULT SLDD_ENCODE(uint8_t* buffer, uint32_t size, uint8_t* buffercomp, uint32_t* sizecomp) {

    if (buffer == NULL || buffercomp == NULL  || size <= 0) { return SLDD_RESULT::SLDD_ERROR_INVALID_PARAM; }


	uint8_t* pstr		= buffercomp + 0x01u;
	const uint8_t first = *buffer;
	const uint8_t* end	= buffer + size;

	int leftCount			= 0x07u;
	int rightCount			= 0x07u;
	const bool leftc		= (first >> 7)	& 0x1u;
	const bool rightc		= first			& 0x1u;

    for (uint8_t* p = buffer; p != end; ++p) {
        const uint8_t cur = *p;

        int left_c= 0;
        int right_c = 0;
        bool left_open = true;
        bool right_open = true;

		uint8_t bitLm = 0x80u;
		uint8_t bitRm = 0x01u;

		while (bitLm != 0u) {
			bool bitL = (cur & bitLm);
			bool bitR = (cur & bitRm);

			bitLm >>= 1;
			bitRm <<= 1;

			if (left_open) {if (bitL == leftc){++left_c;}else{left_open = false;}}
            if (right_open) {if (bitR == rightc){++right_c;}else{right_open = false;}}
            
		}
        if (left_c < leftCount)  {leftCount = left_c;}
        if (right_c < rightCount) {rightCount = right_c;}
        if (leftCount == 0 && rightCount == 0) {break;}
    }


    if (leftCount + rightCount > 8) {
        rightCount -= leftCount + rightCount - 8;
        if (rightCount < 0) {rightCount = 0;}
    }


	uint32_t total 	= size * (0x08u - leftCount - rightCount);
	*sizecomp 		= (total + 0x0Fu) >> 0x03u;

	*buffercomp = (uint8_t)((leftCount << 5) |(rightCount << 2) |(((uint8_t)leftc) << 1) | ((uint8_t)rightc));

	uint8_t mstart 	= (0x80u >> leftCount);
	uint8_t mend 	= (0x80u >> (8 - rightCount));
	uint32_t step 	= 0;
	
	for (uint8_t* p = buffer, *c = pstr; p < end; ++p) {
		for (uint8_t bit = mstart; bit > mend; bit >>= 0x01u) {
			if (*p & bit) {
				c = pstr + (step >> 0x03u);
				*c |= (0x80u >> (step & 0x07u));
			}
			++step;
		}
	}
	//Tails remover
	while (*sizecomp > 1 && buffercomp[*sizecomp - 1] == 0) {
		--*sizecomp;
	}

    return SLDD_RESULT::SLDD_OK;
}




SLDD_RESULT SLDD_DECODE(uint8_t* buffer, uint32_t size, uint8_t* bufferde, uint32_t sizede) {

    if (buffer == NULL || bufferde == NULL  || size <= 0 ) { return SLDD_RESULT::SLDD_ERROR_INVALID_PARAM; }

	uint8_t* pstr			= buffer + 0x01u;
	uint8_t DataByte		= *buffer;
	const uint8_t* endd 	= bufferde + sizede;
	const uint8_t* endp 	= buffer + size;

	uint32_t leftCount		= (DataByte >> 5)	& 0x7u;
	uint32_t rightCount		= (DataByte >> 2)	& 0x7u;
	const bool leftc		= (DataByte >> 1)	& 0x1u;
	const bool rightc		= DataByte			& 0x1u;

	uint8_t mstart 			= (0x80u >> leftCount);
	uint8_t mend 			= (0x80u >> (8 - rightCount));
	uint8_t  chr			= 0x00u;

	if (leftCount > 0 && leftc)  {chr |= ((uint8_t)((1u << leftCount) - 1u)) << (8 - leftCount);}
    if (rightCount > 0 && rightc) {chr |= ((uint8_t)((1u << rightCount) - 1u));}

	uint32_t step = 0;
//&& p < endp error!
	for (uint8_t* d = bufferde, *p = pstr; d < endd; ++d) {
		*d = chr;
		for (uint8_t bit = mstart; bit > mend && p < endp; bit >>= 0x01u) {
				if (*p & (0x80u >> (step & 0x07u))) { *d |= bit; }
				p = pstr + (++step >> 0x03u);
		}
	}

    return SLDD_RESULT::SLDD_OK;
}




#endif // SLEP_SLDD_IMP 
#endif // SLEP_SLDD_H