//================================================================================================================================================
// FILE: mega_fuse.h
// (c) GIE 2016-08-26  03:07
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_FUSE_2016_08_26_03_07
#define H_GUARD_MEGA_FUSE_2016_08_26_03_07
//================================================================================================================================================
#include "mega_future.hpp"
#include "gie_fuse.hpp"

#include "megaapi.h"

#include <memory>
//================================================================================================================================================
namespace gie {

    struct mega_fuse_impl : public gie::fuse_i {


        enum supported_ops {
            HAS_GETATTR=false,
            HAS_FGETATTR=false,
            HAS_OPEN=false,
            HAS_RELEASE=false,
            HAS_OPENDIR=false,
            HAS_RELEASEDIR=false,
            HAS_READDIR=false
        };


        using file_handle_type = int;
        using directory_handle_type = int;

        mega_fuse_impl(const mega_fuse_impl&) = delete;
        mega_fuse_impl& operator=(const mega_fuse_impl&) = delete;

        mega_fuse_impl(mega_fuse_impl&& other){
            m_mega_api.swap(other.m_mega_api);
        }

        explicit mega_fuse_impl(std::string const& login, std::string const& password){
            m_mega_api->setLogLevel(mega::MegaApi::LOG_LEVEL_INFO);

            auto listener = make_listener([](auto&&, auto&&){ return true;});
            auto f = listener->future();
            m_mega_api->login(login.c_str(), password.c_str(), listener.release());
            GIE_DEBUG_LOG("waiting for login");
            f.get();
            GIE_DEBUG_LOG("login successful");
        }



    private:
        std::unique_ptr<mega::MegaApi> m_mega_api = std::make_unique<mega::MegaApi>("BhU0CKAT", (const char*)NULL, "MEGA/SDK FUSE filesystem");


    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
