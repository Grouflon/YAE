#pragma once

#include <resource.h>
#include <filesystem.h>

#include <mirror.h>

namespace yae {

class YAELIB_API FileResource : public Resource
{
	MIRROR_CLASS(FileResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	FileResource(const char* _path);
	virtual~ FileResource();

	virtual Error onLoaded(String& _outErrorDescription) override;
	virtual void onUnloaded() override;

	const void* getContent() const { return m_content; }
	size_t getContentSize() const { return m_contentSize; }

private:
	void* m_content = nullptr;
	size_t m_contentSize = 0;
	Path m_path;
};

template <>
struct ResourceIDGetter<FileResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};

}
