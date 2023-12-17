#pragma once

#include <yae/types.h>

#include <core/containers/Array.h>
#include <core/containers/HashMap.h>

struct ImGuiInputTextCallbackData;

namespace yae {

typedef void (*ConsoleCommand)(u32, const char**);

class YAE_API Console
{
public:
	void init();
	void shutdown();

	void execute(const char* _command);
	void clearLog();

	void registerCommand(const char* _name, ConsoleCommand _callback);
	void unregisterCommand(const char* _name);

	void drawConsole();

	bool serializeSettings(Serializer& _serializer);

//private:
	void _onLog(const char* _categoryName, LogVerbosity _verbosity, const char* _fileInfo, const char* _message);

	int _consoleInputTextCallback(ImGuiInputTextCallbackData* _data);

	struct Command
	{
		String128 name;
		ConsoleCommand callback;
	};
	Array<Command> m_commands;
	HashMap<StringHash, u32> m_nameToCommand;

	String256 m_inputField;
	i32 m_commandHistoryPosition = -1;
	bool m_scrollToBottom = false;

	Array<String128> m_commandHistory;
	u32 m_maxCommandHistorySize = 10;

	enum class ConsoleMode : u8
	{
		CLOSED = 0,
		INPUT,
		INPUT_AND_LOG
	};
	ConsoleMode m_consoleMode = ConsoleMode::CLOSED;

	struct LogEntry
	{
		String64 category;
		String64 fileInfo;
		String128 message;
		LogVerbosity verbosity;
	};
	Array<LogEntry> m_log;
};

} // namespace yae
