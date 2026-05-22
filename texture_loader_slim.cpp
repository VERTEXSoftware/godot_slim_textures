


#include "texture_loader_slim.h"

#include "core/io/file_access.h"
#include "core/io/file_access_memory.h"
#include "scene/resources/image_texture.h"

#include "core/error/error_macros.h"
#include "core/object/class_db.h"

#include "core/string/print_string.h"
#include "core/string/ustring.h"

#define SLIM_MAGIC 0x4D494C5373696854
#define SLIM_VERSION_MAJOR 1
#define SLIM_VERSION_MINOR 4
#define SLIM_VERSION_BUGFIX 0
#define SLIM_VERSION_HOTFIX 0

#define SLIM_VERSION ((SLIM_VERSION_MAJOR << 24) | (SLIM_VERSION_MINOR << 16) | (SLIM_VERSION_BUGFIX << 8) | (SLIM_VERSION_HOTFIX))

#define SLIM_STREAM_IMP
#define SLEP_SLDD_IMP
#define SLEP_MASKARED_IMP
#define RLE_IMP
#define RICE_IMP

//Custom Compression
#include "compress/SLDD.h"
#include "compress/MASKARED.h"
#include "compress/RLE.h"
#include "compress/RICE.h"

//Uncomment to enable texture loading logs
//#define SLIM_DEBUG_LOG

#pragma pack(push, 1)
struct _SLIM_HEADER
{
	uint64_t _magic;
	uint32_t _version;
	uint16_t _canvas_width;
	uint16_t _canvas_height;
	uint8_t  _canvas_channel;
	uint16_t _layers;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SLIM_LAYER_HEADER
{
	uint16_t _id;
	uint16_t _width;
	uint16_t _height;
	uint16_t _x;
	uint16_t _y;
	uint16_t _z;
	uint8_t	 _mipmaps;
	uint8_t  _channel;
	uint8_t  _name_size;
	uint16_t _ext_size;
};
#pragma pack(pop)


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


static Ref<Image> load_slim_from_file_access(Ref<FileAccess> f, Error *r_error) {

	if (r_error){*r_error = ERR_FILE_CORRUPT;}

	_SLIM_HEADER _slim_h{};

	f->get_buffer((uint8_t*)&_slim_h, sizeof(_SLIM_HEADER));

	if (_slim_h._magic != (uint64_t)SLIM_MAGIC) 			{ERR_FAIL_V_MSG(Ref<Image>(), "[SLIM_LOAD_ERROR][Invalid SLIM magic number]");	}
	if (_slim_h._version != (uint64_t)SLIM_VERSION) 		{ERR_FAIL_V_MSG(Ref<Image>(), "[SLIM_LOAD_ERROR][Invalid SLIM version]");		}

	_SLIM_LAYER_HEADER _slim_lh{};

	f->get_buffer((uint8_t*)&_slim_lh, sizeof(_SLIM_LAYER_HEADER));

	uint32_t pos = f->get_position();

	f->seek(pos + _slim_lh._name_size + _slim_lh._ext_size);

	if (_slim_lh._channel <1 || _slim_lh._channel > 4)	{ERR_FAIL_V_MSG(Ref<Image>(), "[SLIM_LOAD_ERROR][Invalid SLIM Layer channels]");}

	Image::Format img_format = Image::FORMAT_RGBA8;

	switch (_slim_lh._channel)
	{
		case 1:
			img_format = Image::FORMAT_L8;
			break;
		case 2:
			img_format = Image::FORMAT_LA8;
			break;
		case 3:
			img_format = Image::FORMAT_RGB8;
			break;
		case 4:
			img_format = Image::FORMAT_RGBA8;
			break;
		default:
			ERR_FAIL_V_MSG(Ref<Image>(), "[SLIM_LOAD_ERROR][Invalid SLIM Layer channels]");
	}

	uint8_t m_data		[1280u]{};	//Curret	block memory
	uint8_t m_read		[1280u]{};	//Read		block memory
	uint8_t m_size		[5u]{};		//Size 		blocks packed

	uint32_t qnt		= 0;
	uint16_t meta_code	= 0;

	uint64_t data_size = (uint64_t)(_slim_lh._height * _slim_lh._width * _slim_lh._channel);
	Vector<uint8_t> data;
	data.resize(data_size);

	uint8_t *ptr = data.ptrw();

	for (uint32_t blcY = 0; blcY < _slim_lh._height; blcY += 16)
	{
		for (uint32_t blcX = 0; blcX < _slim_lh._width; blcX += 16)
		{

			f->get_buffer((uint8_t*)&meta_code,  sizeof(uint16_t));

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

			f->get_buffer(m_size,  cm_size);

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

			f->get_buffer(m_read,  st_size);

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

					const uint32_t index	= _slim_lh._channel * (row * _slim_lh._width + column);
					const uint32_t idxclr	= m_data[1024u + Cout];

					++Cout;

					uint32_t chn0			= (uint32_t)m_data[idxclr];
					uint32_t chn1 			= (uint32_t)m_data[idxclr + 256u];
					uint32_t chn2 			= (uint32_t)m_data[idxclr + 512u];
					uint32_t chn3 			= (uint32_t)m_data[idxclr + 768u];

					if (qnt > 0) {
						double level 			= 0.8673689 + 0.3571519 * ((double)qnt);

						const uint32_t tchn0 	= (uint32_t)(chn0*qnt + level);
						const uint32_t tchn1 	= (uint32_t)(chn1*qnt + level);
						const uint32_t tchn2 	= (uint32_t)(chn2*qnt + level);
						const uint32_t tchn3 	= (uint32_t)(chn3*qnt + level);

    					chn0 					= (uint8_t)(tchn0  > 255 ? 255 : tchn0);
						chn1 					= (uint8_t)(tchn1  > 255 ? 255 : tchn1);
						chn2 					= (uint8_t)(tchn2  > 255 ? 255 : tchn2);
						chn3 					= (uint8_t)(tchn3  > 255 ? 255 : tchn3);
					}

					switch (img_format)
					{

						case Image::Image::FORMAT_L8:
						{
							ptr[index]		= (uint8_t)chn0;
							break;
						}
						case Image::Image::FORMAT_LA8:
						{
							ptr[index]		= (uint8_t)chn0;
							ptr[index + 1] 	= (uint8_t)chn3;
							break;
						}
						case Image::FORMAT_RGB8:
						{
							ptr[index]		= (uint8_t)chn0;
							ptr[index + 1] 	= (uint8_t)chn1;
							ptr[index + 2] 	= (uint8_t)chn2;
							break;
						}
						case Image::FORMAT_RGBA8:
						{
							ptr[index]		= (uint8_t)chn0;
							ptr[index + 1] 	= (uint8_t)chn1;
							ptr[index + 2] 	= (uint8_t)chn2;
							ptr[index + 3] 	= (uint8_t)chn3;
							break;
						}
						default:
							ERR_FAIL_V_MSG(Ref<Image>(), "[SLIM_LOAD_ERROR][Unsupported SLIM format: " + itos(_slim_lh._channel)+"]");
					}
				}
			}
		}
	}

	Ref<Image> image = memnew(Image(_slim_lh._width, _slim_lh._height, 0, img_format, data));

	if(_slim_lh._mipmaps>0){
		image->generate_mipmaps();
	}

	if (r_error){*r_error = OK;}
	return image;
}


Ref<Resource> ResourceFormatSLIM::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error){*r_error = ERR_CANT_OPEN;}

	Error err;
	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);
	if (f.is_null()) {
		ERR_FAIL_V_MSG(Ref<Resource>(), "[SLIM_LOAD_ERROR][" + p_path+"]");
	}
	
	#ifdef SLIM_DEBUG_LOG
		uint64_t start_time = OS::get_singleton()->get_ticks_usec();
	#endif

	Ref<Image> img = load_slim_from_file_access(f, r_error);

	#ifdef SLIM_DEBUG_LOG
		uint64_t end_time = OS::get_singleton()->get_ticks_usec();
		double ms = (end_time - start_time) / 1000.0;
	#endif

	if (img.is_null()) {
		return Ref<Resource>();
	}

	Ref<ImageTexture> texture = ImageTexture::create_from_image(img);
	
	#ifdef SLIM_DEBUG_LOG
		printf("[SLIM_LOAD_OK][%.3f ms][%s]\n", ms, p_path.utf8().get_data());
	#endif
	
	return texture;
}

void ResourceFormatSLIM::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("slim");
}

bool ResourceFormatSLIM::handles_type(const String &p_type) const {
	return ClassDB::is_parent_class(p_type, "Texture2D");
}

String ResourceFormatSLIM::get_resource_type(const String &p_path) const {
	if (p_path.get_extension().to_lower() == "slim") {
		return "Texture2D";
	}
	return "";
}

ResourceFormatSLIM::ResourceFormatSLIM() {
	printf("This SLIM (Sleptsov Image Decoding Module)\n"
           "Version: %d.%d.%d.%d\n"
           "Copyright (C) 2026 VERTEX Software by Sleptsov Vladimir\n\n",
           SLIM_VERSION_MAJOR, SLIM_VERSION_MINOR, SLIM_VERSION_BUGFIX, SLIM_VERSION_HOTFIX);
}
