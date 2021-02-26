#pragma once

#include "Resource.h"

namespace yae {

class FileResource : public Resource
{
public:
	FileResource(const char* _path);

	virtual Error onLoaded(std::string& _outErrorDescription) override;
	virtual void onUnloaded() override;

	const void* getContent() const { return m_content; }
	size_t getContentSize() const { return m_contentSize; }

private:
	void* m_content = nullptr;
	size_t m_contentSize = 0;
};

}
