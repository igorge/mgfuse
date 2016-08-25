//================================================================================================================================================
// FILE: gie_fuse.h
// (c) GIE 2016-08-25  17:53
//
//================================================================================================================================================
#ifndef H_GUARD_GIE_FUSE_2016_08_25_17_53
#define H_GUARD_GIE_FUSE_2016_08_25_17_53
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "gie/debug/magic_cookie.hpp"
#include "gie/exceptions.hpp"
#include "gie/debug.hpp"

#include <fuse.h>

#include <utility>
//================================================================================================================================================
namespace gie {




    struct fuse_i : gie::cookie_checker<> {

        virtual int open (const char *, struct fuse_file_info *){
            GIE_DEBUG_LOG("UNIMPLEMENTED");
            return  -EOPNOTSUPP;
        }



    };


    extern fuse_operations fuse_ops;


    template <class FuseImplementation>
    struct fuse_api_mapper_t: fuse_i {

        fuse_api_mapper_t(const fuse_api_mapper_t&) = delete;
        fuse_api_mapper_t& operator=(const fuse_api_mapper_t&) = delete;

    private:
        FuseImplementation m_fuse_imp;

        //fuse_operations m_fuse_ops{};

    private:
        virtual int open(const char *, struct fuse_file_info *) override {
            GIE_DEBUG_LOG("UNIMPLEMENTED");
            return  -EOPNOTSUPP;
        }

    public:
        fuse_api_mapper_t(FuseImplementation&& fi) : m_fuse_imp(std::move(fi)) {
           // m_fuse_ops.open = impl::fuse_op_open;
        }

    };



}


//================================================================================================================================================
#endif
//================================================================================================================================================
