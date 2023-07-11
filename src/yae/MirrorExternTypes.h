#pragma once

#include <yae/types.h>

#include <mirror.h>

namespace yae {

MIRROR_EXTERN_CLASS(String)
(
);

MIRROR_EXTERN_CLASS(Vector2, FloatArray = 2)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
);

MIRROR_EXTERN_CLASS(Vector3, FloatArray = 3)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
	MIRROR_MEMBER(z)();
);

MIRROR_EXTERN_CLASS(Vector4, FloatArray = 4)
(
	MIRROR_MEMBER(x)();
	MIRROR_MEMBER(y)();
	MIRROR_MEMBER(z)();
	MIRROR_MEMBER(w)();
);

MIRROR_EXTERN_CLASS(Quaternion, FloatArray = 4)
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

} // namespace yae