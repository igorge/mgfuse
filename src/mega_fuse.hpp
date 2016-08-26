//================================================================================================================================================
// FILE: mega_fuse.h
// (c) GIE 2016-08-26  03:07
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_FUSE_2016_08_26_03_07
#define H_GUARD_MEGA_FUSE_2016_08_26_03_07
//================================================================================================================================================
#include "gie_fuse.hpp"

#include "mega.h"
#include "megaapi_impl.h"

#include <memory>
//================================================================================================================================================
namespace gie {

    struct fuse_impl : public gie::fuse_i, mega::MegaApp {
        fuse_impl(const fuse_impl&) = delete;
        fuse_impl& operator=(const fuse_impl&) = delete;

        fuse_impl(fuse_impl&& other) = default;
//    fuse_impl(fuse_impl&& other){
//        client = other.client;
//        other.client = nullptr;
//    }


        std::unique_ptr<mega::MegaHttpIO> m_mega_http_io = std::make_unique<mega::MegaHttpIO>();
        std::unique_ptr<mega::MegaFileSystemAccess> m_mega_fs_io = std::make_unique<mega::MegaFileSystemAccess>();
        std::unique_ptr<mega::MegaWaiter> m_mega_waiter = std::make_unique<mega::MegaWaiter>();
        std::unique_ptr<mega::MegaGfxProc> m_mega_gfx_proc = std::make_unique<mega::MegaGfxProc>();

        std::unique_ptr<mega::MegaClient> m_client = std::make_unique<mega::MegaClient>(
                this,
                m_mega_waiter.get(),
                m_mega_http_io.get(),
                m_mega_fs_io.get(),
                new mega::MegaDbAccess(),
                m_mega_gfx_proc.get(),
                "", "");

        explicit fuse_impl(int dummy){
            GIE_DEBUG_TRACE1(dummy);
        }

        int a = 10;


    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
