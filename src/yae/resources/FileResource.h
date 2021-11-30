#pragma once

#include <yae/types.h>
#include <yae/resource.h>

#include <mirror/mirror.h>

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

	const void* getContent() const { return m_content; }
	size_t getContentSize() const { return m_contentSize; }

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	void* m_content = nullptr;
	size_t m_contentSize = 0;
};

template <>
struct ResourceIDGetter<FileResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};

}
