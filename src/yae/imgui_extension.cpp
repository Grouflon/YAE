#include "imgui_extension.h"

#include <yae/math_3d.h>

#include <imgui/imgui.h>

namespace ImGui {

bool DragVector3(const char* _label, yae::Vector3& _v, float _speed, float _min, float _max)
{
	return DragFloat3(_label, yae::math::data(_v), _speed, _min, _max);
}

bool DragRotation(const char* _label, yae::Quaternion& _q)
{
	yae::Vector3 euler = yae::math::euler(_q) * yae::R2D;
	bool result = DragVector3(_label, euler);
	_q = yae::Quaternion::FromEuler(euler * yae::D2R);
	return result;
}


} // namespace ImGui

namespace yae {

void imgui_matrix4(const Matrix4& _matrix)
{
	imgui_matrix4(math::data(_matrix));
}

void imgui_matrix4(const float _matrix[16])
{
	ImGui::Text("%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f",
	_matrix[0], _matrix[1], _matrix[2], _matrix[3],
	_matrix[4], _matrix[5], _matrix[6], _matrix[7],
	_matrix[8], _matrix[9], _matrix[10], _matrix[11],
	_matrix[12], _matrix[13], _matrix[14], _matrix[15]
	);
}

void imgui_matrix3(const Matrix3& _matrix)
{
	imgui_matrix3(math::data(_matrix));
}

void imgui_matrix3(const float _matrix[9])
{
	ImGui::Text("%.4f %.4f %.4f\n%.4f %.4f %.4f\n%.4f %.4f %.4f",
	_matrix[0], _matrix[1], _matrix[2],
	_matrix[3], _matrix[4], _matrix[5],
	_matrix[6], _matrix[7], _matrix[8]
	);
}

void imgui_quaternion(const Quaternion& _q)
{
	ImGui::Text("%.4f %.4f %.4f %.4f",
		_q.x, _q.y, _q.z, _q.w
	);
}

} // namespace yae
