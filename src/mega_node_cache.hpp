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
            m_root.reset();
        }

        auto authorize(mega::MegaNode &node) -> std::unique_ptr<mega::MegaNode> {

            std::unique_ptr<mega::MegaNode> authorized_node{ self()->mega().authorizeNode(&node) };
            GIE_CHECK(authorized_node); // failed to authorize
            return authorized_node;

        }


    public:
        auto get_children(shared_mega_node_t const& node){

            GIE_CHECK(node->isFolder());

            std::shared_ptr<mega::MegaNodeList> children { mega().getChildren(node.get()) };

            GIE_CHECK(children);

            return children;
        }
    private:

        auto get_node(shared_mega_node_t const& parent, path_type const p) -> shared_mega_node_t {
            auto const file_name = p.filename().string();   

            if (parent) {

                GIE_CHECK_EX(parent->isFolder(), exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

//                GIE_DEBUG_LOG("Mega node query: "<< parent->getName() << "/" << file_name << ", parent access: " << mega().getAccess(parent.get()) );


                shared_mega_node_t authorized_node{mega().getChildNode(parent.get(), file_name.c_str())};
                GIE_CHECK_EX(authorized_node, exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

                GIE_CHECK( !authorized_node->getChildren() );

                return authorized_node;

            } else {
                assert(file_name == "/");

                if(!m_root){

                    m_root.reset( self().mega().getRootNode() );

                    GIE_CHECK(m_root);

                }

                return m_root;
            }
        }


    public:

        auto get_node(path_type const& p) {

            shared_mega_node_t current_node = nullptr;

            boost::for_each(p, [&](auto&& i){
                current_node = get_node(current_node, i);
            });

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
