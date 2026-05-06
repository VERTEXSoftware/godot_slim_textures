// SPDX-License-Identifier: MIT
// SLIM (Sleptsov Image format) for C/C++
// Version: 1.4.0.0
// Copyright (C) 2026 Sleptsov Vladimir 
// https://github.com/VERTEXSoftware

#ifndef SLIM_H
#define SLIM_H

#define SLIM_MAGIC 0x4D494C5373696854
#define SLIM_VERSION_MAJOR 1
#define SLIM_VERSION_MINOR 4
#define SLIM_VERSION_BUGFIX 0
#define SLIM_VERSION_HOTFIX 0

#define SLIM_VERSION ((SLIM_VERSION_MAJOR << 24) | (SLIM_VERSION_MINOR << 16) | (SLIM_VERSION_BUGFIX << 8) | (SLIM_VERSION_HOTFIX))

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>


#define SLIM_STREAM_IMP
#define SLEP_SLDD_IMP
#define SLEP_MASKARED_IMP
#define RLE_IMP
#define RICE_IMP

//Custom Stream
#include "./SLIMSTREAM.h"

//Custom Compression
#include "./compress/SLDD.h"
#include "./compress/MASKARED.h"
#include "./compress/RLE.h"
#include "./compress/RICE.h"

#ifndef SLIM_MALLOC
#include <stdlib.h>
#define SLIM_MALLOC(sz) malloc(sz)
#define SLIM_FREE(p) 	free(p)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
		ERROR_OK			= 0x0,
		ERROR_FILE			= 0x1,
		ERROR_BLOCK			= 0x2,
		ERROR_DATA			= 0x3,
		ERROR_END			= 0x4,
		ERROR_NOTSUP		= 0x5,
		ERROR_NONE			= 0x6,
		ERROR_MEM			= 0x7,
		ERROR_ARG			= 0x8,
		ERROR_COLORSPACE	= 0x9
} SLIMERROR;

typedef enum {
		CODE_NONE			= 0x0,
		CODE_GRAY			= 0x1,
		CODE_RGB			= 0x2,
		CODE_BGR			= 0x3,
		CODE_RGBA			= 0x4,
		CODE_BGRA			= 0x5,
} SLIMCODE;


typedef struct{
	uint32_t version;

    uint16_t width;
    uint16_t height;

    uint16_t layers;

    uint8_t  code;
} SLIMHeaderDesc;

typedef struct {
	uint16_t id;

    uint16_t width;
    uint16_t height;

    uint16_t x;
    uint16_t y;
    uint16_t z;

    uint8_t  code;
	uint8_t  forced_code;
    uint8_t  quality;

	uint8_t  name_size;	
	uint16_t ext_size;	

	void* 	 img;
	char*	 name;
	char* 	 ext;
} SLIMLayerDesc;

typedef struct
{
	uint16_t 				id;

	uint16_t 				x;
    uint16_t 				y;
    uint16_t 				z;

	uint16_t				width;
	uint16_t				height;
	uint8_t					code;

	uint32_t 				block_256_all;
	uint32_t 				block_256_exist;
	uint32_t 				block_256_empty;

	uint32_t				block_color_table_max;
	uint32_t				block_color_table_min;
	uint32_t				block_color_table_avg;

	uint32_t				block_q_max;
	uint32_t				block_q_min;
	uint32_t				block_q_avg;

	uint32_t				all_c;
	uint32_t				reuse_c;
	uint32_t				origin_c;	
	uint32_t				rle_c;
	uint32_t				rice_c;
	uint32_t				sldd_c;
	uint32_t				maskared_c;

	uint8_t  				name_size;	
	uint16_t 				ext_size;	

	char*	 				name;
	char* 	 				ext;

} SLIMLayerInfoDesc;


SLIMERROR SLIM_Read_Header		(SLIM_STREAM* file, SLIMHeaderDesc* desc);
SLIMERROR SLIM_Read_Layer		(SLIM_STREAM* file, SLIMLayerDesc* desc);


SLIMERROR SLIM_Write_Header		(SLIM_STREAM* file, const SLIMHeaderDesc* desc);
SLIMERROR SLIM_Write_Layer		(SLIM_STREAM* file, const SLIMLayerDesc* desc);


SLIMERROR SLIM_Read_Layer_Map	(SLIM_STREAM* file, SLIMLayerDesc* desc);
SLIMERROR SLIM_Read_Layer_Info	(SLIM_STREAM* file, SLIMLayerInfoDesc* desc);


void* SLIM_Malloc				(const uint32_t size);

SLIMERROR SLIM_Free				(void* data);
SLIMERROR SLIM_Free_Layer		(SLIMLayerDesc* desc);
SLIMERROR SLIM_Free_Layer_Info	(SLIMLayerInfoDesc* desc);


#ifdef __cplusplus
}
#endif

#ifdef SLEP_SLIM_IMP

void* SLIM_Malloc(uint32_t size){
	if(size==0){return NULL;}
	return SLIM_MALLOC(size);
}

SLIMERROR SLIM_Free(void* data){
	if(data==NULL){return SLIMERROR::ERROR_NONE;}
	SLIM_FREE(data);
	return SLIMERROR::ERROR_OK;
}

SLIMERROR SLIM_Free_Layer(SLIMLayerDesc* desc){
	if(desc==NULL){return SLIMERROR::ERROR_NONE;}

	if(desc->img!=NULL){SLIM_FREE(desc->img);}
	if(desc->name!=NULL){SLIM_FREE(desc->name);}
	if(desc->ext!=NULL){SLIM_FREE(desc->ext);}

	return SLIMERROR::ERROR_OK;
}

SLIMERROR SLIM_Free_Layer_Info(SLIMLayerInfoDesc* desc){

	if(desc==NULL){return SLIMERROR::ERROR_NONE;}
	if(desc->name!=NULL){SLIM_FREE(desc->name);}
	if(desc->ext!=NULL){SLIM_FREE(desc->ext);}

	return SLIMERROR::ERROR_OK;
}



bool IS_ORIG_LINE(uint8_t* a, uint8_t* b, uint32_t count) {

	uint8_t* _a = a;
	uint8_t* _b = b;
	uint32_t num = count;

	while (num--) {
		if (*_a++ != *_b++) {
			return true;
		}
	}

	return false;
}

uint8_t SPACE_CHANNEL_COUNT(uint8_t code){
	switch (code)
	{
	case SLIMCODE::CODE_GRAY: 
		return 1; 
	case SLIMCODE::CODE_RGB:
	case SLIMCODE::CODE_BGR:  
		return 3;
	case SLIMCODE::CODE_RGBA:
	case SLIMCODE::CODE_BGRA: 
		return 4;
	default:
		return 0;
	}
	return 0;
}

uint8_t COUNT_SPACE_CHANNEL(uint8_t code){
	switch (code)
	{
	case 1: 
		return SLIMCODE::CODE_GRAY;
	case 3:
		return SLIMCODE::CODE_RGB; 
	case 4: 
		return SLIMCODE::CODE_RGBA;
	default:
		return 0;
	}
	return 0;
}


void GEN_CLR_MAP(uint8_t* R, uint8_t* G, uint8_t* B, uint8_t* A, uint32_t* size, uint8_t* idx, uint32_t pidx, uint8_t cR, uint8_t cG, uint8_t cB, uint8_t cA) {

	uint32_t pos = 0x0u;
	uint32_t fnd = ((uint32_t)cR << 24u) | ((uint32_t)cG << 16u) | ((uint32_t)cB << 8u) | (uint32_t)cA;

    while (pos < *size) {
        uint32_t cur = ((uint32_t)R[pos] << 24u) |((uint32_t)G[pos] << 16u) | ((uint32_t)B[pos] << 8u) | (uint32_t)A[pos];
        
        if (cur == fnd) {
            idx[pidx] = pos;
            return;
        }
        if (cur > fnd) {
            break;
        }
        ++pos;
    }

	for (uint32_t i = 0; i < pidx; ++i) {
		if (idx[i] >= pos) { ++idx[i]; }
	}

	for (uint32_t i = *size; i > pos; --i) {
		R[i] = R[i - 1];
		G[i] = G[i - 1];
		B[i] = B[i - 1];
		A[i] = A[i - 1];
	}

	R[pos] 		= cR;
	G[pos] 		= cG;
	B[pos] 		= cB;
	A[pos] 		= cA;
	idx[pidx] 	= pos;
	++*size;

}




uint16_t ENCODE_REVOLVER(bool orig, uint8_t* src, uint8_t* dest, uint32_t size, uint32_t* r_size) {

	//--------------------------------------------------------------//
	//Encode by the revolver method
	//--------------------------------------------------------------//

	if (size <= 0) 		{return 0;}
	if (orig==false)  	{return 0;}

	uint8_t t_rle      [1024u]{};
	uint8_t t_rice     [1024u]{};
    uint8_t t_sldd     [1024u]{};
    uint8_t t_maskared [1024u]{};

    uint32_t r_size_pack    [5u]{size,size,size,size,size};
    uint8_t* pack           [5u]{src, t_rle,t_rice, t_sldd, t_maskared };

	uint16_t pos_mode = 0;

    RLE_ENCODE(src, 		size, t_rle, 		&r_size_pack[1u]);
	RICE_ENCODE(src, 		size, t_rice, 		&r_size_pack[2u]);
    SLDD_ENCODE(src, 		size, t_sldd, 		&r_size_pack[3u]);
    MASKARED_ENCODE(src, 	size, t_maskared, 	&r_size_pack[4u]);

    for(uint16_t i = 1; i < 5; ++i){
        if(r_size_pack[pos_mode]>r_size_pack[i]){
            pos_mode = i;
        }
    }

	*r_size = r_size_pack[pos_mode];

	uint8_t* d = dest;
	uint8_t* s = pack[pos_mode];
	uint8_t* e = s + *r_size;
	while (s < e) {*d++ = *s++;}

	return pos_mode+1;
}




void  DECODE_REVOLVER(uint16_t mode, uint8_t* src, uint8_t* dest, uint32_t size) {

	//--------------------------------------------------------------//
	//Decode by the revolver method
	//--------------------------------------------------------------//

	if (size<=0) 	{return;}
	if (mode==0)  	{return;}

	uint32_t r_size = 256;

	switch (mode)
	{
		case 1:
		{
			uint8_t* d = dest;
			uint8_t* s = src;
			uint8_t* e = s + size;
			while (s < e) {*d++ = *s++;}
			break;
		}	
		case 2:
		{
			RLE_DECODE(src, size, dest, &r_size);
			break;
		}
		case 3:
		{
			RICE_DECODE(src, size, dest, r_size);
			break;
		}
		case 4:
		{
			SLDD_DECODE(src, size, dest, r_size);
			break;
		}		
		case 5:
		{
			MASKARED_DECODE(src, size, dest, r_size);
			break;
		}
		default:
		{
			return;
		}		
	}
}


uint32_t BLOCK_ANALYZER(const uint8_t level,const uint8_t* img, const uint32_t m_WIDTH, const uint32_t m_HEIGHT,const  uint32_t blocksX,const uint32_t blocksY, const uint32_t channels) {

	//--------------------------------------------------------------//
	//Counting unique colors
	//--------------------------------------------------------------//

	uint32_t colors[256u]{};
	uint32_t colorCount = 0;

    for (uint32_t y = 0; y < 16; ++y)
    {
        for (uint32_t x = 0; x < 16; ++x)
        {
			uint32_t row    = blocksY + y;
            uint32_t column = blocksX + x;

            if (column >= m_WIDTH || row >= m_HEIGHT){continue;}

            uint32_t idx = channels * (row * m_WIDTH + column);

            uint8_t r = channels>0	? img[idx]		:0;
            uint8_t g = channels>1	? img[idx+1]	:0;
            uint8_t b = channels>2	? img[idx+2]	:0;
			uint8_t a = channels>3	? img[idx+3]	:0;

            uint32_t color = (r << 24) | (g << 16) | (b << 8) | a;
			bool found = false;

			for (uint32_t i = 0; i < colorCount; ++i)
			{
				if (colors[i] == color){found = true;break;}
			}

			if (!found){colors[colorCount++] = color;}
        }
    }

   	uint32_t levelq = colorCount * 0.0274509803;  // (7 / 255)
	
	uint32_t count = 0;
	double sumDiff = 0;
    const double realLevelq = (levelq == 0 ? 1.0 : (1.0 / (((double)levelq) * 2.0)));

	//--------------------------------------------------------------//
	//PSNR Analysis
	//--------------------------------------------------------------//

    for (uint32_t y = 0; y < 16; ++y)
    {
        for (uint32_t x = 0; x < 16; ++x)
        {
            uint32_t column = blocksX + x;
			uint32_t row    = blocksY + y;

            if (column >= m_WIDTH || row >= m_HEIGHT){continue;}

            uint32_t idx = channels * (row * m_WIDTH + column);

			if(channels>0){ 
				double c = (double)img[idx]; 
                double d = c - (c * realLevelq);
                sumDiff += d * d;
			} 
			if(channels>1){ 
				double c = (double)img[idx+1]; 
                double d = c - (c * realLevelq);
                sumDiff += d * d;
			} 
			if(channels>2){ 
				double c = (double)img[idx+2]; 
                double d = c - (c * realLevelq);
                sumDiff += d * d;
			} 
			if(channels>3){ 
				double c = (double)img[idx+3]; 
                double d = c - (c * realLevelq);
                sumDiff += d * d;
			}
            count += channels;
        }
    }

	double psnr = 1.0 - (sumDiff / count / 65025.0);

	//--------------------------------------------------------------//
	//Adjusting the evaluation level and quantization
	//--------------------------------------------------------------//

	if (psnr < 0.0){psnr = 0.0;}
    if (psnr > 1.0){psnr = 1.0;}
    
  	const double factor = (255.0 - (double)level) * 0.0156862745; //(4.0 / 255.0)

	uint32_t idxt = levelq * psnr * factor;

	if (idxt > 7){ idxt = 7;}

	return idxt;
}





SLIMERROR SLIM_Write_Header(SLIM_STREAM* file, const SLIMHeaderDesc* desc){

	if (file == NULL)								{return SLIMERROR::ERROR_ARG;}
	if (desc == NULL)								{return SLIMERROR::ERROR_ARG;}
	if (desc->height == 0 || desc->width == 0)		{return SLIMERROR::ERROR_ARG;}

	if (!SLIM_STREAM_ISOPEN(file))					{return SLIMERROR::ERROR_FILE;}

	const uint8_t m_Channels = SPACE_CHANNEL_COUNT(desc->code);

	if(m_Channels == 0 || m_Channels > 4)			{return SLIMERROR::ERROR_NOTSUP;}

	#pragma pack(push, 1)
	struct _SLIM_HEADER
	{
		uint64_t _magic;
		uint32_t _version;
		uint16_t _width;
		uint16_t _height;
		uint16_t _layers;
		uint8_t  _channels;
	};
	#pragma pack(pop)

	_SLIM_HEADER _slim_h{};

	_slim_h._magic 		= SLIM_MAGIC;
	_slim_h._version 	= SLIM_VERSION;
	_slim_h._width 		= desc->width;
	_slim_h._height 	= desc->height;
	_slim_h._layers 	= desc->layers;
	_slim_h._channels	= m_Channels;

	if (!SLIM_STREAM_WRITE(file, &_slim_h, sizeof(_SLIM_HEADER), 1)) { return SLIMERROR::ERROR_BLOCK; }
	
	return SLIMERROR::ERROR_OK;
}


SLIMERROR SLIM_Read_Header(SLIM_STREAM* file, SLIMHeaderDesc* desc){

	if (file == NULL)				{return SLIMERROR::ERROR_ARG;}
	if (desc == NULL)				{return SLIMERROR::ERROR_ARG;}

	if (!SLIM_STREAM_ISOPEN(file))	{return SLIMERROR::ERROR_FILE;}

	#pragma pack(push, 1)
	struct _SLIM_HEADER
	{
		uint64_t _magic;
		uint32_t _version;
		uint16_t _width;
		uint16_t _height;
		uint16_t _layers;
		uint8_t  _channels;
	};
	#pragma pack(pop)

	_SLIM_HEADER _slim_h{};

	if (!SLIM_STREAM_READ(file, &_slim_h, sizeof(_SLIM_HEADER), 1)) { return SLIMERROR::ERROR_END; }

	if(_slim_h._magic != SLIM_MAGIC)					{return SLIMERROR::ERROR_NOTSUP;}
	if(_slim_h._version != SLIM_VERSION)				{return SLIMERROR::ERROR_NOTSUP;}
	if(_slim_h._height == 0 || _slim_h._width == 0)		{return SLIMERROR::ERROR_BLOCK;}
	if(_slim_h._channels == 0 || _slim_h._channels > 4)	{return SLIMERROR::ERROR_NOTSUP;}

	const uint8_t m_Code = COUNT_SPACE_CHANNEL(_slim_h._channels);

	if(m_Code == 0)		{return SLIMERROR::ERROR_BLOCK;}

	desc->version 		= _slim_h._version;
	desc->width 		= _slim_h._width;
	desc->height 		= _slim_h._height;
	desc->layers 		= _slim_h._layers;
	desc->code 			= m_Code;
	
	return SLIMERROR::ERROR_OK;
}


SLIMERROR SLIM_Write_Layer(SLIM_STREAM* file, const SLIMLayerDesc* desc){

	if (file == NULL)							{return SLIMERROR::ERROR_ARG;}
	if (desc == NULL)							{return SLIMERROR::ERROR_ARG;}
	if (desc->img == NULL)						{return SLIMERROR::ERROR_ARG;}
	if (desc->width == 0 || desc->height == 0) 	{return SLIMERROR::ERROR_ARG;}

	if (!SLIM_STREAM_ISOPEN(file))				{return SLIMERROR::ERROR_FILE;}

	const uint8_t m_Channels 	= SPACE_CHANNEL_COUNT(desc->code);

	if(m_Channels == 0 || m_Channels > 4)		{return SLIMERROR::ERROR_NOTSUP;}

	#pragma pack(push, 1)
	struct _SLIM_LAYER_HEADER
	{
		uint16_t _id;
		uint16_t _width;
		uint16_t _height;
		uint16_t _x;
		uint16_t _y;
		uint16_t _z;
		uint8_t  _channel;
		uint8_t  _name_size;	
		uint16_t _ext_size;	
	};
	#pragma pack(pop)

	_SLIM_LAYER_HEADER _slim_lh{};

	_slim_lh._id 			= desc->id;
	_slim_lh._width 		= desc->width;
	_slim_lh._height 		= desc->height;
	_slim_lh._x 			= desc->x;
	_slim_lh._y 			= desc->y;
	_slim_lh._z 			= desc->z;
	_slim_lh._channel 		= m_Channels;
	_slim_lh._name_size 	= (desc->name_size>0 && desc->name!=NULL) ? desc->name_size : 0;
	_slim_lh._ext_size 		= (desc->ext_size>0 && desc->ext!=NULL) ? desc->ext_size : 0;

	if(!SLIM_STREAM_WRITE(file, &_slim_lh, sizeof(_SLIM_LAYER_HEADER), 1)) 				{ return SLIMERROR::ERROR_BLOCK; }

	if(_slim_lh._name_size  > 0 && desc->name != NULL){
		if (!SLIM_STREAM_WRITE(file, desc->name, sizeof(char), _slim_lh._name_size)) 	{ return SLIMERROR::ERROR_BLOCK; }
	}
	if(_slim_lh._ext_size > 0 && desc->ext != NULL){
		if (!SLIM_STREAM_WRITE(file, desc->ext, sizeof(char), _slim_lh._ext_size)) 		{ return SLIMERROR::ERROR_BLOCK; }
	}

	const uint8_t  m_QUALITY = desc->quality;
	const uint8_t* m_IMG 	 = (uint8_t*)desc->img;

	uint8_t m_data		[1280u]{}; 	//Old		block memory
	uint8_t l_data		[1280u]{}; 	//Curret	block memory
	uint8_t m_write		[1280u]{}; 	//Curret	block packed
	uint8_t m_size		[5u]{};		//Size 		blocks packed

	//Pointers old block memory
	uint8_t* m_ch0 = m_data;
	uint8_t* m_ch1 = m_data + 256u;
	uint8_t* m_ch2 = m_data + 512u;
	uint8_t* m_ch3 = m_data + 768u;
	uint8_t* m_idx = m_data + 1024u;

	//Pointers curret block memory
	uint8_t* l_ch0 = l_data;
	uint8_t* l_ch1 = l_data + 256u;
	uint8_t* l_ch2 = l_data + 512u;
	uint8_t* l_ch3 = l_data + 768u;
	uint8_t* l_idx = l_data + 1024u;

	for (uint32_t blcY = 0; blcY < _slim_lh._height; blcY += 16)
	{
		for (uint32_t blcX = 0; blcX < _slim_lh._width; blcX += 16)
		{
			uint32_t Cout 			= 0x0u;
			uint32_t CColor 		= 0x0u;
			const uint32_t qnt_idx 	= BLOCK_ANALYZER(m_QUALITY, m_IMG, _slim_lh._width, _slim_lh._height, blcX, blcY, m_Channels);
			const uint8_t  qnt		= (uint8_t)qnt_idx << 1;

			for (uint32_t y = 0; y < 16; ++y)
			{
				for (uint32_t x = 0; x < 16; ++x)
				{
					const uint32_t column 	= blcX + x;
					const uint32_t row 		= blcY + y;

					if (column >= _slim_lh._width || row >= _slim_lh._height) {continue;}

					const uint32_t index 	= m_Channels * (row * _slim_lh._width + column);

					uint8_t Rc = 0;
					uint8_t Gc = 0;
					uint8_t Bc = 0;
					uint8_t Ac = 0;

					switch (desc->code)
					{
					case SLIMCODE::CODE_GRAY:
					{
						Rc = m_IMG[index];
						break;
					}
					case SLIMCODE::CODE_RGB:
					{
						Rc = m_IMG[index];
						Gc = m_IMG[index + 1];
						Bc = m_IMG[index + 2];
						break;
					}
					case SLIMCODE::CODE_BGR:
					{
						Bc = m_IMG[index];
						Gc = m_IMG[index + 1];
						Rc = m_IMG[index + 2];	
						break;
					}
					case SLIMCODE::CODE_RGBA:
					{
						Ac = m_IMG[index + 3];
						if(Ac>0){
							Rc = m_IMG[index];
							Gc = m_IMG[index + 1];
							Bc = m_IMG[index + 2];
						}
						break;
					}
					case SLIMCODE::CODE_BGRA:
					{
						Ac = m_IMG[index + 3];
						if(Ac>0){
							Bc = m_IMG[index];
							Gc = m_IMG[index + 1];
							Rc = m_IMG[index + 2];
						}
						break;
					}
					default:
						return SLIMERROR::ERROR_NOTSUP;
					}

					if(qnt>0){
						Rc /= qnt;
						Gc /= qnt;
						Bc /= qnt;
						Ac /= qnt;
					}

					GEN_CLR_MAP(l_ch0, l_ch1, l_ch2, l_ch3, &CColor, l_idx, Cout, Rc, Gc, Bc, Ac);
					++Cout;
				}
			}

			const bool ch0_org	= IS_ORIG_LINE(m_ch0, l_ch0, CColor);
			const bool ch1_org	= IS_ORIG_LINE(m_ch1, l_ch1, CColor);
			const bool ch2_org	= IS_ORIG_LINE(m_ch2, l_ch2, CColor);
			const bool ch3_org	= IS_ORIG_LINE(m_ch3, l_ch3, CColor);
			const bool idx_org	= IS_ORIG_LINE(m_idx, l_idx, Cout);

			if (ch0_org || ch1_org || ch2_org|| ch3_org) {

				for (uint32_t i = 0; i < CColor; ++i) {
					if (ch0_org) { m_ch0[i] = l_ch0[i]; }
					if (ch1_org) { m_ch1[i] = l_ch1[i]; }
					if (ch2_org) { m_ch2[i] = l_ch2[i]; }
					if (ch3_org) { m_ch3[i] = l_ch3[i]; }
				}

				for (uint32_t i = CColor; i < 256; ++i) {
					if (ch0_org) { m_ch0[i] = 0x0u;}
					if (ch1_org) { m_ch1[i] = 0x0u;}
					if (ch2_org) { m_ch2[i] = 0x0u;}
					if (ch3_org) { m_ch3[i] = 0x0u;}
				}
			}

			if (idx_org) {
				for (uint32_t i = 0; i < Cout; ++i) {
					m_idx[i] = l_idx[i];
				}
				for (uint32_t i = Cout; i < 256; ++i) {
					m_idx[i] = 0x0u;
				}
			}

			uint32_t ch0_c = 0x0u;
			uint32_t ch1_c = 0x0u;
			uint32_t ch2_c = 0x0u;
			uint32_t ch3_c = 0x0u;
			uint32_t idx_c = 0x0u;

			const uint16_t v0 = ENCODE_REVOLVER(ch0_org, l_ch0, m_write, CColor, &ch0_c);
			const uint16_t v1 = ENCODE_REVOLVER(ch1_org, l_ch1, m_write + ch0_c, CColor, &ch1_c);
			const uint16_t v2 = ENCODE_REVOLVER(ch2_org, l_ch2, m_write + ch0_c + ch1_c, CColor, &ch2_c);
			const uint16_t v3 = ENCODE_REVOLVER(ch3_org, l_ch3, m_write + ch0_c + ch1_c + ch2_c, CColor, &ch3_c);
			const uint16_t v4 = ENCODE_REVOLVER(idx_org, l_idx, m_write + ch0_c + ch1_c + ch2_c + ch3_c, Cout, &idx_c);
			const uint16_t meta_code = uint16_t(((v0 * 1296u + v1 * 216u + v2 * 36u + v3 * 6u + v4) << 3u) | (qnt_idx & 0x07u));

			if(!SLIM_STREAM_WRITE(file, &meta_code, sizeof(uint16_t), 1)) { return SLIMERROR::ERROR_BLOCK; }

			uint8_t cm_size = 0x0u;

			if(ch0_org){ m_size[cm_size++] = uint8_t(ch0_c - 0x1u);}
			if(ch1_org){ m_size[cm_size++] = uint8_t(ch1_c - 0x1u);}
			if(ch2_org){ m_size[cm_size++] = uint8_t(ch2_c - 0x1u);}
			if(ch3_org){ m_size[cm_size++] = uint8_t(ch3_c - 0x1u);}
			if(idx_org){ m_size[cm_size++] = uint8_t(idx_c - 0x1u);}

			if(!SLIM_STREAM_WRITE(file, m_size, sizeof(uint8_t), cm_size))									{ return SLIMERROR::ERROR_BLOCK; }
			if(!SLIM_STREAM_WRITE(file, m_write, sizeof(uint8_t), ch0_c + ch1_c + ch2_c + ch3_c + idx_c))	{ return SLIMERROR::ERROR_BLOCK; }
		}
	}

	return SLIMERROR::ERROR_OK;
}


SLIMERROR SLIM_Read_Layer(SLIM_STREAM* file, SLIMLayerDesc* desc){

	if(file == NULL)				{return SLIMERROR::ERROR_ARG;}
	if(desc == NULL)				{return SLIMERROR::ERROR_ARG;}

	if (!SLIM_STREAM_ISOPEN(file))	{return SLIMERROR::ERROR_FILE;}

	#pragma pack(push, 1)
	struct _SLIM_LAYER_HEADER
	{
		uint16_t _id;
		uint16_t _width;
		uint16_t _height;
		uint16_t _x;
		uint16_t _y;
		uint16_t _z;
		uint8_t  _channel;
		uint8_t  _name_size;
		uint16_t _ext_size;	
	};
	#pragma pack(pop)

	_SLIM_LAYER_HEADER _slim_lh{};

	if(!SLIM_STREAM_READ(file, &_slim_lh, sizeof(_SLIM_LAYER_HEADER), 1)) 	{return SLIMERROR::ERROR_BLOCK;}

	if(_slim_lh._width == 0 || _slim_lh._height == 0) 						{return SLIMERROR::ERROR_BLOCK;}

	if(_slim_lh._channel == 0 || _slim_lh._channel > 4)						{return SLIMERROR::ERROR_BLOCK;}

	const uint8_t m_CODE_CUR 		= COUNT_SPACE_CHANNEL(_slim_lh._channel);
	const uint8_t m_CODE			= (desc->forced_code == 0) ? m_CODE_CUR : desc->forced_code;
	const uint8_t m_CHANNELS 		= SPACE_CHANNEL_COUNT(m_CODE);

	if(m_CHANNELS == 0 )			{return SLIMERROR::ERROR_NOTSUP;}

	desc->id 			= _slim_lh._id;
	desc->height 		= _slim_lh._height;
	desc->width 		= _slim_lh._width;
	desc->x 			= _slim_lh._x;
	desc->y 			= _slim_lh._y;
	desc->z 			= _slim_lh._z;
	desc->code 			= m_CODE_CUR;
	desc->name_size 	= _slim_lh._name_size;
	desc->ext_size 		= _slim_lh._ext_size;
	desc->img 			= (uint8_t*)SLIM_MALLOC(_slim_lh._width * _slim_lh._height * m_CHANNELS);

	if(_slim_lh._name_size>0){
		desc->name = (char*)SLIM_MALLOC(_slim_lh._name_size*sizeof(char));
		if(!SLIM_STREAM_READ(file, desc->name, sizeof(char), _slim_lh._name_size)) 	{return SLIMERROR::ERROR_BLOCK;}
	}

	if(_slim_lh._ext_size>0){
		desc->ext = (char*)SLIM_MALLOC(_slim_lh._ext_size*sizeof(char));
		if(!SLIM_STREAM_READ(file, desc->ext, sizeof(char), _slim_lh._ext_size)) 	{return SLIMERROR::ERROR_BLOCK;}
	}

	uint8_t* m_IMG 	 	= (uint8_t*)desc->img;

	uint8_t m_data		[1280u]{};	//Curret	block memory
	uint8_t m_read		[1280u]{};	//Read		block memory
	uint8_t m_size		[5u]{};		//Size 		blocks packed

	uint32_t qnt		= 0;
	uint16_t meta_code	= 0;

	for (uint32_t blcY = 0; blcY < _slim_lh._height; blcY += 16)
	{
		for (uint32_t blcX = 0; blcX < _slim_lh._width; blcX += 16)
		{

			if (!SLIM_STREAM_READ(file, &meta_code, sizeof(uint16_t), 1)){ return SLIMERROR::ERROR_END; }

			qnt 			= (uint32_t)(meta_code & 0x07u) << 1;
			meta_code 		>>= 0x03u;

			uint32_t t;

			t = meta_code / 1296u; const uint16_t v0 = t;  meta_code -= t * 1296u;
			t = meta_code /  216u; const uint16_t v1 = t;  meta_code -= t *  216u;
			t = meta_code /   36u; const uint16_t v2 = t;  meta_code -= t *   36u;
			t = meta_code /    6u; const uint16_t v3 = t;  meta_code -= t *    6u;
			const uint16_t v4 = meta_code;

			bool ch0_org	= (v0>0);
			bool ch1_org	= (v1>0);
			bool ch2_org	= (v2>0);
			bool ch3_org	= (v3>0);
			bool idx_org	= (v4>0);

			const uint8_t cm_size = ch0_org + ch1_org + ch2_org + ch3_org + idx_org;

			if (!SLIM_STREAM_READ(file, m_size, sizeof(uint8_t), cm_size)){ return SLIMERROR::ERROR_END; }

			uint8_t  cm_pos 			= 0x0u;
			const uint32_t cmps_ch0 	= ch0_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch1 	= ch1_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch2 	= ch2_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch3 	= ch3_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_idx 	= idx_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;

			const uint32_t st_ch1		= cmps_ch0;
			const uint32_t st_ch2		= st_ch1 + cmps_ch1;
			const uint32_t st_ch3		= st_ch2 + cmps_ch2;
			const uint32_t st_idx		= st_ch3 + cmps_ch3;
			const uint32_t st_size		= st_idx + cmps_idx;

			if (!SLIM_STREAM_READ(file, m_read, sizeof(uint8_t), st_size)){ return SLIMERROR::ERROR_END; }

			DECODE_REVOLVER(v0, m_read, m_data, cmps_ch0);
			DECODE_REVOLVER(v1, m_read + st_ch1, m_data + 256u, cmps_ch1);
			DECODE_REVOLVER(v2, m_read + st_ch2, m_data + 512u, cmps_ch2);
			DECODE_REVOLVER(v3, m_read + st_ch3, m_data + 768u, cmps_ch3);
			DECODE_REVOLVER(v4, m_read + st_idx, m_data + 1024u, cmps_idx);

			uint32_t Cout		= 0x0u;

			for (uint32_t y = 0; y < 16; ++y)
			{
				for (uint32_t x = 0; x < 16; ++x)
				{

					const uint32_t column	= blcX + x;
					const uint32_t row		= blcY + y;

					if (column >= _slim_lh._width || row >=_slim_lh._height) { continue; }

					const uint32_t index	= m_CHANNELS * (row * _slim_lh._width + column);
					const uint32_t idxclr	= m_data[1024u + Cout];

					uint32_t chn0		= (uint32_t)m_data[idxclr];
					uint32_t chn1 		= (uint32_t)m_data[idxclr + 256u];
					uint32_t chn2 		= (uint32_t)m_data[idxclr + 512u];
					uint32_t chn3 		= (uint32_t)m_data[idxclr + 768u];

					if (qnt > 0) {
						double level 	= 0.8673689 + 0.3571519 * ((double)qnt);

						const uint32_t tchn0 	= (uint32_t)(chn0*qnt + level);
						const uint32_t tchn1 	= (uint32_t)(chn1*qnt + level);
						const uint32_t tchn2 	= (uint32_t)(chn2*qnt + level);
						const uint32_t tchn3 	= (uint32_t)(chn3*qnt + level);

    					chn0 			= (uint8_t)(tchn0  > 255 ? 255 : tchn0);
						chn1 			= (uint8_t)(tchn1  > 255 ? 255 : tchn1);
						chn2 			= (uint8_t)(tchn2  > 255 ? 255 : tchn2);
						chn3 			= (uint8_t)(tchn3  > 255 ? 255 : tchn3);
					}


					switch (m_CODE)
					{
					case SLIMCODE::CODE_GRAY:
					{
						m_IMG[index]		= (chn0 + chn1 + chn2) / (_slim_lh._channel >= 3 ? 3 : _slim_lh._channel);
						break;
					}
					case SLIMCODE::CODE_RGB:
					{
						m_IMG[index]		= chn0;
						m_IMG[index + 1] 	= chn1;
						m_IMG[index + 2] 	= chn2;
						break;
					}
					case SLIMCODE::CODE_BGR:
					{
						m_IMG[index]		= chn2;
						m_IMG[index + 1] 	= chn1;
						m_IMG[index + 2] 	= chn0;
						break;
					}
					case SLIMCODE::CODE_RGBA:
					{
						m_IMG[index]		= chn0;
						m_IMG[index + 1] 	= chn1;
						m_IMG[index + 2] 	= chn2;
						m_IMG[index + 3] 	= (_slim_lh._channel < m_CHANNELS)? 255 : chn3;
						break;
					}
					case SLIMCODE::CODE_BGRA:
					{
						m_IMG[index]		= chn2;
						m_IMG[index + 1] 	= chn1;
						m_IMG[index + 2] 	= chn0;
						m_IMG[index + 3] 	= (_slim_lh._channel < m_CHANNELS)? 255 : chn3;
						break;
					}
					default:
						return SLIMERROR::ERROR_NOTSUP;
					}

					++Cout;
				}
			}
		}
	}
	return SLIMERROR::ERROR_OK;
}


SLIMERROR SLIM_Read_Layer_Map(SLIM_STREAM* file, SLIMLayerDesc* desc){

	if(file == NULL)							{return SLIMERROR::ERROR_ARG;}
	if(desc == NULL)							{return SLIMERROR::ERROR_ARG;}

	if (!SLIM_STREAM_ISOPEN(file))				{return SLIMERROR::ERROR_FILE;}

	#pragma pack(push, 1)
	struct _SLIM_LAYER_HEADER
	{
		uint16_t _id;
		uint16_t _width;
		uint16_t _height;
		uint16_t _x;
		uint16_t _y;
		uint16_t _z;
		uint8_t  _channel;
		uint8_t  _name_size;
		uint16_t _ext_size;	
	};
	#pragma pack(pop)

	_SLIM_LAYER_HEADER _slim_lh{};

	if(!SLIM_STREAM_READ(file, &_slim_lh, sizeof(_SLIM_LAYER_HEADER), 1)) 	{return SLIMERROR::ERROR_BLOCK;}

	if(_slim_lh._width == 0 || _slim_lh._height == 0) 			{return SLIMERROR::ERROR_BLOCK;}

	if(_slim_lh._channel == 0 || _slim_lh._channel > 4)			{return SLIMERROR::ERROR_BLOCK;}

	const uint8_t m_CODE_CUR 		= COUNT_SPACE_CHANNEL(_slim_lh._channel);
	const uint8_t m_CODE			= (desc->forced_code == 0) ? m_CODE_CUR : desc->forced_code;
	const uint8_t m_CHANNELS 		= SPACE_CHANNEL_COUNT(m_CODE);

	if(m_CHANNELS == 0 )			{return SLIMERROR::ERROR_NOTSUP;}

	desc->id 			= _slim_lh._id;
	desc->height 		= _slim_lh._height;
	desc->width 		= _slim_lh._width;
	desc->x 			= _slim_lh._x;
	desc->y 			= _slim_lh._y;
	desc->z 			= _slim_lh._z;
	desc->code 			= m_CODE_CUR;
	desc->name_size 	= _slim_lh._name_size;
	desc->ext_size 		= _slim_lh._ext_size;
	desc->img 			= (uint8_t*)SLIM_MALLOC(_slim_lh._width * _slim_lh._height * m_CHANNELS);

	if(_slim_lh._name_size>0){
		desc->name = (char*)SLIM_MALLOC(_slim_lh._name_size*sizeof(char));
		if(!SLIM_STREAM_READ(file, desc->name, sizeof(char), _slim_lh._name_size)) 	{return SLIMERROR::ERROR_BLOCK;}
	}

	if(_slim_lh._ext_size>0){
		desc->ext = (char*)SLIM_MALLOC(_slim_lh._ext_size*sizeof(char));
		if(!SLIM_STREAM_READ(file, desc->ext, sizeof(char), _slim_lh._ext_size)) 	{return SLIMERROR::ERROR_BLOCK;}
	}

	uint8_t* m_IMG 	 	= (uint8_t*)desc->img;

	uint8_t  m_size		[5]{};		//Size blocks packed

	uint8_t  qnt_idx	= 0;
	uint16_t meta_code	= 0;

	for (uint32_t blcY = 0; blcY < _slim_lh._height; blcY += 16)
	{
		for (uint32_t blcX = 0; blcX < _slim_lh._width; blcX += 16)
		{

			if (!SLIM_STREAM_READ(file, &meta_code, sizeof(uint16_t), 1)){ return SLIMERROR::ERROR_END; }

			qnt_idx 		= (meta_code & 0x07u);
			meta_code 		>>= 0x03u;

			uint32_t t;

			t = meta_code / 1296u; const uint16_t v0 = t;  meta_code -= t * 1296u;
			t = meta_code /  216u; const uint16_t v1 = t;  meta_code -= t *  216u;
			t = meta_code /   36u; const uint16_t v2 = t;  meta_code -= t *   36u;
			t = meta_code /    6u; const uint16_t v3 = t;  meta_code -= t *    6u;
			const uint16_t v4 = meta_code;

			bool ch0_org	= (v0>0);
			bool ch1_org	= (v1>0);
			bool ch2_org	= (v2>0);
			bool ch3_org	= (v3>0);
			bool idx_org	= (v4>0);

			const uint8_t cm_size = ch0_org + ch1_org + ch2_org + ch3_org + idx_org;

			if (!SLIM_STREAM_READ(file, m_size, sizeof(uint8_t), cm_size)){ return SLIMERROR::ERROR_END; }

			uint8_t  cm_pos 			= 0x0u;
			const uint32_t cmps_ch0 	= ch0_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch1 	= ch1_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch2 	= ch2_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch3 	= ch3_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_idx 	= idx_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;

			const uint32_t st_size		= cmps_ch0 + cmps_ch1 + cmps_ch2 + cmps_ch3 + cmps_idx;

			if (!SLIM_STREAM_SEEK(file, st_size, SLIM_STREAM_SEEK_MODE::SLIM_STREAM_SEEK_CUR)){ return SLIMERROR::ERROR_END; }

			for (uint32_t y = 0; y < 16; ++y)
			{
				for (uint32_t x = 0; x < 16; ++x)
				{

					const uint32_t column	= blcX + x;
					const uint32_t row		= blcY + y;

					if (column >= _slim_lh._width || row >=_slim_lh._height) { continue; }

					const uint32_t index	= m_CHANNELS * (row * _slim_lh._width + column);

					switch (m_CODE)
					{
					case SLIMCODE::CODE_GRAY:
					{
						m_IMG[index]		= qnt_idx;
						break;
					}
					case SLIMCODE::CODE_RGB:
					{
						m_IMG[index]		= qnt_idx;
						m_IMG[index + 1] 	= qnt_idx;
						m_IMG[index + 2] 	= qnt_idx;
						break;
					}
					case SLIMCODE::CODE_BGR:
					{
						m_IMG[index]		= qnt_idx;
						m_IMG[index + 1] 	= qnt_idx;
						m_IMG[index + 2] 	= qnt_idx;
						break;
					}
					case SLIMCODE::CODE_RGBA:
					{
						m_IMG[index]		= qnt_idx;
						m_IMG[index + 1] 	= qnt_idx;
						m_IMG[index + 2] 	= qnt_idx;
						m_IMG[index + 3] 	= 255;
						break;
					}
					case SLIMCODE::CODE_BGRA:
					{
						m_IMG[index]		= qnt_idx;
						m_IMG[index + 1] 	= qnt_idx;
						m_IMG[index + 2] 	= qnt_idx;
						m_IMG[index + 3] 	= 255;
						break;
					}
					default:
						return SLIMERROR::ERROR_NOTSUP;
					}
				}
			}
		}
	}

	return SLIMERROR::ERROR_OK;

}


SLIMERROR SLIM_Read_Layer_Info(SLIM_STREAM* file, SLIMLayerInfoDesc* desc){

	if(file == NULL)							{return SLIMERROR::ERROR_ARG;}
	if(desc == NULL)							{return SLIMERROR::ERROR_ARG;}

	if (!SLIM_STREAM_ISOPEN(file))				{return SLIMERROR::ERROR_FILE;}

	#pragma pack(push, 1)
	struct _SLIM_LAYER_HEADER
	{
		uint16_t _id;
		uint16_t _width;
		uint16_t _height;
		uint16_t _x;
		uint16_t _y;
		uint16_t _z;
		uint8_t  _channel;
		uint8_t  _name_size;
		uint16_t _ext_size;	
	};
	#pragma pack(pop)

	_SLIM_LAYER_HEADER _slim_lh{};

	if(!SLIM_STREAM_READ(file, &_slim_lh, sizeof(_SLIM_LAYER_HEADER), 1)) 	{return SLIMERROR::ERROR_BLOCK;}

	if(_slim_lh._width == 0 || _slim_lh._height == 0) 			{return SLIMERROR::ERROR_BLOCK;}

	if(_slim_lh._channel == 0 || _slim_lh._channel > 4)			{return SLIMERROR::ERROR_BLOCK;}

	const uint8_t m_CODE 		= COUNT_SPACE_CHANNEL(_slim_lh._channel);

	desc->id 					= _slim_lh._id;
	desc->height 				= _slim_lh._height;
	desc->width 				= _slim_lh._width;
	desc->x 					= _slim_lh._x;
	desc->y 					= _slim_lh._y;
	desc->z 					= _slim_lh._z;
	desc->code 					= m_CODE;
	desc->name_size 			= _slim_lh._name_size;
	desc->ext_size 				= _slim_lh._ext_size;
	desc->block_256_all			= 0;
	desc->block_256_exist		= 0;
	desc->block_256_empty		= 0;
	desc->block_color_table_max	= 0;
	desc->block_color_table_min	= 0;
	desc->block_color_table_avg	= 0;
	desc->block_q_max			= 0;
	desc->block_q_min			= 0;
	desc->block_q_avg			= 0;
	desc->all_c					= 0;
	desc->reuse_c				= 0;
	desc->origin_c				= 0;
	desc->rle_c					= 0;
	desc->rice_c				= 0;
	desc->sldd_c				= 0;
	desc->maskared_c			= 0;


	if(_slim_lh._name_size>0){
		desc->name = (char*)SLIM_MALLOC(_slim_lh._name_size*sizeof(char));
		if(!SLIM_STREAM_READ(file, desc->name, sizeof(char), _slim_lh._name_size)) 	{return SLIMERROR::ERROR_BLOCK;}
	}

	if(_slim_lh._ext_size>0){
		desc->ext = (char*)SLIM_MALLOC(_slim_lh._ext_size*sizeof(char));
		if(!SLIM_STREAM_READ(file, desc->ext, sizeof(char), _slim_lh._ext_size)) 	{return SLIMERROR::ERROR_BLOCK;}
	}

	uint8_t m_data		[1280]{};	//Curret	block memory
	uint8_t m_read		[1280]{};	//Read		block memory
	uint8_t m_size		[5]{};		//Size 		blocks packed

	uint32_t qnt		= 0;
	uint16_t meta_code	= 0;

	for (uint32_t blcY = 0; blcY < _slim_lh._height; blcY += 16)
	{
		for (uint32_t blcX = 0; blcX < _slim_lh._width; blcX += 16)
		{

			if (!SLIM_STREAM_READ(file, &meta_code, sizeof(uint16_t), 1)){ return SLIMERROR::ERROR_END; }

			qnt 			= (meta_code & 0x07u) << 1;
			meta_code 		>>= 0x03u;

			uint32_t t;

			t = meta_code / 1296u; const uint16_t v0 = t;  meta_code -= t * 1296u;
			t = meta_code /  216u; const uint16_t v1 = t;  meta_code -= t *  216u;
			t = meta_code /   36u; const uint16_t v2 = t;  meta_code -= t *   36u;
			t = meta_code /    6u; const uint16_t v3 = t;  meta_code -= t *    6u;
			const uint16_t v4 = meta_code;

			bool ch0_org		= (v0>0);
			bool ch1_org		= (v1>0);
			bool ch2_org		= (v2>0);
			bool ch3_org		= (v3>0);
			bool idx_org		= (v4>0);

			const uint8_t cm_size = ch0_org + ch1_org + ch2_org + ch3_org + idx_org;
		
			desc->reuse_c		+= (v0==0)+(v1==0)+(v2==0)+(v3==0)+(v4==0);
			desc->origin_c		+= (v0==1)+(v1==1)+(v2==1)+(v3==1)+(v4==1);
			desc->rle_c			+= (v0==2)+(v1==2)+(v2==2)+(v3==2)+(v4==2);
			desc->rice_c		+= (v0==3)+(v1==3)+(v2==3)+(v3==3)+(v4==3);
			desc->sldd_c	 	+= (v0==4)+(v1==4)+(v2==4)+(v3==4)+(v4==4);
			desc->maskared_c	+= (v0==5)+(v1==5)+(v2==5)+(v3==5)+(v4==5);


			
			if(ch0_org || ch1_org || ch2_org || ch3_org){
				if(desc->block_q_max<qnt){desc->block_q_max=qnt;}
				if(desc->block_q_min>qnt){desc->block_q_min=qnt;}
			}

			desc->block_256_all++;
			desc->block_256_exist 	+= (cm_size>0);
			desc->block_256_empty 	+= (cm_size==0);
			desc->block_q_avg 		+= qnt;

			if (!SLIM_STREAM_READ(file, m_size, sizeof(uint8_t), cm_size)){ return SLIMERROR::ERROR_END; }

			uint8_t  cm_pos 			= 0x0u;
			const uint32_t cmps_ch0 	= ch0_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch1 	= ch1_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch2 	= ch2_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_ch3 	= ch3_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;
			const uint32_t cmps_idx 	= idx_org ? 0x1u + ((uint32_t)m_size[cm_pos++]) : 0x0u;

			const uint32_t st_ch1		= cmps_ch0;
			const uint32_t st_ch2		= st_ch1 + cmps_ch1;
			const uint32_t st_ch3		= st_ch2 + cmps_ch2;
			const uint32_t st_idx		= st_ch3 + cmps_ch3;
			const uint32_t st_size		= st_idx + cmps_idx;

			if (!SLIM_STREAM_READ(file, m_read, sizeof(uint8_t), st_size)){ return SLIMERROR::ERROR_END; }

			DECODE_REVOLVER(v0, m_read, m_data, cmps_ch0);
			DECODE_REVOLVER(v1, m_read + st_ch1, m_data + 256, cmps_ch1);
			DECODE_REVOLVER(v2, m_read + st_ch2, m_data + 512, cmps_ch2);
			DECODE_REVOLVER(v3, m_read + st_ch3, m_data + 768, cmps_ch3);
			DECODE_REVOLVER(v4, m_read + st_idx, m_data + 1024, cmps_idx);

			uint32_t lc_blk_max = 0;
			uint32_t lc_blk_min = 0xFFFFFFFFu;
			if(idx_org){
				for(uint32_t idx = 0; idx<256; ++idx){
					uint32_t idxclr = m_data[1024 + idx]+1;
					if(desc->block_color_table_min>idxclr){desc->block_color_table_min=idxclr;}
					if(desc->block_color_table_max<idxclr){desc->block_color_table_max=idxclr;}			
					if(lc_blk_min>idxclr){lc_blk_min=idxclr;}
					if(lc_blk_max<idxclr){lc_blk_max=idxclr;}
				}
				desc->block_color_table_avg += lc_blk_max;
			}
			
		}
	}
	desc->all_c 		+= desc->reuse_c;
	desc->all_c 		+= desc->origin_c;
	desc->all_c 		+= desc->rle_c;
	desc->all_c 		+= desc->rice_c;
	desc->all_c 		+= desc->sldd_c;
	desc->all_c 		+= desc->maskared_c;
	desc->block_q_avg /= desc->block_256_all;
	desc->block_color_table_avg /= desc->block_256_exist;


	return SLIMERROR::ERROR_OK;
}

#endif // SLEP_SLIM_IMP
#endif // SLIM_H