#pragma once

#include <yae/types.h>

#include <editor/ResourceEditor.h>
#include <editor/MirrorInspector.h>
#include <editor/InputInspector.h>

#include <mirror/mirror.h>

namespace yae {

class ShaderProgram;

namespace editor {

class Editor
{
public:
	void init();
	void shutdown();
	void reload();
	void update(float _dt);
	bool serialize(yae::Serializer* _serializer);

	bool showMemoryProfiler = false;
	bool showFrameRate = false;
	bool showMirrorDebugWindow = false;
	bool showRendererDebugWindow = false;
	bool showDemoWindow = false;

	ShaderProgram* wireframeShader = nullptr;
	ShaderProgram* normalsShader = nullptr;

	ResourceEditor resourceEditor;
	InputInspector inputInspector;
	MirrorInspector mirrorInspector;

	// mirror window
	mirror::TypeID selectedTypeID = mirror::UNDEFINED_TYPEID;

//private:
	void _displayTypeTreeNode(mirror::Type* _type, mirror::Type* _parent = nullptr);
};

} // namespace editor
} // namespace yae