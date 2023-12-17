#include "Console.h"

#include <core/logger.h>
#include <core/math.h>
#include <core/Program.h>
#include <core/serialization/serialization.h>
#include <core/string.h>

#include <yae/imgui_extension.h>
#include <yae/InputSystem.h>

#include <imgui/imgui.h>
#include <mirror/mirror.h>

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
	StringHash commandHash = string::toLowerCase(_command);
	u32* commandIndexPtr = m_nameToCommand.get(commandHash);
	if (commandIndexPtr != nullptr)
	{
		YAE_LOGF_CAT("console", "> %s", m_commands[*commandIndexPtr].name.c_str());
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

	StringHash hash = string::toLowerCase(_name);
	m_nameToCommand[hash] = m_commands.size() - 1;
}

void Console::unregisterCommand(const char* _name)
{
	StringHash hash = string::toLowerCase(_name);
	YAE_ASSERT(m_nameToCommand.get(hash) != nullptr);

	u32 commandIndex = *m_nameToCommand.get(hash);
	m_commands.erase(commandIndex);
	for (u32 i = commandIndex; i < m_commands.size(); ++i)
	{
		StringHash commandHash = string::toLowerCase(m_commands[i].name);
		m_nameToCommand[commandHash] = i;
	}
}

int Console::_consoleInputTextCallback(ImGuiInputTextCallbackData* _data)
{
	switch (_data->EventFlag)
    {
    	case ImGuiInputTextFlags_CallbackHistory:
    	{
            const i32 previousHistoryPosition = m_commandHistoryPosition;
			if (_data->EventKey == ImGuiKey_UpArrow)
			{
				m_commandHistoryPosition = math::min(m_commandHistoryPosition + 1, i32(m_commandHistory.size() - 1));
			}
			else if (_data->EventKey == ImGuiKey_DownArrow)
			{
				m_commandHistoryPosition = math::max(m_commandHistoryPosition - 1, -1);
			}

			if (previousHistoryPosition != m_commandHistoryPosition)
			{
            	const char* command = (m_commandHistoryPosition >= 0) ? m_commandHistory[m_commandHistory.size() - 1 - m_commandHistoryPosition].c_str() : "";
            	_data->DeleteChars(0, _data->BufTextLen);
				_data->InsertChars(0, command);
			}
    	}
    	break;
    }
	return 0;
}

void Console::drawConsole()
{
	bool reclaimFocus = false;

	if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
	{
		m_consoleMode = ConsoleMode((u8(m_consoleMode) + 1) % 3);
		reclaimFocus = true;
	}

	if (m_consoleMode == ConsoleMode::CLOSED)
		return;

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
	    		if (m_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                	ImGui::SetScrollHereY(1.0f);
                m_scrollToBottom = false;
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
	    	ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
	    	auto callback = [](ImGuiInputTextCallbackData* _data)
	    	{
	    		Console* console = (Console*)_data->UserData;
	    		return console->_consoleInputTextCallback(_data);
	    	};

	    	if (ImGui::InputText("Input", &m_inputField, flags, callback, this))
	    	{
	    		// Add to command history
	    		{
	    			for (i32 i = m_commandHistory.size() - 1; i >= 0; --i)
					{
						if (m_commandHistory[i] == m_inputField)
						{
							m_commandHistory.erase(i);
							break;
						}
					}
					m_commandHistory.push_back(m_inputField.c_str());

					while (m_commandHistory.size() > m_maxCommandHistorySize)
					{
						m_commandHistory.erase(0, 1);
					}
					program().saveSettings();
	    		}

	    		execute(m_inputField.c_str());
	    		m_inputField.clear();
				m_commandHistoryPosition = -1;
				reclaimFocus = true;
				m_scrollToBottom = true;
	    	}

	    	// Auto-focus on window apparition
	        ImGui::SetItemDefaultFocus();
	        if (reclaimFocus)
	            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    		ImGui::PopStyleVar(1);
    	}
    	ImGui::EndChild();
    	ImGui::PopStyleVar(1);
    	ImGui::PopStyleVar(1);
    }
    ImGui::End();
}

bool Console::serializeSettings(Serializer& _serializer)
{
	return serialization::serializeClassInstanceMembers(_serializer, *this);
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

MIRROR_CLASS(yae::Console)
(
	MIRROR_MEMBER(m_commandHistory);
	MIRROR_MEMBER(m_maxCommandHistorySize);
);
