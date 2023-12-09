#pragma once

#include <yae/types.h>

#include <core/FunctionPointer.h>
#include <core/containers/HashMap.h>

namespace yae {

enum class FileChangeType : u8
{
	ADDED,
	REMOVED,
	MODIFIED,
	RENAMED_OLD,
	RENAMED_NEW
};

typedef void(*FileWatchFunction)(const char*, FileChangeType, void*);

class YAE_API FileWatchSystem
{
public:
	void init();
	void shutdown();

	void startFileWatcher(const char* _filePath, FileWatchFunction _onFileChangedFunction, void* _userData = nullptr);
	void stopFileWatcher(const char* _filePath);

	void pauseAllWatchers();
	void resumeAllWatchers();

//private:
	class FileWatcher
	{
	public:
		String256 filePath;
		FunctionPointer<void, const char*, FileChangeType, void*> fileChangedFunction = nullptr;
		void* userData = nullptr;
		void* fileWatch = nullptr;
	};

	void _startFileWatch(FileWatcher* _fileWatcher);
	void _stopFileWatch(FileWatcher* _fileWatcher);

	HashMap<StringHash, FileWatcher*> m_fileWatchers;

};

} // namespace