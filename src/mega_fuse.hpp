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

    struct mega_fuse_impl {


        enum supported_ops {
            HAS_GETATTR=true,
            HAS_FGETATTR=false,
            HAS_OPEN=false,
            HAS_RELEASE=false,
            HAS_OPENDIR=true,
            HAS_RELEASEDIR=true,
            HAS_READDIR=false
        };


        using file_handle_type = void*;
        using directory_handle_type = void*;

        using path_type = boost::filesystem::path;

        mega_fuse_impl(const mega_fuse_impl&) = delete;
        mega_fuse_impl& operator=(const mega_fuse_impl&) = delete;

        mega_fuse_impl(mega_fuse_impl&& other){
            m_mega_api.swap(other.m_mega_api);
        }

        explicit mega_fuse_impl(std::string const& login, std::string const& password){
            m_mega_api->setLogLevel(mega::MegaApi::LOG_LEVEL_INFO);

            auto ignore_result_value = [](auto&&, auto&&){ return true;};

            {
                auto listener = make_listener(ignore_result_value);
                auto f = listener->future();
                m_mega_api->login(login.c_str(), password.c_str(), listener.release());
                GIE_DEBUG_LOG("waiting for login");
                f.get();
                GIE_DEBUG_LOG("login successful");
            }

            {
                auto listener = make_listener(ignore_result_value);
                auto f = listener->future();
                m_mega_api->fetchNodes(listener.release());
                GIE_DEBUG_LOG("waiting for fetchNodes()");
                f.get();
                GIE_DEBUG_LOG("fetchNodes() successful");
            }

        }



    private:
        std::unique_ptr<mega::MegaApi> m_mega_api = std::make_unique<mega::MegaApi>("BhU0CKAT", (const char*)NULL, "MEGA/SDK FUSE filesystem");


        auto get_node(mega::MegaNode* const parent, path_type const& fn){

            if(parent==nullptr){
                assert(fn.filename()=="/");

                std::unique_ptr<mega::MegaNode> root{ m_mega_api->getRootNode() };
                GIE_CHECK( root );

                std::unique_ptr<mega::MegaNode> authorized_root{ m_mega_api->authorizeNode(root.get()) };
                GIE_CHECK(authorized_root);

                GIE_DEBUG_LOG( authorized_root->getName() );

                GIE_DEBUG_LOG(authorized_root->getChildren()-> size());
            }

            return parent;
        }

        auto get_node(path_type const& path){

            mega::MegaNode* parent = nullptr;
            for(auto&& i : path){
                parent = get_node(parent, i);
            }
        }

    public:


        void getattr(path_type const& path, struct stat * st){

            get_node(path);
        }


        auto opendir(boost::filesystem::path const& path, fuse_file_info * fi) -> directory_handle_type{
            assert(!path.empty());
            assert(fi);



            //auto dir = std::make_unique<directory_handle>(m_root / path);

            return 0;//dir.release();
        }

        void releasedir(const char * path, fuse_file_info * fi, directory_handle_type const handle ){
            assert(path);
            assert(fi);
            assert(handle);

            //auto dir = std::unique_ptr<directory_handle>(handle);
        }



    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
