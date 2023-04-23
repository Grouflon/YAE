#include "serialization_test.h"

#include <yae/serialization/JsonSerializer.h>
#include <yae/serialization/BinarySerializer.h>
#include <yae/containers/Array.h>
#include <test/test_macros.h>

using namespace yae;

namespace serialization_test {

struct Nest
{
	bool a;
	bool b;
	float c;

    bool operator==(const Nest& _rhs) const
    {
        return
            a == _rhs.a &&
            b == _rhs.b &&
            c == _rhs.c
        ;
    }    
};
struct Sfouf
{
    u32 a;
    float b;
    yae::Array<yae::Array<u8>> array;
    bool c;
    Nest nest;
    String str;

    bool operator==(const Sfouf& _rhs) const
    {
    	return
    		a == _rhs.a &&
    		b == _rhs.b &&
    		array == _rhs.array &&
    		c == _rhs.c &&
            nest == _rhs.nest &&
            str == _rhs.str;
    	;
    }
};

void serializeSfouf(yae::Serializer& _serializer, Sfouf& _sfouf)
{
    TEST(_serializer.beginSerializeObject());
    {
    	if (_serializer.getMode() == SerializationMode::WRITE)
    	{
        	TEST(_serializer.serialize(_sfouf.a, "a"));
	        TEST(_serializer.serialize(_sfouf.b, "b"));
            TEST(_serializer.serialize(_sfouf.c, "c"));
        	TEST(_serializer.serialize(_sfouf.str, "str"));
    	}

        u32 arraySize = _sfouf.array.size();
        TEST(_serializer.beginSerializeArray(arraySize, "array"));
        {
	        _sfouf.array.resize(arraySize);
	        for (auto& array : _sfouf.array)
	        {
	            u32 arraySize2 = array.size();
	            TEST(_serializer.beginSerializeArray(arraySize2));
	            {
	            	array.resize(arraySize2);
		            for (auto& element : array)
		            {
		                TEST(_serializer.serialize(element));
		            }
	            }
	            TEST(_serializer.endSerializeArray());
	        }
        }
        TEST(_serializer.endSerializeArray());

    	TEST(_serializer.beginSerializeObject("nest"));
    	{
        	TEST(_serializer.serialize(_sfouf.nest.a, "a"));
        	TEST(_serializer.serialize(_sfouf.nest.b, "b"));
        	TEST(_serializer.serialize(_sfouf.nest.c, "c"));
    	}
    	TEST(_serializer.endSerializeObject());


        if (_serializer.getMode() == SerializationMode::READ)
    	{
        	TEST(_serializer.serialize(_sfouf.c, "c"));
        	TEST(_serializer.serialize(_sfouf.a, "a"));
            TEST(_serializer.serialize(_sfouf.b, "b"));
	        TEST(_serializer.serialize(_sfouf.str, "str"));
    	}
    }
    TEST(_serializer.endSerializeObject());
}

void initSfouf(Sfouf& _sfouf)
{
	_sfouf.a = 2;
    _sfouf.b = 2.f;
    _sfouf.array.resize(3);
    _sfouf.array[0].resize(3);
    _sfouf.array[1].resize(4);
    _sfouf.array[2].resize(5);
    _sfouf.array[2][0] = 5;
    _sfouf.array[2][1] = 4;
    _sfouf.array[2][2] = 3;
    _sfouf.array[2][3] = 2;
    _sfouf.array[2][4] = 1;
    _sfouf.c = true;
    _sfouf.nest.a = true;
    _sfouf.nest.b = false;
    _sfouf.nest.c = 7.2f;
    _sfouf.str = "bonjour";
}

void testJsonSerializer()
{
	Sfouf sfoufWrite;
    initSfouf(sfoufWrite);
    Sfouf sfoufRead;
    Allocator& allocator = toolAllocator();

    JsonSerializer serializer(&allocator);

	serializer.beginWrite();
    serializeSfouf(serializer, sfoufWrite);
    serializer.endWrite();

    size_t dataSize = serializer.getWriteDataSize();
    void* data = allocator.allocate(dataSize);
    memcpy(data, serializer.getWriteData(), dataSize);

    YAE_VERIFY(serializer.parseSourceData(data, dataSize));
    serializer.beginRead();
    serializeSfouf(serializer, sfoufRead);
    serializer.endRead();

    auto printSfouf = [](const Sfouf& _s)
    {
        YAE_LOGF("a: %d", _s.a);
        YAE_LOGF("b: %f", _s.b);
        YAE_LOGF("array: %d", _s.array.size());
        for (auto& child : _s.array)
        {
            YAE_LOGF("\tarray: %d", child.size());
            for (u8 a : child)
            {
                YAE_LOGF("\t\t%d", a);
            }
        }
        YAE_LOGF("c: %d", _s.c);
        YAE_LOGF("nest.a: %d", _s.nest.a);
        YAE_LOGF("nest.b: %d", _s.nest.b);
        YAE_LOGF("nest.c: %f", _s.nest.c);
        YAE_LOGF("str: %s", _s.str.c_str());
        YAE_LOG("");
    };

    //printSfouf(sfoufWrite);
    //printSfouf(sfoufRead);

    TEST(sfoufWrite == sfoufRead);

    allocator.deallocate(data);
}

void testBinarySerializer()
{
	Sfouf sfoufWrite;
    initSfouf(sfoufWrite);
    Sfouf sfoufRead;
    Allocator& allocator = toolAllocator();

    yae::BinarySerializer serializer(&allocator);

    serializer.beginWrite();
    serializeSfouf(serializer, sfoufWrite);
    serializer.endWrite();

    size_t dataSize = serializer.getWriteDataSize();
    void* data = allocator.allocate(dataSize);
    memcpy(data, serializer.getWriteData(), dataSize);

    serializer.setReadData(data, dataSize);
    serializer.beginRead();
    serializeSfouf(serializer, sfoufRead);
    serializer.endRead();

    TEST(sfoufWrite == sfoufRead);

    allocator.deallocate(data);	
}

} // namespace serialization_test
