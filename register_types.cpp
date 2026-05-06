

#include "register_types.h"

#include "texture_loader_slim.h"

#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "scene/resources/image_texture.h"

static Ref<ResourceFormatSLIM> resource_loader_slim;

void initialize_godot_slim_textures_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	if constexpr (GD_IS_CLASS_ENABLED(ImageTexture)) {	
		resource_loader_slim.instantiate();
		ResourceLoader::add_resource_format_loader(resource_loader_slim);
	}
}

void uninitialize_godot_slim_textures_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	if constexpr (GD_IS_CLASS_ENABLED(ImageTexture)) {
		ResourceLoader::remove_resource_format_loader(resource_loader_slim);
		resource_loader_slim.unref();
	}
}