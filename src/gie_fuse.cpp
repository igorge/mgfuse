//================================================================================================================================================
// FILE: gie_fuse.cpp
// (c) GIE 2016-08-25  17:53
//
//================================================================================================================================================
//#include "stdafx.h"
//================================================================================================================================================
#include "gie_fuse.hpp"
//================================================================================================================================================

namespace gie {

    fuse_operations fuse_ops{};

}


namespace {

    template <class MainFun>
    int fuse_ctx_run(MainFun const &fun){
        try {

            auto const ctx = fuse_get_context();
            GIE_CHECK(ctx);
            GIE_CHECK(ctx->private_data);

            auto const data = static_cast<::gie::fuse_i *>(ctx->private_data);

            data->is_cookie_valid();

            return fun(data);

        } catch( boost::exception const & e ) {
            GIE_LOG( "\n======= uncaught exception =======\n" << diagnostic_information(e) );
            return -EREMOTEIO;
        } catch( std::exception const & e ) {
            GIE_LOG( "\n======= uncaught exception =======\n" << typeid(e).name() << "\n" << e.what() );
            return -EREMOTEIO;
        } catch( ... ) {
            GIE_LOG( "\n======= unknown uncaught exception =======" );
            return -EREMOTEIO;
        }

    }

    template <class MainFun>
    void* fuse_void_ptr_ctx_run(MainFun const &fun){
        try {

            return fun();

        } catch( boost::exception const & e ) {
            GIE_LOG( "\n======= uncaught exception =======\n" << diagnostic_information(e) );
            return nullptr;
        } catch( std::exception const & e ) {
            GIE_LOG( "\n======= uncaught exception =======\n" << typeid(e).name() << "\n" << e.what() );
            return nullptr;
        } catch( ... ) {
            GIE_LOG( "\n======= unknown uncaught exception =======" );
            return nullptr;
        }

    }

    void * fuse_op_init (struct fuse_conn_info *conn) noexcept {
        GIE_DEBUG_TRACE();
        return fuse_void_ptr_ctx_run([&] {

            auto const ctx = fuse_get_context();
            GIE_CHECK(ctx);
            GIE_CHECK(ctx->private_data);

            auto const data = static_cast<::gie::fuse_i *>(ctx->private_data);
            data->is_cookie_valid();

            return data;
        });
    }



    void fuse_op_destroy (void * private_data) noexcept {
        GIE_DEBUG_TRACE();
        fuse_void_ptr_ctx_run([&] {

            GIE_CHECK(private_data);

            auto const data = static_cast<::gie::fuse_i *>(private_data);
            data->is_cookie_valid();

            return nullptr;
        });
    }



    int fuse_op_getattr(const char * path, struct stat *) noexcept {
        GIE_DEBUG_TRACE1(path);
        return fuse_ctx_run([&](::gie::fuse_i * const data){
            GIE_UNIMPLEMENTED();
            return -EACCES;
        });
    }


    //int (*readlink) (const char *, char *, size_t);
//    int (*mkdir) (const char *, mode_t);
//    int (*unlink) (const char *);
//    int (*rmdir) (const char *);
//    int (*rename) (const char *, const char *);
//    int (*truncate) (const char *, off_t);



    int fuse_op_open(const char * path, struct fuse_file_info * fi) noexcept {
        GIE_DEBUG_TRACE1(path);
        return fuse_ctx_run([&](::gie::fuse_i * const data){
            return data->open(path, fi);
        });
    }

//    int (*read) (const char *, char *, size_t, off_t,
//                 struct fuse_file_info *);
//
//    int (*write) (const char *, const char *, size_t, off_t,
//                  struct fuse_file_info *);
//    int (*flush) (const char *, struct fuse_file_info *);
//
//    int (*release) (const char *, struct fuse_file_info *);
//
//    int (*fsync) (const char *, int, struct fuse_file_info *);


    int fuse_op_opendir(const char * path, struct fuse_file_info *) noexcept {
        GIE_DEBUG_TRACE1(path);
        return fuse_ctx_run([&](::gie::fuse_i * const data){

            GIE_UNIMPLEMENTED();
            return -EACCES;
        });
    }


//    int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t,
//                    struct fuse_file_info *);

    int fuse_op_releasedir(const char * path, struct fuse_file_info *) noexcept {
        GIE_DEBUG_TRACE1(path);

        return fuse_ctx_run([&](::gie::fuse_i * const data){

            GIE_UNIMPLEMENTED();
            return -EACCES;
        });
    }

//    int (*access) (const char *, int);
//    int (*create) (const char *, mode_t, struct fuse_file_info *);
//    int (*ftruncate) (const char *, off_t, struct fuse_file_info *);
//    int (*fgetattr) (const char *, struct stat *, struct fuse_file_info *);


    struct fuse_ops_initer_t {

        fuse_ops_initer_t(){
            GIE_DEBUG_TRACE();

            using ::gie::fuse_ops;

            fuse_ops.init = fuse_op_init;
            fuse_ops.destroy = fuse_op_destroy;
            fuse_ops.getattr = fuse_op_getattr;
            fuse_ops.open = fuse_op_open;
            fuse_ops.opendir = fuse_op_opendir;
            fuse_ops.releasedir = fuse_op_releasedir;

        }

    };


    fuse_ops_initer_t fuse_static_initer;

}
//================================================================================================================================================
