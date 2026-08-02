#pragma once

#ifdef _DEBUG

// In debug mode, expand to printf with all arguments
#define DEBUG_PRINT(...) printf(__VA_ARGS__)

#else
// In release/production mode, expand to nothing (zero overhead)
#define DEBUG_PRINT(...) ((void)0)

#endif
