//================================================================================================================================================
// FILE: gie_fuse_fs_local.h
// (c) GIE 2016-09-01  00:16
//
//================================================================================================================================================
#ifndef H_GUARD_GIE_FUSE_FS_LOCAL_2016_09_01_00_16
#define H_GUARD_GIE_FUSE_FS_LOCAL_2016_09_01_00_16
//================================================================================================================================================
#include "gie/exceptions.hpp"
#include "boost/filesystem/fstream.hpp"
#include <fuse.h>
//================================================================================================================================================
namespace gie {

    inline std::ios_base::openmode fuse_flags_to_ios(int flags){
        std::ios_base::openmode mode = std::ios_base::binary;

        if(flags&O_TRUNC) mode|=std::ios_base::trunc;
        if(flags&O_APPEND) mode|=std::ios_base::app;
        if(flags&O_RDONLY) mode|=std::ios_base::in;
        if(flags&O_WRONLY) mode|=std::ios_base::out;
        if(flags&O_RDWR) mode|= (std::ios_base::in|std::ios_base::out);

        return mode;
    }


    struct fuse_fs_local{

        typedef boost::filesystem::fstream * file_handle_type;

        fuse_fs_local(const fuse_fs_local&) = delete;
        fuse_fs_local& operator=(const fuse_fs_local&) = delete;

        fuse_fs_local(fuse_fs_local&&) = default;

        file_handle_type open(const char * path, fuse_file_info * fi){
            assert(path);
            assert(fi);

            auto file_handle = std::make_unique<boost::filesystem::fstream>(path, fuse_flags_to_ios(fi->flags));
            file_handle->exceptions(std::ios_base::badbit | std::ios_base::failbit);

            return file_handle.release();
        }

        void release(const char * path, fuse_file_info * fi, file_handle_type const handle ){
            assert(path);
            assert(fi);
            assert(handle);

            auto file = std::unique_ptr<boost::filesystem::fstream>(handle);

            file->close();
        }


        explicit fuse_fs_local(){

        }

    };


}
//================================================================================================================================================
#endif
//================================================================================================================================================
