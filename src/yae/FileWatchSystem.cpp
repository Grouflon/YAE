#include "FileWatchSystem.h"

#define YAE_FILEWATCH_ENABLED (YAE_PLATFORM_WINDOWS == 1)
#if YAE_FILEWATCH_ENABLED
#include <FileWatch/FileWatch.hpp>
#endif

namespace yae {

void FileWatchSystem::init()
{

}

void FileWatchSystem::shutdown()
{
	YAE_ASSERT(m_fileWatchers.size() == 0);
}

void FileWatchSystem::startFileWatcher(const char* _filePath, FileWatchFunction _onFileChangedFunction, void* _userData)
{
	YAE_ASSERT(_filePath != nullptr);
	YAE_ASSERT(_onFileChangedFunction != nullptr);

	StringHash id(_filePath);

	YAE_ASSERT_MSGF(m_fileWatchers.get(id) == nullptr, "Several watchers for file \"%s\". Multiple watchers on the same file is not supported yet", _filePath);

	FileWatcher* fileWatcher = defaultAllocator().create<FileWatcher>();
	fileWatcher->filePath = _filePath;
	fileWatcher->fileChangedFunction = _onFileChangedFunction;
	fileWatcher->userData = _userData;
	m_fileWatchers.set(id, fileWatcher);

	_startFileWatch(fileWatcher);
}

void FileWatchSystem::stopFileWatcher(const char* _filePath)
{
	YAE_ASSERT(_filePath != nullptr);
	StringHash id(_filePath);
	FileWatcher** fileWatcherPtr = m_fileWatchers.get(id);
	YAE_ASSERT(fileWatcherPtr != nullptr);

	_stopFileWatch(*fileWatcherPtr);
	defaultAllocator().destroy(*fileWatcherPtr);

	m_fileWatchers.remove(id);
}

void FileWatchSystem::pauseAllWatchers()
{
	for (auto pair : m_fileWatchers)
	{
		_stopFileWatch(pair.value);
	}
}

void FileWatchSystem::resumeAllWatchers()
{
	for (auto pair : m_fileWatchers)
	{
		_startFileWatch(pair.value);
	}
}

void FileWatchSystem::_startFileWatch(FileWatcher* _fileWatcher)
{
	YAE_ASSERT(_fileWatcher != nullptr);

#if YAE_FILEWATCH_ENABLED
	_fileWatcher->fileWatch = defaultAllocator().create<filewatch::FileWatch<std::string>>(
		_fileWatcher->filePath.c_str(),
		[_fileWatcher](const std::string& _path, const filewatch::Event _changeType)
		{
			FileChangeType changeType;
			switch(_changeType)
			{
			case filewatch::Event::added: changeType = FileChangeType::ADDED; break;
			case filewatch::Event::removed: changeType = FileChangeType::REMOVED; break;
			case filewatch::Event::modified: changeType = FileChangeType::MODIFIED; break;
			case filewatch::Event::renamed_old: changeType = FileChangeType::RENAMED_OLD; break;
			case filewatch::Event::renamed_new: changeType = FileChangeType::RENAMED_NEW; break;
			}

			_fileWatcher->fileChangedFunction(_path.c_str(), changeType, _fileWatcher->userData);
		}
	);
#else
	YAE_ASSERT_MSG(false, "FileWatch not enabled");
#endif
}

void FileWatchSystem::_stopFileWatch(FileWatcher* _fileWatcher)
{
	YAE_ASSERT(_fileWatcher != nullptr);

#if YAE_FILEWATCH_ENABLED
	auto watcher = (filewatch::FileWatch<std::string>*)_fileWatcher->fileWatch;
	YAE_ASSERT(watcher != nullptr);
	defaultAllocator().destroy(watcher);
#else
	YAE_ASSERT_MSG(false, "FileWatch not enabled");
#endif
}

} // namespace yae
