#include "string.h"

#include <yae/math.h>
#include <yae/string.h>

#include <mirror/mirror.h>

namespace yae {
namespace string {

size_t safeCopyToBuffer(char* _destination, const char* _source, size_t _destinationSize)
{
	u32 sourceLength = strlen(_source);
	u32 copiedLength = math::min(sourceLength, u32(_destinationSize) - 1);
	strncpy(_destination, _source, copiedLength);
	_destination[copiedLength] = 0;
	return copiedLength;
}

} // namespace string
} // namespace yae


MIRROR_CLASS(yae::String)
(
);
