#pragma once

#include <yae/types.h>

#include <mirror/mirror.h>

namespace yae {
namespace editor {

class MirrorInspector
{
public:

	void update();

	bool opened = false;
//private:

	MIRROR_CLASS_NOVIRTUAL(MirrorInspector)
	(
		MIRROR_MEMBER(opened)();
	);
};

} // namespace editor
} // namespace yae
