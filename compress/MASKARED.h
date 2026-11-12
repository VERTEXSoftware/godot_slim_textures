// ┌──────────────────────────────────┐
// │            __          _   _  _  │
// │ |\/|  /\  (_  |/  /\  |_) |_ | \ │
// │ |  | /--\ __) |\ /--\ | \ |_ |_/ │
// │ Sleptsov compression algorithm   │
// └──────────────────────────────────┘
// SPDX-License-Identifier: MIT
// MASKARED compression algorithm for C/C++
// Version: 1.4.0.0
// Copyright (C) 2026 Sleptsov Vladimir 
// https://github.com/VERTEXSoftware


#ifndef SLEP_MASKARED_H
#define SLEP_MASKARED_H

#define MASKARED_VER_MAJOR  1
#define MASKARED_VER_MINOR  4
#define MASKARED_VER_BUGFIX 0
#define MASKARED_VER_HOTFIX 0

#define MASKARED_VER ((MASKARED_VER_MAJOR << 24) | (MASKARED_VER_MINOR << 16) | (MASKARED_VER_BUGFIX << 8) | (MASKARED_VER_HOTFIX))

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <cstdint>

	typedef enum {
		SL_OK 					= 0,
		SL_ERROR_INVALID_PARAM 	= 1,
		SL_ERROR_DATA 			= 2

	} MASKARED_RESULT;

	extern uint32_t        MASKARED_VERSION		    ();

	extern MASKARED_RESULT MASKARED_SIZE_CALC		(uint8_t* buf, uint32_t size, uint32_t* sizec, uint8_t mask = 0x00u);

	extern MASKARED_RESULT MASKARED_ENCODE		    (uint8_t* buf, uint32_t size, uint8_t* bufc, uint32_t* sizec);
	extern MASKARED_RESULT MASKARED_DECODE		    (uint8_t* buf, uint32_t size, uint8_t* bufd, uint32_t  sized);


#ifdef __cplusplus
}
#endif

#ifdef SLEP_MASKARED_IMP

uint32_t MASKARED_VERSION(){ return MASKARED_VER; }

#ifdef SLEP_MASKARED_EXP

static constexpr uint8_t MASKARED_PCTABLE[0x100u] = {
    0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 0x05, 0x06, 0x06, 0x07, 0x06, 0x07, 0x07, 0x08,
};


MASKARED_RESULT MASKARED_ENCODE(uint8_t* buf, uint32_t size, uint8_t* bufc, uint32_t* sizec) {

    if (buf == NULL || bufc == NULL  || size == 0x00u) { return MASKARED_RESULT::SL_ERROR_INVALID_PARAM; }

	const uint8_t* 	end			= buf + size;
	uint8_t* 		pstr		= bufc + 0x02u;
	uint8_t 		move 		= 0x00u;
	uint8_t 		accum		= 0x00u;
	uint8_t 		mask 		= 0xFFu;
	uint32_t 		step		= 0x00u;
	uint16_t 		pos			= 0x00u;
	uint8_t			hist		[0x100u]{};

   for (uint8_t *p = buf; p < end; ++p) {
        *(hist + *p) = 0x01u;
    }
	
	for (uint16_t i = 0x00u; i < 0x100u; ++i) {
		if (*(hist + i) == 1) {
			*(hist + pos++) = uint8_t(i);
		}
	}

	for (uint8_t m = 0x00u; m < 0x80u; ++m)
    {
		uint8_t mask_tmp = 0x00u;

		for (uint16_t i = 0x00u; i < pos; ++i)
    	{
			mask_tmp |= (*buf + m) ^ (*(hist + i) + m);
		}

		if(*(MASKARED_PCTABLE+mask_tmp) < *(MASKARED_PCTABLE+mask)){
			mask = mask_tmp;
			move = m;
			if(mask == 0x00u){break;}
		}	
	}

	uint8_t first = *buf + move;

	for (uint8_t bit = 0x80u; bit > 0x00u; bit >>= 0x01u) {
		if (!(mask & bit)) {
			if (first & bit) {
				accum |= (0x80u >> step);
			}
			++step;
		}
	}

	uint32_t total 	= size * (0x08u - step);

	*sizec 			= (step + total + 0x17u) >> 0x03u;
	*bufc 			= move;
	*(bufc + 0x01u) = mask;
	*pstr 			= accum;

	for (uint8_t *c, *p = buf; p < end; ++p) {
		const uint8_t ps = uint8_t(*p+move);
		for (uint8_t bit = 0x80u; bit > 0x00u; bit >>= 0x01u) {
			if (mask & bit) {
				if (ps & bit) {
					c = pstr + (step >> 0x03u);
					*c |= (0x80u >> (step & 0x07u));
				}
				++step;
			}
		}
	}

	//Tails remover
    uint32_t pos_tail = *sizec;
    while (pos_tail > 0x01u && *(bufc + pos_tail - 0x01u) == 0x00u) {
        --pos_tail;
    }
    *sizec = pos_tail;

    return MASKARED_RESULT::SL_OK;
}


MASKARED_RESULT MASKARED_DECODE(uint8_t* buf, uint32_t size, uint8_t* bufd, uint32_t sized) {

    if (buf == NULL || bufd == NULL  || size == 0x00u || sized == 0x00u) { return MASKARED_RESULT::SL_ERROR_INVALID_PARAM; }

	uint8_t* 		pstr	= buf + 0x02u;
	const uint8_t 	mask	= *(buf + 0x01u);
	const uint8_t 	move	= *buf;
	const uint8_t 	accum 	= *pstr;
	const uint8_t* 	endd 	= bufd + sized;
	const uint8_t* 	endp 	= buf + size;
	uint8_t  		chr		= 0x00u;
	uint32_t 		step	= 0x00u;

	for (uint8_t bit = 0x80u; bit > 0x00u; bit >>= 0x01u) {
		if (!(mask & bit)) {
			if ((accum << step) & 0x80u) {
				chr |= bit;
			}
			++step;
		}
	}

	for (uint8_t* d = bufd, *p = pstr; d < endd; ++d) {
		*d = chr;
		for (uint8_t bit = 0x80u; bit > 0x00u && p < endp; bit >>= 0x01u) {
			if (mask & bit) {
				if (*p & (0x80u >> (step & 0x07u))) { *d |= bit; }
				p = pstr + (++step >> 0x03u);
			}
		}
		*d-=move;
	}

    return MASKARED_RESULT::SL_OK;
}

#else


MASKARED_RESULT MASKARED_ENCODE(uint8_t* buf, uint32_t size, uint8_t* bufc, uint32_t* sizec) {

    if (buf == NULL || bufc == NULL  || size == 0x00u) { return MASKARED_RESULT::SL_ERROR_INVALID_PARAM; }

	uint8_t* pstr		= bufc + 0x01u;
	const uint8_t first = *buf;
	const uint8_t* end	= buf + size;
	uint8_t mask		= 0x00u;
	uint8_t accum		= 0x00u;
	uint32_t step		= 0x00u;

	for (uint8_t* p = buf + 0x01u; p < end && mask != 0xFFu; ++p) {
		mask |= (first ^ *p);
	}

	for (uint8_t bit = 0x80u; bit > 0x00u; bit >>= 0x01u) {
		if (!(mask & bit)) {
			if (first & bit) {
				accum |= (0x80u >> step);
			}
			++step;
		}
	}

	uint32_t total = size * (0x08u - step);
	*sizec = (step + total + 0x0Fu) >> 0x03u;

	*bufc = mask;
	*pstr = accum;

	for (uint8_t *c, *p = buf; p < end; ++p) {
		for (uint8_t bit = 0x80u; bit > 0x00u; bit >>= 0x01u) {
			if (mask & bit) {
				if (*p & bit) {
					c = pstr + (step >> 0x03u);
					*c |= (0x80u >> (step & 0x07u));
				}
				++step;
			}
		}
	}

	//Tails remover
    uint32_t pos_tail = *sizec;
    while (pos_tail > 0x01u && *(bufc + pos_tail - 0x01u) == 0x00u) {
        --pos_tail;
    }
    *sizec = pos_tail;

    return MASKARED_RESULT::SL_OK;
}


MASKARED_RESULT MASKARED_DECODE(uint8_t* buf, uint32_t size, uint8_t* bufd, uint32_t sized) {

    if (buf == NULL || bufd == NULL  || size == 0x00u || sized == 0x00u) { return MASKARED_RESULT::SL_ERROR_INVALID_PARAM; }

	uint8_t* pstr		= buf + 0x01u;
	const uint8_t mask	= *buf;
	const uint8_t accum = *pstr;
	const uint8_t* endd = bufd + sized;
	const uint8_t* endp = buf + size;
	uint8_t  chr		= 0x00u;
	uint32_t step		= 0x00u;

	for (uint8_t bit = 0x80u; bit > 0x00u; bit >>= 0x01u) {
		if (!(mask & bit)) {
			if ((accum << step) & 0x80u) {
				chr |= bit;
			}
			++step;
		}
	}

	for (uint8_t* d = bufd, *p = pstr; d < endd; ++d) {
		*d = chr;
		for (uint8_t bit = 0x80u; bit > 0x00u && p < endp; bit >>= 0x01u) {
			if (mask & bit) {
				if (*p & (0x80u >> (step & 0x07u))) { *d |= bit; }
				p = pstr + (++step >> 0x03u);
			}
		}
	}

    return MASKARED_RESULT::SL_OK;
}

#endif // SLEP_MASKARED_EXP
#endif // SLEP_MASKARED_IMP 
#endif // SLEP_MASKARED_H