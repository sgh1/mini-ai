
#ifndef MINIAI_ERROR_H_
#define MINIAI_ERROR_H_

namespace miniai
{    

#ifdef MINIAI_EXEPTIONS

#include "miniai_exception.h"

#include <exception>

// Throw miniai exception if we can handle it.
class miniai_error
{
    static void error(const char* err)
    {
         throw miniai_exception(err);
    }
};

#else

#include <cstdio>

// If we don't have exceptions, print message and terminate.
class miniai_error
{
    static void error(const char* err)
    {
        // Print message.
        printf("*** miniai error occured: %s ***\n",err);

        // Terminate.
        std::terminate();
    }
};

#endif

}

#endif