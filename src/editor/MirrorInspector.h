#pragma once

#include <yae/types.h>

#include <mirror/mirror.h>

namespace yae {
namespace editor {

class MirrorInspector
{
public:
	bool update();

	bool opened = false;
};

} // namespace editor
} // namespace yae
