//================================================================================================================================================
// FILE: gie_fuse.h
// (c) GIE 2016-08-25  17:53
//
//================================================================================================================================================
#ifndef H_GUARD_GIE_FUSE_2016_08_25_17_53
#define H_GUARD_GIE_FUSE_2016_08_25_17_53
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "gie/debug/magic_cookie.hpp"
#include "gie/exceptions.hpp"
#include "gie/debug.hpp"

#include "boost/tti/has_member_function.hpp"

#include <fuse.h>

#include <type_traits>
#include <utility>
//================================================================================================================================================
namespace gie {





    struct fuse_i : gie::cookie_checker<> {

        virtual int open (const char *, struct fuse_file_info *){
            GIE_UNIMPLEMENTED();
        }

        virtual void release(const char *, struct fuse_file_info *){
            GIE_UNIMPLEMENTED();
        }



    };





    template <class FuseImplementation>
    struct fuse_api_mapper_t: fuse_i {

        typedef decltype(fuse_file_info::fh) internal_fuse_file_handle_type;
        typedef typename FuseImplementation::file_handle_type file_handle_type;

        fuse_api_mapper_t(const fuse_api_mapper_t&) = delete;
        fuse_api_mapper_t& operator=(const fuse_api_mapper_t&) = delete;

    private:

        BOOST_TTI_HAS_MEMBER_FUNCTION(open);
        BOOST_TTI_HAS_MEMBER_FUNCTION(release);

        enum avaliable_ops {
            HAS_OPEN = has_member_function_open<FuseImplementation, file_handle_type, boost::mpl::vector<const char *,fuse_file_info*> >::value,
            HAS_RELEASE = has_member_function_release<FuseImplementation, void, boost::mpl::vector<const char *,fuse_file_info*, const file_handle_type> >::value
        };

    private:
        FuseImplementation m_fuse_imp;

        fuse_operations m_fuse_ops{};

    private:


        template <class MainFun>
        static int fuse_ctx_run(MainFun const &fun){
            try {

                auto const ctx = fuse_get_context();
                GIE_CHECK(ctx);
                GIE_CHECK(ctx->private_data);

                auto const data = static_cast<::gie::fuse_i *>(ctx->private_data);

                data->is_cookie_valid();

                return fun(data);
            } catch (gie::exception::unimplemented const& e) {
                GIE_LOG( "\n======= UNIMPLEMENTED =======\n" << diagnostic_information(e) );
                return -EOPNOTSUPP;
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
        static void* fuse_void_ptr_ctx_run(MainFun const &fun){
            try {

                return fun();

            } catch (gie::exception::unimplemented const& e) {
                GIE_LOG( "\n======= UNIMPLEMENTED =======\n" << diagnostic_information(e) );
            } catch( boost::exception const & e ) {
                GIE_LOG( "\n======= uncaught exception =======\n" << diagnostic_information(e) );
            } catch( std::exception const & e ) {
                GIE_LOG( "\n======= uncaught exception =======\n" << typeid(e).name() << "\n" << e.what() );
            } catch( ... ) {
                GIE_LOG( "\n======= unknown uncaught exception =======" );
            }

            return nullptr;
        }


        internal_fuse_file_handle_type to_internal_fuse_handle(file_handle_type const& handle) noexcept {
            BOOST_STATIC_ASSERT(std::is_pointer<file_handle_type>::value);
            BOOST_STATIC_ASSERT(sizeof(internal_fuse_file_handle_type)>=sizeof(file_handle_type));

            return reinterpret_cast<internal_fuse_file_handle_type>(handle); //allowed by c++11
        }

        file_handle_type from_internal_fuse_handle(internal_fuse_file_handle_type const& handle) noexcept {
            return reinterpret_cast<file_handle_type>(handle);
        }



        static void * fuse_op_init (struct fuse_conn_info *conn) noexcept {
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



        static void fuse_op_destroy (void * private_data) noexcept {
            GIE_DEBUG_TRACE();
            fuse_void_ptr_ctx_run([&] {

                GIE_CHECK(private_data);

                auto const data = static_cast<::gie::fuse_i *>(private_data);
                data->is_cookie_valid();

                return nullptr;
            });
        }

        static int fuse_op_open(const char * path, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);
            return fuse_ctx_run([&](::gie::fuse_i * const data){
                return data->open(path, fi);
            });
        }

        static int fuse_op_release(const char * path, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);
            return fuse_ctx_run([&](::gie::fuse_i * const data){
                data->release(path, fi);
                return 0;
            });
        }


        //
        // open
        //

        template<bool dummy=true>
        typename std::enable_if< HAS_OPEN && dummy, void >::type open_dispatch(const char * path, struct fuse_file_info * fi) {
            fi->fh=to_internal_fuse_handle(m_fuse_imp.open(path, fi));
        }

        template<bool dummy=true>
        typename std::enable_if< !HAS_OPEN && dummy, void >::type open_dispatch(const char * path, struct fuse_file_info * fi) {
            GIE_UNIMPLEMENTED();
        }


        int open(const char * path, struct fuse_file_info * fi) override {

            GIE_CHECK(path);
            GIE_CHECK(fi);
            GIE_CHECK( !(fi->flags & O_CREAT) );

            open_dispatch<>(path, fi);

            return  0;
        }

        //
        // release
        //

        template<bool dummy=true>
        typename std::enable_if< HAS_RELEASE && dummy, void >::type release_dispatch(const char * path, struct fuse_file_info * fi) {
            GIE_CHECK(fi->fh);

            m_fuse_imp.release(path, fi, from_internal_fuse_handle(fi->fh));
        }

        template<bool dummy=true>
        typename std::enable_if< !HAS_RELEASE && dummy, void >::type release_dispatch(const char * path, struct fuse_file_info * fi) {
            GIE_UNIMPLEMENTED();
        }

        void release(const char * path, struct fuse_file_info * fi) override {
            GIE_CHECK(path);
            GIE_CHECK(fi);

            release_dispatch<>(path, fi);
        }



    public:
        fuse_api_mapper_t(FuseImplementation&& fi) : m_fuse_imp(std::move(fi)) {

            m_fuse_ops.init = fuse_op_init;
            m_fuse_ops.destroy = fuse_op_destroy;

            if(HAS_OPEN){ m_fuse_ops.open = fuse_op_open; } else {GIE_DEBUG_LOG("!HAS_OPEN");}
            if(HAS_RELEASE){ m_fuse_ops.release = fuse_op_release; } else {GIE_DEBUG_LOG("!HAS_RELEASE");}




            // m_fuse_ops.open = impl::fuse_op_open;
        }

    };



}


//================================================================================================================================================
#endif
//================================================================================================================================================
