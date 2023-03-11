#include "random.h"

#include <yae/RandomGenerator.h>
#include <yae/random.h>

#include <test/test_macros.h>

using namespace yae;

namespace random {

void testRandom()
{
	yae::RandomGenerator generator;

	yae::random::setSeed(generator, 0);
	TEST(yae::random::range01(generator) == 0.548813521862030029296875f);
	TEST(yae::random::range01(generator) == 0.59284460544586181640625f);
	TEST(yae::random::range01(generator) == 0.71518933773040771484375f);
	TEST(yae::random::range01(generator) == 0.844265758991241455078125f);
	TEST(yae::random::range01(generator) == 0.602763354778289794921875f);
	TEST(yae::random::range01(generator) == 0.857945621013641357421875f);
	TEST(yae::random::range01(generator) == 0.544883191585540771484375f);
	TEST(yae::random::range01(generator) == 0.847251713275909423828125f);
	TEST(yae::random::range01(generator) == 0.4236547946929931640625f);
	TEST(yae::random::range01(generator) == 0.623563706874847412109375f);
	TEST(yae::random::range01(generator) == 0.645894110202789306640625f);
	TEST(yae::random::range01(generator) == 0.384381711483001708984375f);

	yae::random::setSeed(generator, 1);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 0);
	TEST(yae::random::range(generator, 0, 1) == 0);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 1);
	TEST(yae::random::range(generator, 0, 1) == 0);
	TEST(yae::random::range(generator, 0, 1) == 0);
}

} // namespace random
