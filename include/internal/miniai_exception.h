
#ifndef MINIAI_EXCEPTION_H_
#define MINIAI_EXCEPTION_H_

#ifdef MINIAI_EXCEPTIONS

// If we have exceptions, declare miniai exception class.
class miniai_exception : public std::runtime_error
{

};

#endif
#endif