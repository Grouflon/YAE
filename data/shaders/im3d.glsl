#version 450
#if !defined(POINTS) && !defined(LINES) && !defined(TRIANGLES)
	#error No primitive type defined
#endif
#if !defined(VERTEX_SHADER) && !defined(GEOMETRY_SHADER) && !defined(FRAGMENT_SHADER)
	#error No shader stage defined
#endif

#define VertexData \
	_VertexData { \
		noperspective float m_edgeDistance; \
		noperspective float m_size; \
		smooth vec4 m_color; \
	}

#define kAntialiasing 2.0

#ifdef VERTEX_SHADER
	layout(binding = 0) uniform UniformBufferObject {
		mat4 viewProjMatrix;
	} u;
	
	layout(location=0) in vec4 aPositionSize;
	layout(location=1) in vec4 aColor;
	
	layout(location = 0) out VertexData vData;
	
	void main() 
	{
		vData.m_color = aColor.abgr; // swizzle to correct endianness
		#if !defined(TRIANGLES)
			vData.m_color.a *= smoothstep(0.0, 1.0, aPositionSize.w / kAntialiasing);
		#endif
		vData.m_size = max(aPositionSize.w, kAntialiasing);
		gl_Position = u.viewProjMatrix * vec4(aPositionSize.xyz, 1.0);
		#if defined(POINTS)
			gl_PointSize = vData.m_size;
		#endif
	}
#endif

#ifdef GEOMETRY_SHADER
	layout(binding = 1) uniform UniformBufferObject {
		vec2 viewport;
	} u;

	// expand line -> triangle strip
	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;
	
	layout(location = 0) in  VertexData vData[];
	layout(location = 0) out VertexData vDataOut;
	
	void main() 
	{
		vec2 pos0 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
		vec2 pos1 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
		
		vec2 dir = pos0 - pos1;
		dir = normalize(vec2(dir.x, dir.y * u.viewport.y / u.viewport.x)); // correct for aspect ratio
		vec2 tng0 = vec2(-dir.y, dir.x);
		vec2 tng1 = tng0 * vData[1].m_size / u.viewport;
		tng0 = tng0 * vData[0].m_size / u.viewport;
		
	 // line start
		gl_Position = vec4((pos0 - tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw); 
		vDataOut.m_edgeDistance = -vData[0].m_size;
		vDataOut.m_size = vData[0].m_size;
		vDataOut.m_color = vData[0].m_color;
		EmitVertex();
		
		gl_Position = vec4((pos0 + tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw);
		vDataOut.m_color = vData[0].m_color;
		vDataOut.m_edgeDistance = vData[0].m_size;
		vDataOut.m_size = vData[0].m_size;
		EmitVertex();
		
	 // line end
		gl_Position = vec4((pos1 - tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
		vDataOut.m_edgeDistance = -vData[1].m_size;
		vDataOut.m_size = vData[1].m_size;
		vDataOut.m_color = vData[1].m_color;
		EmitVertex();
		
		gl_Position = vec4((pos1 + tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
		vDataOut.m_color = vData[1].m_color;
		vDataOut.m_size = vData[1].m_size;
		vDataOut.m_edgeDistance = vData[1].m_size;
		EmitVertex();
	}
#endif

#ifdef FRAGMENT_SHADER
	layout(location = 0) in VertexData vData;
	
	layout(location = 0) out vec4 fResult;
	
	void main() 
	{
		fResult = vData.m_color;
		
		#if   defined(LINES)
			float d = abs(vData.m_edgeDistance) / vData.m_size;
			d = smoothstep(1.0, 1.0 - (kAntialiasing / vData.m_size), d);
			fResult.a *= d;
			
		#elif defined(POINTS)
			float d = length(gl_PointCoord.xy - vec2(0.5));
			d = smoothstep(0.5, 0.5 - (kAntialiasing / vData.m_size), d);
			fResult.a *= d;
			
		#endif		
	}
#endif
