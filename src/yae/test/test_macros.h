#pragma once

#define TEST(_cond) do { bool __result = _cond; YAE_ASSERT(__result); if (!(__result)) { throw(#_cond); }}while(0)
