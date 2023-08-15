#include "render_types.h"

#include <mirror/mirror.h>

MIRROR_CLASS(yae::TextureParameters)
(
	MIRROR_MEMBER(filter);
);

MIRROR_ENUM(yae::TextureFilter)
(
	MIRROR_ENUM_VALUE(yae::TextureFilter::LINEAR);
	MIRROR_ENUM_VALUE(yae::TextureFilter::NEAREST);
);

MIRROR_ENUM(yae::ShaderType)
(
	MIRROR_ENUM_VALUE(yae::ShaderType::UNDEFINED);
	MIRROR_ENUM_VALUE(yae::ShaderType::VERTEX);
	MIRROR_ENUM_VALUE(yae::ShaderType::GEOMETRY);
	MIRROR_ENUM_VALUE(yae::ShaderType::FRAGMENT);
);
