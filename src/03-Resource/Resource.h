#pragma once

#include <string>

#include <00-Type/IntTypes.h>

#include "ResourceID.h"

namespace yae {

class Resource
{
public:
	enum Error
	{
		ERROR_NONE,
		ERROR_LOAD
	};

	Resource(const char* _name);
	~Resource();
	
	ResourceID getID() const { return m_id; }
	const char* getName() const { return m_name.c_str(); }

	void use();
	bool load();
	bool useLoad();
	void unuse();
	void release();
	void releaseUnuse();

	bool isLoaded() const { return m_loadCount > 0; }
	bool isUsed() const { return m_useCount > 0; }

	Error getError() const { return m_error; }
	const char* getErrorDescription() const { return m_errorDescription.c_str(); }

protected:
	virtual Error onLoaded(std::string& _outErrorDescription) { return ERROR_NONE; }
	virtual void onUnloaded() {}

private:
	ResourceID m_id;
	std::string m_name;
	u32 m_useCount = 0;
	u32 m_loadCount = 0;

	Error m_error = ERROR_NONE;
	std::string m_errorDescription;
};

}