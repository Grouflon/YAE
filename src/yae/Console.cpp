#include "Console.h"

#include <core/logger.h>
#include <core/math.h>

#include <yae/imgui_extension.h>
#include <yae/InputSystem.h>

#include <imgui/imgui.h>

namespace yae {

void Console::init()
{
	YAE_VERBOSEF_CAT("console", "Initializing console...");

	logger().logged.bind(this, &Console::_onLog);

	registerCommand("clear", [](u32, const char**) { console().clearLog(); });

	YAE_VERBOSEF_CAT("console", "Console initialized");
}

void Console::shutdown()
{
	YAE_VERBOSEF_CAT("console", "Shutting down console...");

	unregisterCommand("clear");

	logger().logged.unbind(this, &Console::_onLog);

	YAE_VERBOSEF_CAT("console", "Console shut down");
}

void Console::execute(const char* _command)
{
	m_commandHistory.push_back(_command);

	u32* commandIndexPtr = m_nameToCommand.get(_command);
	if (commandIndexPtr != nullptr)
	{
		YAE_LOGF_CAT("console", "> %s", _command);
		m_commands[*commandIndexPtr].callback(0, nullptr);
	}
	else
	{
		YAE_WARNINGF_CAT("console", "Unknown command: %s", _command);
	}
}

void Console::clearLog()
{
	m_log.clear();
}

void Console::registerCommand(const char* _name, ConsoleCommand _callback)
{
	YAE_ASSERT(m_nameToCommand.get(_name) == nullptr);

	Command command;
	command.name = _name;
	command.callback = _callback;

	m_commands.push_back(command);
	m_nameToCommand[command.name.c_str()] = m_commands.size() - 1;
}

void Console::unregisterCommand(const char* _name)
{
	YAE_ASSERT(m_nameToCommand.get(_name) != nullptr);

	u32 commandIndex = *m_nameToCommand.get(_name);
	m_commands.erase(commandIndex);
	for (u32 i = commandIndex; i < m_commands.size(); ++i)
	{
		m_nameToCommand[m_commands[i].name.c_str()] = i;
	}
}

static int ConsoleInputTextCallback(ImGuiInputTextCallbackData* _data)
{
	return 0;
}

void Console::drawConsole()
{
	if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
	{
		m_consoleMode = ConsoleMode((u8(m_consoleMode) + 1) % 3);
	}

	if (m_consoleMode == ConsoleMode::CLOSED)
		return;

	// NOTE(12/17/2023): This doest work. We need to use imgui callbacks for implementing history
	// bool commandHistoryBrowsed = false;
	// if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
	// {
	// 	m_commandHistoryPosition = math::min(m_commandHistoryPosition + 1, i32(m_commandHistory.size() - 1));
	// 	commandHistoryBrowsed = true;
	// }
	// if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
	// {
	// 	m_commandHistoryPosition = math::min(m_commandHistoryPosition - 1, -1);
	// 	commandHistoryBrowsed = true;
	// }
	// if (commandHistoryBrowsed)
	// {
	// 	if (m_commandHistoryPosition >= 0)
	// 	{
	// 		m_inputField = m_commandHistory[m_commandHistory.size() - 1 - m_commandHistoryPosition];
	// 	}
	// 	else
	// 	{
	// 		m_inputField.clear();
	// 	}
	// }

	ImGuiIO& io = ImGui::GetIO();

    float inputHeight = 35.f;
    float logHeight = 400.f;
    float windowHeight = inputHeight;
    if (m_consoleMode == ConsoleMode::INPUT_AND_LOG)
    {
    	windowHeight += logHeight;
    }

	ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - windowHeight));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, windowHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::SetNextWindowBgAlpha(.8f);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    bool is_open = ImGui::Begin("Console", NULL, window_flags);
    ImGui::PopStyleVar(5);
    if (is_open)
    {
    	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

    	ImGuiChildFlags childFlags = ImGuiChildFlags_AlwaysUseWindowPadding;

    	if (m_consoleMode == ConsoleMode::INPUT_AND_LOG)
    	{
	    	// Log
    		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));
	    	if (ImGui::BeginChild("Log", ImVec2(0.f, logHeight), childFlags))
	    	{
	    		for (const LogEntry& entry : m_log)
	    		{
	    			ImColor color = IM_COL32_WHITE;
	    			switch (entry.verbosity)
	    			{
	    				case LogVerbosity::ERROR: color = IM_COL32(230,0,0,255); break;
	    				case LogVerbosity::WARNING: color = IM_COL32(255,255,0,255); break;
	    				case LogVerbosity::VERBOSE: color = IM_COL32(125,125,125,255); break;
	    			}

	    			ImGui::TextColored(color, "%s", entry.message.c_str());
	    		}
	    	}
	    	ImGui::EndChild();
    		ImGui::PopStyleVar(1);
    	}

    	// Input
		ImGui::SetNextWindowBgAlpha(.4f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7,7));
    	if (ImGui::BeginChild("Input", ImVec2(0.f, inputHeight), childFlags))
    	{
    		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7,7));
    		ImGui::AlignTextToFramePadding();
	    	ImGui::Text(">");
	    	ImGui::SameLine();
	    	ImGui::SetNextItemWidth(-FLT_MIN);
	    	ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
	    	ImGui::SetKeyboardFocusHere();
	    	if (ImGui::InputText("Input", &m_inputField, flags, &ConsoleInputTextCallback, this))
	    	{
	    		execute(m_inputField.c_str());
	    		m_inputField.clear();
				m_commandHistoryPosition = -1;
	    	}
    		ImGui::PopStyleVar(1);
    	}
    	ImGui::EndChild();
    	ImGui::PopStyleVar(1);
    	ImGui::PopStyleVar(1);
    }
    ImGui::End();
}

void Console::_onLog(const char* _categoryName, LogVerbosity _verbosity, const char* _fileInfo, const char* _message)
{
	if (logger().getCategoryVerbosity(_categoryName) < _verbosity)
		return;

	Console::LogEntry entry;
	entry.category = _categoryName;
	entry.fileInfo = _fileInfo;
	entry.message = _message;
	entry.verbosity = _verbosity;

	m_log.push_back(entry);
}

} // namespace yae
