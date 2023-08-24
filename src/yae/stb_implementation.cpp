#include <yae/types.h>

#include <core/memory.h>

void* yae_stb_malloc(size_t _sz)
{
	return yae::defaultAllocator().allocate(_sz);
}

void* yae_stb_realloc(void* _p, size_t _newsz)
{
	return yae::defaultAllocator().reallocate(_p, _newsz);
}

void yae_stb_free(void* _p)
{
	yae::defaultAllocator().deallocate(_p);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#define STBI_ASSERT(x)            YAE_ASSERT(x)
#define STBI_MALLOC(sz)           yae_stb_malloc(sz);
#define STBI_REALLOC(p,newsz)     yae_stb_realloc(p, newsz);
#define STBI_FREE(p)              yae_stb_free(p);
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#pragma clang diagnostic pop


#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define STBTT_malloc(x,u)          yae_stb_malloc(x)
#define STBTT_free(x, u)           yae_stb_free(x)
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
