#include "Renderer.h"

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

} // namespace yae
