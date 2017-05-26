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

                GIE_CHECK(parent->mega_node->isFolder());

                if(auto const& r = parent->children.find(file_name); r!=parent->children.end()){
                    GIE_DEBUG_LOG("Mega node cache hit: "<<file_name);
                    return r->second;
                } else {
                    GIE_DEBUG_LOG("Mega node cache miss: "<< parent->mega_node->getName() << "/" << file_name << ", parent access: " << mega().getAccess(parent->mega_node.get()) );

                    auto const children = parent->mega_node->getChildren(); // MegaNode keeps ownership

                    GIE_CHECK(children);

                    auto const& as_range = to_range(children);
                    auto const found_node_iter = boost::find_if(as_range, [&](mega::MegaNode* node){
                        return file_name==node->getName();
                    });

                    GIE_CHECK_EX(found_node_iter!=as_range.end(), exception::fuse_no_such_file_or_directory() << gie::exception::error_str_einfo(file_name) );

                    auto const found_node = *found_node_iter;
                    GIE_CHECK(found_node);

                    //shared_mega_node_t authorized_node{authorize(*found_node).release()};
                    shared_mega_node_t authorized_node{found_node->copy()};

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

                    std::unique_ptr < mega::MegaNode > root{self()->mega().getRootNode()};
                    GIE_CHECK(root);

                    m_root->mega_node.reset( authorize(*root).release() );
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
