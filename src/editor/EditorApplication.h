#pragma once

#include <yae/types.h>

#include <yae/Application.h>

#include <editor/ResourceEditor.h>
#include <editor/MirrorInspector.h>
#include <editor/InputInspector.h>

#include <mirror/mirror.h>

namespace yae {

class ShaderProgram;
class GameApplication;

namespace editor {

class EditorApplication : public yae::Application
{
public:
	EditorApplication();
	EditorApplication(const char* _name, u32 _width, u32 _height);
	virtual ~EditorApplication();

	void reload();

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

	GameApplication* m_gameApplication = nullptr;

//private:
	virtual void _onStart() override;
	virtual void _onStop() override;
	virtual void _onUpdate(float _dt) override;
	virtual bool _onSerialize(Serializer* _serializer) override;

	void _displayTypeTreeNode(mirror::Type* _type, mirror::Type* _parent = nullptr);
	void _openGameApplication();
	void _closeGameApplication();
};

} // namespace editor
} // namespace yae