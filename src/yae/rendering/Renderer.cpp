#include "Renderer.h"

#include <yae/Mesh.h>

namespace yae {

void Renderer::setViewProjectionMatrix(const Matrix4& _view, const Matrix4& _proj)
{
	m_viewMatrix = _view;
	m_projMatrix = _proj;
}

Matrix4 Renderer::getViewProjectionMatrix() const
{
	return m_projMatrix * m_viewMatrix;	
}

void Renderer::drawMesh(const Matrix4& _transform, const Mesh& _mesh, const TextureHandle& _textureHandle)
{
	drawMesh(
		_transform,
		_mesh.vertices.data(), _mesh.vertices.size(),
		_mesh.indices.data(), _mesh.indices.size(),
		_textureHandle
	);
}


} // namespace yae
