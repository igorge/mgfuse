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

    private:

        using path_type = boost::filesystem::path;

        struct cached_node_t;

        using shared_mega_node_t = std::shared_ptr<mega::MegaNode>;
        using shared_cached_node = std::shared_ptr<cached_node_t>;

        using map_t = std::map<std::string, shared_cached_node>;


        auto self() {
            return static_cast<SelfT *>(this);
        }

        decltype(auto) mega(){
            return self()->mega();
        }


        struct cached_node_t {
            shared_mega_node_t mega_node;
            map_t children;
        };

        shared_cached_node m_root;

        void clear(){
            m_root.reset();
        }

        auto authorize(mega::MegaNode &node) -> std::unique_ptr<mega::MegaNode> {

            std::unique_ptr<mega::MegaNode> authorized_node{ self()->mega().authorizeNode(&node) };
            GIE_CHECK(authorized_node); // failed to authorize
            return authorized_node;

        }

        auto get_node(shared_cached_node const& parent, path_type const p) -> shared_cached_node {
            auto const file_name = p.filename().string();   

            if (parent) {

                GIE_CHECK_EX(parent->mega_node->isFolder(), exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

                if(auto const& r = parent->children.find(file_name); r!=parent->children.end()){
                    GIE_DEBUG_LOG("Mega node cache hit: "<<file_name);
                    return r->second;
                } else {
                    GIE_DEBUG_LOG("Mega node cache miss: "<< parent->mega_node->getName() << "/" << file_name << ", parent access: " << mega().getAccess(parent->mega_node.get()) );


                    shared_mega_node_t authorized_node{mega().getChildNode(parent->mega_node.get(), file_name.c_str())};
                    GIE_CHECK_EX(authorized_node, exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

                    GIE_CHECK( !authorized_node->getChildren() );

                    //insert into cache
                    auto cache_node = std::make_shared<cached_node_t>();
                    cache_node->mega_node = std::move(authorized_node);

                    parent->children.insert(std::make_pair(file_name, cache_node) );

                    return cache_node;
                }

            } else {
                assert(file_name == "/");

                if(!m_root){

                    m_root = std::make_shared<cached_node_t>();
                    m_root->mega_node.reset( self()->mega().getRootNode() );

                    GIE_CHECK(m_root->mega_node);

                }

                return m_root;
            }
        }


    public:

        auto get_node(path_type const p) {

            shared_cached_node current_node = nullptr;

            for (auto &&i : p) {
                current_node = get_node(current_node, i);
            }

            return current_node->mega_node;
        }

        auto& nodes() {
            return *this;
        }


    };
}
//================================================================================================================================================
#endif
//================================================================================================================================================
