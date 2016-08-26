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

    struct fuse_impl : public gie::fuse_i {
        fuse_impl(const fuse_impl&) = delete;
        fuse_impl& operator=(const fuse_impl&) = delete;

        fuse_impl(fuse_impl&& other) = default;

        explicit fuse_impl(int dummy){
            GIE_DEBUG_TRACE1(dummy);
            m_mega_api->setLogLevel(mega::MegaApi::LOG_LEVEL_INFO);

            auto listener = std::make_unique<mega_listener_t>();
            auto f = listener->future();
            m_mega_api->login("user", "password", listener.release());
            GIE_DEBUG_LOG("waiting");
            f.get();
            GIE_DEBUG_LOG("done");
        }



    private:
        std::unique_ptr<mega::MegaApi> m_mega_api = std::make_unique<mega::MegaApi>("BhU0CKAT", (const char*)NULL, "MEGA/SDK FUSE filesystem");


    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
