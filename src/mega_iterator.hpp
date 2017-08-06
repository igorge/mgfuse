//================================================================================================================================================
// FILE: mega_iterator.h
// (c) GIE 2017-05-12  00:08
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_ITERATOR_2017_05_12_00_08
#define H_GUARD_MEGA_ITERATOR_2017_05_12_00_08
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "megaapi.h"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>
//================================================================================================================================================
namespace gie {


    struct mega_node_list_iterator : ::boost::iterator_facade<
                    mega_node_list_iterator,
                    mega::MegaNode*,
                    boost::forward_traversal_tag,
                    mega::MegaNode* const //reference type
                 >
    {
        mega_node_list_iterator()
                : m_node_list(nullptr)
        {}

        explicit mega_node_list_iterator(mega::MegaNodeList* node_list)
                : m_node_list(node_list)
                , m_idx(0)
        {
            assert(node_list);
        }
    private:
        friend class ::boost::iterator_core_access;

        void increment()
        {
            assert(m_node_list);
            assert(m_idx!=-1);

            assert(m_idx < m_node_list->size() );

            ++m_idx;
        }

        bool equal(mega_node_list_iterator const& other) const
        {

            if(m_node_list == other.m_node_list){
                return  m_idx == other.m_idx;
            } else if(m_node_list==nullptr) {
                return other.equal(*this);
            } else if(other.m_node_list==nullptr){
                if(m_idx==m_node_list->size()) return true; else return false;
            } else {
                GIE_UNEXPECTED();
            }

        }

        auto dereference() const -> value_type {
            auto node = m_node_list->get(m_idx);
            assert(node);
            return node;
        }

        mega::MegaNodeList * const m_node_list;
        int m_idx = -1;
    };


    inline
    auto to_range(mega::MegaNodeList * const node_list){
        return boost::make_iterator_range( mega_node_list_iterator(node_list), mega_node_list_iterator() );
    }


}
//================================================================================================================================================
#endif
//================================================================================================================================================
