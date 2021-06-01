#pragma once

#if !defined(FREAK_STATIC) && !defined(FREAK_EXPORTS) && !defined(FREAK_IMPORTS)
#define FREAK_STATIC
#endif

#if defined FREAK_STATIC
#define FREAK_API
#elif defined FREAK_EXPORTS
#define FREAK_API __declspec(dllexport)
#elif defined FREAK_IMPORTS
#defined FREAK_API __declspec(dllimport)
#endif
