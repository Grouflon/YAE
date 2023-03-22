#include "string.h"

#include <yae/math.h>
#include <yae/string.h>

namespace yae {
namespace string {

size_t safeCopyToBuffer(char* _destination, const char* _source, size_t _destinationSize)
{
	size_t sourceLength = strlen(_source);
	size_t copiedLength = math::min(sourceLength, _destinationSize - 1);
	strncpy(_destination, _source, copiedLength);
	_destination[copiedLength] = 0;
	return copiedLength;
}

} // namespace string
} // namespace yae
