//================================================================================================================================================
// FILE: mega_exception.h
// (c) GIE 2016-08-26  18:16
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_EXCEPTION_2016_08_26_18_16
#define H_GUARD_MEGA_EXCEPTION_2016_08_26_18_16
//================================================================================================================================================
#include "gie/exceptions.hpp"
#include "gie/debug.hpp"

#include "megaapi.h"

#include <iostream>
//================================================================================================================================================
#define GIE_MEGA_CHECK(x) do{ if( (x).getErrorCode()!=::mega::MegaError::API_OK) { ::gie::debug::brk(); GIE_THROW(::gie::exception::mega() << ::gie::exception::mega_error_einfo(x));} } while(false)

namespace mega {
    std::ostream& operator<<(std::ostream& os, MegaError const& error)
    {
        os << "MegaError("<< error.getErrorCode()<<"): "<<error.getErrorString();

        return os;
    }
}


namespace gie {

    namespace exception {
        struct mega : virtual boost::exception, virtual std::exception {};

        typedef boost::error_info<struct tag_mega_error_einfo, ::mega::MegaError> mega_error_einfo;
    }


}
//================================================================================================================================================
#endif
//================================================================================================================================================
