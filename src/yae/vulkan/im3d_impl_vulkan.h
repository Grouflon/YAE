#pragma once

#include <yae/types.h>

namespace yae
{

class ShaderResource;

struct im3d_Instance
{
	ShaderResource* vertexShader;
	ShaderResource* geometryShader;
	ShaderResource* fragmentShader;
};

im3d_Instance* im3d_Init();
void im3d_Shutdown(im3d_Instance* _context);
void im3d_NewFrame(im3d_Instance* _context);
void im3d_EndFrame(im3d_Instance* _context);

} // namespace yae
