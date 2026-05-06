
#ifndef RLE_H
#define RLE_H

#define RLE_VER_MAJOR  1
#define RLE_VER_MINOR  1
#define RLE_VER_BUGFIX 0
#define RLE_VER_HOTFIX 0

#define RLE_VER ((RLE_VER_MAJOR << 24) | (RLE_VER_MINOR << 16) | (RLE_VER_BUGFIX << 8) | (RLE_VER_HOTFIX))

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <cstdint>

	typedef enum {
		RLE_OK = 0,
		RLE_ERROR_INVALID_PARAM = 1,
		RLE_ERROR_DATA = 2

	} RLE_RESULT;

	extern uint32_t   RLE_VERSION		();

	extern RLE_RESULT RLE_ENCODE		(uint8_t* buf, uint32_t size, uint8_t* bufc, uint32_t* sizec);
	extern RLE_RESULT RLE_DECODE		(uint8_t* data, uint32_t Length, uint8_t* outdata, uint32_t* counter);


#ifdef __cplusplus
}
#endif

#ifdef RLE_IMP

uint32_t RLE_VERSION(){ return RLE_VER; }


RLE_RESULT RLE_ENCODE(uint8_t* data, uint32_t Length, uint8_t* outdata, uint32_t* counter)
{
    if (data == NULL || outdata == NULL  || Length <= 0) { return RLE_RESULT::RLE_ERROR_INVALID_PARAM; }

	uint32_t idx = 0;
	uint32_t i = 0;

	while (i < Length) {
		uint32_t cnt = 1;
		while (i + cnt < Length && data[i + cnt] == data[i] && cnt < 127) { ++cnt; }

		if (cnt > 1) {
			outdata[idx++] = uint8_t(cnt);
			outdata[idx++] = data[i];
			i += cnt;
		}
		else {
			cnt = 0;
			while (i + cnt < Length && (i + cnt + 1 >= Length || data[i + cnt] != data[i + cnt + 1]) && cnt < 127) { ++cnt; }
			outdata[idx++] = uint8_t(~cnt + 1);
			for (uint32_t j = 0; j < cnt; ++j) { outdata[idx++] = data[i + j]; }
			i += cnt;
		}
	}
	*counter = idx;
	return RLE_RESULT::RLE_OK;
}


RLE_RESULT RLE_DECODE(uint8_t* data, uint32_t Length, uint8_t* outdata, uint32_t* outLength)
{
    if (data == NULL || outdata == NULL  || Length <= 0 ) { return RLE_RESULT::RLE_ERROR_INVALID_PARAM; }

	uint32_t idx = 0;
	uint32_t i = 0;

	while (i < Length) {
		int8_t cnt = int8_t(data[i++]);

		if (cnt > 0) {
			for (uint32_t j = 0; j < uint32_t(cnt); ++j) { outdata[idx++] = data[i]; }
			++i;
		}
		else {
			for (uint32_t j = 0; j < uint32_t(~cnt + 1); ++j) {
				outdata[idx++] = data[i++];
			}
		}
	}
	*outLength = idx;
	return RLE_RESULT::RLE_OK;
}


#endif // RLE_IMP 
#endif // RLE_H