#pragma once

#include <yae/types.h>
#include <yae/resources/Resource.h>

#include <mirror/mirror.h>

namespace yae {

class YAE_API File : public Resource
{
	MIRROR_CLASS(File)
	(
		MIRROR_PARENT(Resource)
	);

public:
	File();
	virtual~ File();

	void setPath(const char* _path);
	const char* getPath() const;

	const void* getContent() const;
	size_t getContentSize() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
	void* m_content = nullptr;
	size_t m_contentSize = 0;
};

}
