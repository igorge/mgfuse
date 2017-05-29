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


        using fuse_op_def_opendir = fuse_method_def<fuse_op_implemented>;

        enum supported_ops {
            HAS_GETATTR=true,
            HAS_FGETATTR=false,
            HAS_OPEN=false,
            HAS_RELEASE=false,
//            HAS_OPENDIR=true,
            HAS_RELEASEDIR=true,
            HAS_READDIR=true
        };


        struct file_handle_impl_t;

        struct directory_handle_impl_t {
            std::shared_ptr<mega::MegaNodeList> children;
        };

        using file_handle_type = file_handle_impl_t*;
        using directory_handle_type = directory_handle_impl_t*;

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

        struct impl_t : mega_node_cache_t<impl_t>, gie::cookie_checker<> {

            impl_t(impl_t const&) = delete;
            impl_t& operator=(impl_t const&) = delete;

            impl_t(){}

            boost::mutex m_mega_lock;
            std::unique_ptr<mega::MegaApi> m_mega_api = std::make_unique<mega::MegaApi>("BhU0CKAT", (const char*) nullptr, "MEGA/SDK FUSE filesystem");

            auto mega() -> mega::MegaApi& {
                this->is_cookie_valid();
                assert(m_mega_api);
                return *m_mega_api;
            }

            ~impl_t(){
                this->clear();
            }
        };

        using shared_mega_node_t = impl_t::shared_mega_node_t;

        std::unique_ptr<impl_t> m_impl = std::make_unique<impl_t>();

        auto impl() -> impl_t& {
            assert(m_impl);
            return *m_impl;
        }

        auto mega() -> mega::MegaApi& {
            return impl().mega();
        }

        decltype(auto) get_node(path_type const& path){
            return impl().nodes().get_node(path);
        }

        auto get_children(shared_mega_node_t const& node){
            return impl().get_children(node);
        }

    public:


        void getattr(path_type const& path, struct stat * st){
            assert(st);

            mutex::scoped_lock lock {impl().m_mega_lock};

            auto const& node = get_node(path);

            if(!node) GIE_THROW(exception::fuse_no_such_file_or_directory());

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

            auto children = get_children(get_node(path));
            GIE_CHECK(children);

            auto handle = std::unique_ptr<directory_handle_impl_t>(new directory_handle_impl_t{children});

            return handle.release();
        }

        void releasedir(const char * path, fuse_file_info * fi, directory_handle_type const handle ){
            assert(path);
            assert(fi);
            assert(handle);

            mutex::scoped_lock lock {impl().m_mega_lock};

            auto dir = std::unique_ptr<directory_handle_impl_t>(handle);
        }

        void readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi, directory_handle_type const handle) {

            static_assert(std::numeric_limits<off_t>::max() >= std::numeric_limits<int>::max() );  // off_t can hold all values of mega::MegaNodeList::get index (int)

            assert(path);
            assert(fi);
            assert(handle);

            auto const num_of_entries = handle->children->size();

            GIE_CHECK(offset>=0);
            GIE_CHECK(offset <= num_of_entries);



            bool filled_up = false;
            while (!filled_up && offset != num_of_entries) {

                auto const node = handle->children->get(offset);
                GIE_CHECK(node);

                auto const filename = node->getName();
                GIE_CHECK(filename);
                GIE_CHECK(*filename != 0); //must be non empty

                if (auto const r = filler(buf, filename, nullptr, ++offset); r == 1){
                    filled_up = true;
                } else if (r == 0) {
                    // do nothing
                } else {
                    GIE_UNEXPECTED();
                }

            }

        }



    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
