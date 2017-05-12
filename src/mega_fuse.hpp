//================================================================================================================================================
// FILE: mega_fuse.h
// (c) GIE 2016-08-26  03:07
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_FUSE_2016_08_26_03_07
#define H_GUARD_MEGA_FUSE_2016_08_26_03_07
//================================================================================================================================================
#include "mega_node_cache.hpp"
#include "mega_iterator.hpp"
#include "mega_future.hpp"
#include "gie_fuse.hpp"

#include "megaapi.h"

#include <boost/range/algorithm.hpp>

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
        using mutex = boost::mutex;

        using stat_t = struct stat;

        mega_fuse_impl(const mega_fuse_impl&) = delete;
        mega_fuse_impl& operator=(const mega_fuse_impl&) = delete;

        mega_fuse_impl(mega_fuse_impl&& other){
            m_impl.swap(other.m_impl);
        }

        explicit mega_fuse_impl(std::string const& login, std::string const& password){
            m_impl->m_mega_api->setLogLevel(mega::MegaApi::LOG_LEVEL_INFO);

            auto const ignore_result_value = [](auto&&, auto&&){ return true;};

            {
                auto listener = make_listener(ignore_result_value);
                auto f = listener->future();
                mega().login(login.c_str(), password.c_str(), listener.release());
                GIE_DEBUG_LOG("waiting for login");
                f.get();
                GIE_DEBUG_LOG("login successful");
            }

            {
                auto listener = make_listener(ignore_result_value);
                auto f = listener->future();
                mega().fetchNodes(listener.release());
                GIE_DEBUG_LOG("waiting for fetchNodes()");
                f.get();
                GIE_DEBUG_LOG("fetchNodes() successful");
            }

        }



    private:

        struct impl_t : mega_node_cache_t<impl_t> {
            boost::mutex m_mega_lock;
            std::unique_ptr<mega::MegaApi> m_mega_api = std::make_unique<mega::MegaApi>("BhU0CKAT", (const char*) nullptr, "MEGA/SDK FUSE filesystem");

            auto mega() -> mega::MegaApi& {
                assert(m_mega_api);
                return *m_mega_api;
            }
        };


        std::unique_ptr<impl_t> m_impl = std::make_unique<impl_t>();

        auto impl() -> impl_t& {
            assert(m_impl);
            return *m_impl;
        }

        auto mega() -> mega::MegaApi& {
            return impl().mega();
        }


        auto authorize(std::unique_ptr<mega::MegaNode>& node) -> std::unique_ptr<mega::MegaNode>& {
            assert(node);

            std::unique_ptr<mega::MegaNode> authorized_node{ mega().authorizeNode(node.get()) };
            GIE_CHECK(authorized_node.get());

            node.swap(authorized_node);
            return node;
        }

        auto get_node(std::unique_ptr<mega::MegaNode> const& parent, path_type const& fn) -> std::unique_ptr<mega::MegaNode> {

            auto const file_name = fn.filename();

            if(parent){

                auto const children = to_range(parent->getChildren());
                auto const found_node = boost::find_if(children, [&](mega::MegaNode* node){
                    return file_name==node->getName();
                });

                if(found_node==children.end()){
                    GIE_THROW(exception::fuse_no_such_file_or_directory());
                } else {
                    return std::unique_ptr<mega::MegaNode>{ (*found_node)->copy()};
                }

            } else {
                assert(file_name=="/");

                std::unique_ptr<mega::MegaNode> root{ mega().getRootNode() };
                GIE_CHECK( root );

                authorize(root);

                return root;
            }

        }

        auto get_node(path_type const& path){

            impl().nodes().get_node(path);

            std::unique_ptr<mega::MegaNode> current_node{};

            for(auto&& i : path){
                current_node = get_node(current_node, i);
            }

            return current_node;
        }

    public:


        void getattr(path_type const& path, struct stat * st){
            assert(st);

            mutex::scoped_lock lock {impl().m_mega_lock};

            auto noded = get_node("/test333/test2");

            auto node = get_node(path);
            if(!node) GIE_THROW(exception::fuse_no_such_file_or_directory());

            for(auto i:to_range(node->getChildren())){
                GIE_DEBUG_LOG(i->getName());
            }

            *st = stat_t();

            switch (auto const nodeType = node->getType(); nodeType){
                case mega::MegaNode::TYPE_FILE:
                    st->st_mode = S_IFREG;
                    break;
                case mega::MegaNode::TYPE_ROOT:
                case mega::MegaNode::TYPE_FOLDER:
                    st->st_mode = S_IFDIR;
                    break;
                default:
                    GIE_UNEXPECTED_EX(gie::exception::error_str_einfo("Unknown mega node type.") << gie::exception::error_int_einfo(nodeType));
            }
        }


        auto opendir(boost::filesystem::path const& path, fuse_file_info * fi) -> directory_handle_type{

            assert(!path.empty());
            assert(fi);

            mutex::scoped_lock lock {impl().m_mega_lock};



            //auto dir = std::make_unique<directory_handle>(m_root / path);
            GIE_UNIMPLEMENTED();

            return 0;//dir.release();
        }

        void releasedir(const char * path, fuse_file_info * fi, directory_handle_type const handle ){
            assert(path);
            assert(fi);
            assert(handle);

            mutex::scoped_lock lock {impl().m_mega_lock};

            GIE_UNIMPLEMENTED();
            //auto dir = std::unique_ptr<directory_handle>(handle);
        }



    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
