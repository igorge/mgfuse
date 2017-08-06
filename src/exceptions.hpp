//================================================================================================================================================
// FILE: exceptions.h
// (c) GIE 2017-05-25  20:34
//
//================================================================================================================================================
#ifndef H_GUARD_EXCEPTIONS_2017_05_25_20_34
#define H_GUARD_EXCEPTIONS_2017_05_25_20_34
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "gie/exceptions.hpp"

#include <errno.h>
//================================================================================================================================================
namespace gie {

    namespace exception {

        typedef boost::error_info< struct tag_error_fuse_einfo, int> fuse_errorno_einfo;

        struct fuse_exception : virtual gie::exception::root {};

        struct fuse_errorno_exception : virtual fuse_exception {
            explicit fuse_errorno_exception(error_t const err) : m_errno(err) {}
            fuse_errorno_exception(fuse_errorno_exception&&) = default;
            fuse_errorno_exception(fuse_errorno_exception const& other) = default;

            error_t get_errno()const{
                return m_errno;
            }

        private:
            error_t m_errno;
        };

        struct fuse_no_such_file_or_directory : fuse_errorno_exception {
            fuse_no_such_file_or_directory() : fuse_errorno_exception(ENOENT) {}
        };

        struct fuse_not_a_directory : fuse_errorno_exception {
            fuse_not_a_directory () : fuse_errorno_exception(ENOTDIR) {}
        };

        struct fuse_exists : fuse_errorno_exception {
            fuse_exists() : fuse_errorno_exception(EEXIST) {}
        };
    }

}
//================================================================================================================================================
#endif
//================================================================================================================================================
