#pragma once

#include <yae/types.h>
#include <yae/math_types.h>
#include <yae/memory.h>

// User-defined assertion handler (default is cassert assert()).
#define IM3D_ASSERT(e) YAE_ASSERT(e)

// User-defined malloc/free. Define both or neither (default is cstdlib malloc()/free()).
#define IM3D_MALLOC(size) ::yae::toolAllocator().allocate(size)
#define IM3D_FREE(ptr) ::yae::toolAllocator().deallocate(ptr)

// User-defined API declaration (e.g. __declspec(dllexport)).
#define IM3D_API YAE_API

// Use a thread-local context pointer.
//#define IM3D_THREAD_LOCAL_CONTEXT_PTR 1

// Use row-major internal matrix layout.
//#define IM3D_MATRIX_ROW_MAJOR 1

// Force vertex data alignment (default is 4 bytes).
//#define IM3D_VERTEX_ALIGNMENT 4

// Enable internal culling for primitives (everything drawn between Begin*()/End()). The application must set a culling frustum via AppData.
//#define IM3D_CULL_PRIMITIVES 1

// Enable internal culling for gizmos. The application must set a culling frustum via AppData.
//#define IM3D_CULL_GIZMOS 1

// Conversion to/from application math types.
#define IM3D_VEC2_APP \
	Vec2(const yae::Vector2& _v)          { x = _v.x; y = _v.y;     } \
	operator yae::Vector2() const         { return yae::Vector2(x, y); }
#define IM3D_VEC3_APP \
	Vec3(const yae::Vector3& _v)          { x = _v.x; y = _v.y; z = _v.z; } \
	operator yae::Vector3() const         { return yae::Vector3(x, y, z);    }
#define IM3D_VEC4_APP \
	Vec4(const yae::Vector4& _v)          { x = _v.x; y = _v.y; z = _v.z; w = _v.w; } \
	operator yae::Vector4() const         { return yae::Vector4(x, y, z, w);           }
#define IM3D_MAT3_APP \
	Mat3(const yae::Matrix3& _m)          { for (int i = 0; i < 9; ++i) m[i] = *(&(_m[0][0]) + i); } \
	operator yae::Matrix3() const         { yae::Matrix3 ret; for (int i = 0; i < 9; ++i) *(&(ret[0][0]) + i) = m[i]; return ret; }
#define IM3D_MAT4_APP \
	Mat4(const yae::Matrix4& _m)          { for (int i = 0; i < 16; ++i) m[i] = *(&(_m[0][0]) + i); } \
	operator yae::Matrix4() const         { yae::Matrix4 ret; for (int i = 0; i < 16; ++i) *(&(ret[0][0]) + i) = m[i]; return ret; }
