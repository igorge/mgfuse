//================================================================================================================================================
// FILE: mega_node_cache.h
// (c) GIE 2017-05-12  19:36
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_NODE_CACHE_2017_05_12_19_36
#define H_GUARD_MEGA_NODE_CACHE_2017_05_12_19_36
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "exceptions.hpp"
#include "mega_iterator.hpp"

#include "megaapi.h"

#include <boost/range/algorithm.hpp>

#include <memory>
//================================================================================================================================================
namespace gie {

    template<class SelfT>
    struct mega_node_cache_t {

        friend SelfT;

        using shared_mega_node_t = std::shared_ptr<mega::MegaNode>;

    private:
        using path_type = boost::filesystem::path;

        mega_node_cache_t(mega_node_cache_t const&) = delete;
        mega_node_cache_t& operator=(mega_node_cache_t const&) = delete;

        mega_node_cache_t(){}

        decltype(auto) self() {
            return (*static_cast<SelfT *>(this));
        }

        decltype(auto) mega(){
            return self().mega();
        }


        shared_mega_node_t m_root;

        void clear(){

            self().synchronized([&]{
                m_root.reset();
            });

        }

        auto authorize(mega::MegaNode &node) -> std::unique_ptr<mega::MegaNode> {

            std::unique_ptr<mega::MegaNode> authorized_node{ self()->mega().authorizeNode(&node) };
            GIE_CHECK(authorized_node); // failed to authorize
            return authorized_node;

        }


    public:

        template <typename NodeT>
        auto get_children(NodeT& node){

            GIE_CHECK(node.isFolder());

            std::shared_ptr<mega::MegaNodeList> children { mega().getChildren(&node) };

            GIE_CHECK(children);

            return children;
        }

        template <typename NodeT>
        auto get_child_by_name(NodeT& node, std::string const& name) {
            GIE_CHECK(node.isFolder());

            return shared_mega_node_t{ mega().getChildNode(&node, name.c_str()) };
        }


    private:

        auto impl_get_node(shared_mega_node_t const& parent, path_type const p) -> shared_mega_node_t {
            auto const file_name = p.filename().string();   

            if (parent) {

                GIE_CHECK_EX(parent->isFolder(), exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

//                GIE_DEBUG_LOG("Mega node query: "<< parent->getName() << "/" << file_name << ", parent access: " << mega().getAccess(parent.get()) );

                auto authorized_node = get_child_by_name(*parent, file_name);
                GIE_CHECK_EX(authorized_node, exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

                GIE_CHECK( !authorized_node->getChildren() );

                return authorized_node;

            } else {
                assert(file_name == "/");

                return self().synchronized([&]{

                    if(!m_root){

                        m_root.reset( self().mega().getRootNode() );

                        GIE_CHECK(m_root);
                    }

                    return m_root;
                });

            }
        }


    public:

        auto get_node(path_type const& p) -> shared_mega_node_t {

            self().assert_cookie_is_valid();

            shared_mega_node_t current_node = nullptr;

            auto i = p.begin();
            auto const& end = p.end();

            while( i !=end ){
                current_node = impl_get_node(current_node, *i);
                if( !current_node ) break; // no need to search any further if component not found
                ++i;
            }

            return current_node;
        }

        auto& nodes() {
            return *this;
        }


    };
}
//================================================================================================================================================
#endif
//================================================================================================================================================
