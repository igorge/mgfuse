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
#include "megaapi.h"

#include <memory>
//================================================================================================================================================
namespace gie {

    template<class SelfT>
    struct mega_node_cache_t {

    private:

        using path_type = boost::filesystem::path;

        struct cached_node_t;

        using shared_mega_node_t = std::shared_ptr<mega::MegaNode>;
        using shared_cached_node = std::shared_ptr<cached_node_t>;

        using map_t = std::map<std::string, shared_cached_node>;


        auto self() {
            return static_cast<SelfT *>(this);
        }


        struct cached_node_t {
            shared_mega_node_t mega_node;
            map_t children;
        };

        shared_cached_node m_root;

        auto authorize(mega::MegaNode &node) -> std::unique_ptr<mega::MegaNode> {

            std::unique_ptr<mega::MegaNode> authorized_node{ self()->mega().authorizeNode(&node) };
            GIE_CHECK(authorized_node.get());
            return authorized_node;

        }

        auto get_node(shared_cached_node const& parent, path_type const p) -> shared_cached_node {
            auto const file_name = p.filename().string();

            if (parent) {

                if(auto const& r = parent->children.find(file_name); r!=parent->children.end()){
                    GIE_DEBUG_LOG("Mega node cache hit: "<<file_name);
                    return r->second;
                }

//            auto const children = to_range(parent->getChildren());
//            auto const found_node = boost::find_if(children, [&](mega::MegaNode* node){
//                return file_name==node->getName();
//            });
//
//            if(found_node==children.end()){
//                GIE_THROW(exception::fuse_no_such_file_or_directory());
//            } else {
//                return std::unique_ptr<mega::MegaNode>{ (*found_node)->copy()};
//            }

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
        }

        auto &nodes() {
            return *this;
        }


    };
}
//================================================================================================================================================
#endif
//================================================================================================================================================
