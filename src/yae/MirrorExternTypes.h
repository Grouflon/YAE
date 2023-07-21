#pragma once

#include <yae/types.h>
#include <yae/resources/ResourceID.h>

#include <mirror.h>

namespace yae {

MIRROR_EXTERN_CLASS(String)
(
);

MIRROR_EXTERN_CLASS(Vector2, SerializeType = float, SerializeArraySize = 2)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
);

MIRROR_EXTERN_CLASS(Vector3, SerializeType = float, SerializeArraySize = 3)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
	MIRROR_MEMBER(z)();
);

MIRROR_EXTERN_CLASS(Vector4, SerializeType = float, SerializeArraySize = 4)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
	MIRROR_MEMBER(z)();
	MIRROR_MEMBER(w)();
);

MIRROR_EXTERN_CLASS(Quaternion, SerializeType = float, SerializeArraySize = 4)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
	MIRROR_MEMBER(z)();
	MIRROR_MEMBER(w)();
);

MIRROR_EXTERN_CLASS(Matrix3)
(
	MIRROR_MEMBER(m)();
);

MIRROR_EXTERN_CLASS(Matrix4)
(
	MIRROR_MEMBER(m)();
);

MIRROR_EXTERN_CLASS(Transform)
(
	MIRROR_MEMBER(position)();
	MIRROR_MEMBER(rotation)();
	MIRROR_MEMBER(scale)();
);

MIRROR_EXTERN_CLASS(ResourceID, SerializeType = u32)();

} // namespace yae
