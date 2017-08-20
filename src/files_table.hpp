//================================================================================================================================================
// FILE: files_table.h
// (c) GIE 2017-08-11  15:54
//
//================================================================================================================================================
#ifndef H_GUARD_FILES_TABLE_2017_08_11_15_54
#define H_GUARD_FILES_TABLE_2017_08_11_15_54
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "megaapi.h"

#include "gie/self_impl.hpp"

#include <boost/filesystem/fstream.hpp>

#include <unordered_map>
#include <optional>
//================================================================================================================================================
namespace gie {

    template <class SelfT>
    struct mega_files_table :  self_impl< mega_files_table<SelfT>, SelfT> {
        using mega_handle = mega::MegaHandle;
        static_assert(std::is_integral_v<mega_handle>);


        struct file_descriptor {
            std::optional<mega_handle> m_handle;  // if ! has_value() - newly created
            std::optional<boost::filesystem::fstream> m_local_stream;


        };

        //std::shared_ptr<file_descriptor> get_or_create

    private:

        std::unordered_map<mega_handle, file_descriptor> m_descriptors;

    };

}
//================================================================================================================================================
#endif
//================================================================================================================================================
