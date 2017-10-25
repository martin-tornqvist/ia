#include "rl_utils.hpp"

#include <cassert>

namespace do_not_call
{

void assert_impl(const bool check,
                 const char* check_str,
                 const char* file,
                 const int line,
                 const char* func)
{
    if (!check)
    {
        std::cerr << std::endl
                  << file << ", "
                  << line << ", "
                  << func << "():"
                  << std::endl
                  << std::endl << "*** ASSERTION FAILED! ***"
                  << std::endl
                  << std::endl << "Check that failed:"
                  << std::endl << "\"" << check_str << "\""
                  << std::endl
                  << std::endl;

        assert(false);
    }
}

} // do_not_call
