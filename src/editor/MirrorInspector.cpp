#include "MirrorInspector.h"

#include <yae/imgui_extension.h>

#include <imgui/imgui.h>

namespace yae {
namespace editor {

MIRROR_CLASS_DEFINITION(yae::editor::MirrorInspector);

enum TestEnum
{
	EnumValue1 = 5,
	EnumValue2,
	EnumValue3,
	EnumValue_COUNT = 3,
};
MIRROR_ENUM(TestEnum)
(
	MIRROR_ENUM_VALUE(EnumValue1)()
	MIRROR_ENUM_VALUE(EnumValue2)()
	MIRROR_ENUM_VALUE(EnumValue3)()
);
MIRROR_ENUM_DEFINITION(TestEnum);

enum class TestEnumClass : u16
{
	EnumClassValue1 = 7,
	EnumClassValue2,
	EnumClassValue3,
	EnumClassValue_COUNT = 3,
};
MIRROR_ENUM_CLASS(TestEnumClass)
(
	MIRROR_ENUM_CLASS_VALUE(EnumClassValue1)()
	MIRROR_ENUM_CLASS_VALUE(EnumClassValue2)()
	MIRROR_ENUM_CLASS_VALUE(EnumClassValue3)()
);
MIRROR_ENUM_CLASS_DEFINITION(TestEnumClass);

struct TestData
{
	bool boolean = false;
	bool hiddenVariable = false;

	char character = 0;

	u8 unsigned8 = 0;
	u16 unsigned16 = 0;
	u32 unsigned32 = 0;
	u64 unsigned64 = 0;

	i8 signed8 = 0;
	i16 signed16 = 0;
	i32 signed32 = 0;
	i64 signed64 = 0;

	float float32 = 0;
	double float64 = 0;

	TestEnum testEnum = EnumValue1;
	TestEnumClass testEnumClass = TestEnumClass::EnumClassValue1;

	String string;

	Vector2 vector2;
	Vector3 vector3;
	Vector4 vector4;
	Quaternion quaternion;
	Matrix3 matrix3 = Matrix3::IDENTITY;
	Matrix4 matrix4 = Matrix4::IDENTITY;

	Matrix4 matrixArray[4];

	MIRROR_CLASS_NOVIRTUAL(yae::editor::TestData)
	(
		MIRROR_MEMBER(boolean)();
		MIRROR_MEMBER(hiddenVariable)(Hidden);

		MIRROR_MEMBER(character)();

		MIRROR_MEMBER(unsigned8)();
		MIRROR_MEMBER(unsigned16)();
		MIRROR_MEMBER(unsigned32)();
		MIRROR_MEMBER(unsigned64)();

		MIRROR_MEMBER(signed8)();
		MIRROR_MEMBER(signed16)();
		MIRROR_MEMBER(signed32)();
		MIRROR_MEMBER(signed64)();

		MIRROR_MEMBER(float32)();
		MIRROR_MEMBER(float64)();

		MIRROR_MEMBER(testEnum)();
		MIRROR_MEMBER(testEnumClass)();

		MIRROR_MEMBER(string)();

		MIRROR_MEMBER(vector2)();
		MIRROR_MEMBER(vector3)();
		MIRROR_MEMBER(vector4)();
		MIRROR_MEMBER(quaternion)();
		MIRROR_MEMBER(matrix3)();
		MIRROR_MEMBER(matrix4)();

		MIRROR_MEMBER(matrixArray)();
	);
};
MIRROR_CLASS_DEFINITION(yae::editor::TestData);

void MirrorInspector::update()
{
	static TestData s_testData;

	if (ImGui::Begin("MirrorInspector", &opened))
	{
		ImGui::EditMirrorClassInstance(&s_testData, s_testData.getClass());
	}
	ImGui::End();
}

} // namespace editor
} // namespace yae
