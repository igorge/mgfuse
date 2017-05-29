//================================================================================================================================================
// FILE: gie_fuse_fs_local.h
// (c) GIE 2016-09-01  00:16
//
//================================================================================================================================================
#ifndef H_GUARD_GIE_FUSE_FS_LOCAL_2016_09_01_00_16
#define H_GUARD_GIE_FUSE_FS_LOCAL_2016_09_01_00_16
//================================================================================================================================================
#include "gie_fuse.hpp"
#include "gie/exceptions.hpp"
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <fuse.h>
//================================================================================================================================================
namespace gie {

    inline std::ios_base::openmode fuse_flags_to_ios(int flags){
        std::ios_base::openmode mode = std::ios_base::binary;

        switch( (flags&O_ACCMODE) ) {
            case O_RDONLY: mode|=std::ios_base::in; break;
            case O_WRONLY: mode|=std::ios_base::out; break;
            case O_RDWR  : mode|= (std::ios_base::in|std::ios_base::out); break;
            default: GIE_UNEXPECTED();
        }

        if(flags&O_TRUNC) mode|=std::ios_base::trunc;
        if(flags&O_APPEND) mode|=std::ios_base::app;


        return mode;
    }


    struct fuse_fs_local{


        using fuse_op_def_opendir = fuse_method_def<fuse_op_implemented>;

        enum supported_ops {
            HAS_GETATTR=true,
            HAS_FGETATTR=false,
            HAS_OPEN=true,
            HAS_RELEASE=true,
//            HAS_OPENDIR=true,
            HAS_RELEASEDIR=true,
            HAS_READDIR=true
        };

        typedef boost::filesystem::fstream * file_handle_type;

        struct directory_handle {

            boost::filesystem::directory_iterator m_i;
            boost::filesystem::directory_iterator m_end;
            boost::filesystem::path path;

            bool m_eof = false;
            off_t m_offset = 0;

            explicit directory_handle(boost::filesystem::path const& p_path)
                : m_i(p_path)
            {
            }
        };
        using directory_handle_type = directory_handle*;

        fuse_fs_local(const fuse_fs_local&) = delete;
        fuse_fs_local& operator=(const fuse_fs_local&) = delete;

        fuse_fs_local(fuse_fs_local&&) = default;

        boost::filesystem::path m_root;

        auto open(const char * path, fuse_file_info * fi) -> file_handle_type {
            assert(path);
            assert(fi);

            auto file_handle = std::make_unique<boost::filesystem::fstream>(m_root / path, fuse_flags_to_ios(fi->flags));
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


        void getattr(const char * path, struct stat * st){
            assert(path);

            if( ::stat( (m_root / path).c_str() , st ) ==-1) {
                auto const status = errno;
                if (status==ENOENT) {
                    GIE_THROW(exception::fuse_no_such_file_or_directory());
                } else {
                    GIE_THROW( exception::fuse_stat_failed() << exception::fuse_errorno_einfo(status));
                }
            }
        }


        auto opendir(boost::filesystem::path const& path, fuse_file_info * fi) -> directory_handle_type{
            assert(!path.empty());
            assert(fi);

            auto dir = std::make_unique<directory_handle>(m_root / path);

            return dir.release();
        }

        void releasedir(const char * path, fuse_file_info * fi, directory_handle_type const handle ){
            assert(path);
            assert(fi);
            assert(handle);

            auto dir = std::unique_ptr<directory_handle>(handle);
        }


        void readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi, directory_handle_type const handle) {
            assert(path);
            assert(fi);
            assert(handle);

            GIE_CHECK(!handle->m_eof);


            bool filled_up = false;
            while(!filled_up && handle->m_i!=handle->m_end){

                auto const filename = (*handle->m_i).path().filename().native();
                GIE_CHECK(!filename.empty());
                ++handle->m_i;

                if(handle->m_i!=handle->m_end) {
                    ++handle->m_offset;
                }

                if(auto const r =  filler(buf, filename.c_str(), nullptr, handle->m_offset); r == 1){
                    filled_up = true;
                } else if(r==0) {
                    // do nothing
                } else {
                    GIE_UNEXPECTED();
                }

            }

        }



        explicit fuse_fs_local(boost::filesystem::path const& root) : m_root(root) {

        }

    };


}
//================================================================================================================================================
#endif
//================================================================================================================================================
