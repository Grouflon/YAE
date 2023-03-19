#pragma once

#include <yae/types.h>

#define IM_ASSERT(_EXPR)  YAE_ASSERT(_EXPR)

#define IM_VEC2_CLASS_EXTRA                                                 \
        ImVec2(const yae::Vector2& f) { x = f.x; y = f.y; }                       \
        operator yae::Vector2() const { return yae::Vector2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
        ImVec4(const yae::Vector4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }     \
        operator yae::Vector4() const { return yae::Vector4(x,y,z,w); }
