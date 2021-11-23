#pragma once

#include <yae/types.h>
#include <yae/resource.h>

#include <mirror/mirror.h>

namespace yae {

class YAELIB_API MeshResource : public Resource
{
	MIRROR_CLASS(MeshResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	MeshResource(const char* _path);
	virtual ~MeshResource();

// private:
	virtual Error _doLoad(String& _outErrorDescription) override;
	virtual void _doUnload() override;
};

} // namespace yae
