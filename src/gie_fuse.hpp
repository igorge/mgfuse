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

#include "exceptions.hpp"

#include <boost/exception/all.hpp>

#include <fuse.h>

#include <type_traits>
#include <utility>
//================================================================================================================================================
namespace gie {


    using fuse_op_implemented = std::true_type;

    enum struct fuse_filter_type { no, generic, custom };

    using fuse_filter_type_no = std::integral_constant<fuse_filter_type, fuse_filter_type::no>;
    using fuse_filter_type_generic = std::integral_constant<fuse_filter_type, fuse_filter_type::generic>;
    using fuse_filter_type_custom = std::integral_constant<fuse_filter_type, fuse_filter_type::custom>;


    template <typename IsImplemented = std::false_type, typename ExceptionFilterType = std::integral_constant<fuse_filter_type, fuse_filter_type::no> >
    struct fuse_method_def {
        using is_implemented = IsImplemented;
        using exception_filter_type = ExceptionFilterType;

    };


//    struct fuse_i : gie::cookie_checker<> {
//
//        virtual void getattr(const char * path, struct stat * st) {
//            GIE_UNIMPLEMENTED();
//        }
//
//        virtual void fgetattr(const char * path, struct stat * st, struct fuse_file_info * fi){
//            GIE_UNIMPLEMENTED();
//        }
//
//        virtual void open (const char *, struct fuse_file_info *){
//            GIE_UNIMPLEMENTED();
//        }
//
//        virtual void release(const char *, struct fuse_file_info *){
//            GIE_UNIMPLEMENTED();
//        }
//
//        virtual void opendir(const char * path, struct fuse_file_info *){
//            GIE_UNIMPLEMENTED();
//        }
//
//        virtual void releasedir(const char * path, struct fuse_file_info *){
//            GIE_UNIMPLEMENTED();
//        }
//
//        virtual void readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi){
//            GIE_UNIMPLEMENTED();
//        }
//
//
//        virtual void mkdir(const char* path, mode_t mode){
//            GIE_UNIMPLEMENTED();
//        }
//
//    };



    template <class FuseImplementation>
    struct fuse_api_mapper_t : gie::cookie_checker<> {


        using self_t = fuse_api_mapper_t;

        static_assert( std::is_class<FuseImplementation>::value );

        template<class T> struct dependent_false : std::false_type {};

        typedef typename FuseImplementation::file_handle_type file_handle_type;
        typedef typename FuseImplementation::directory_handle_type directory_handle_type;

        typedef decltype(fuse_file_info::fh) internal_fuse_file_handle_type;

        fuse_api_mapper_t(const fuse_api_mapper_t&) = delete;
        fuse_api_mapper_t& operator=(const fuse_api_mapper_t&) = delete;


    private:
        FuseImplementation m_fuse_imp;

        fuse_operations m_fuse_ops{};

        auto& impl(){
            return m_fuse_imp;
        }

    public:
        fuse_operations* internal_fuse_operation(){
            return &m_fuse_ops;
        }

    private:


        template <class MainFun>
        static int fuse_ctx_run(MainFun const &fun){
            try {

                auto const ctx = fuse_get_context();
                GIE_CHECK(ctx);
                GIE_CHECK(ctx->private_data);

                auto const data = static_cast<self_t *>(ctx->private_data);

                data->is_cookie_valid();

                return fun(data);
            } catch (gie::exception::unimplemented const& e) {
                GIE_LOG( "\n======= UNIMPLEMENTED =======\n" << diagnostic_information(e) );
                return -EOPNOTSUPP;
            } catch( exception::fuse_no_such_file_or_directory const & e ) {
                return -ENOENT;
            } catch( boost::exception const & e ) {
                GIE_LOG( "\n======= uncaught exception =======\n" << diagnostic_information(e) );

                if(auto * const ei = boost::get_error_info<exception::fuse_errorno_einfo>(e); ei){
                    return -(*ei);
                } else {
                    return -EREMOTEIO;
                }

            } catch( std::exception const & e ) {
                GIE_LOG( "\n======= uncaught exception =======\n" << typeid(e).name() << "\n" << e.what() );
                return -EREMOTEIO;
            } catch( ... ) {
                GIE_LOG( "\n======= unknown uncaught exception =======" );
                return -EREMOTEIO;
            }

        }

        template <class MainFun, class Filter>
        static int fuse_ctx_run(MainFun const &fun, Filter const& filter){
            return fuse_ctx_run([&](auto&& data){
                try {
                    return fun(data);
                } catch(...){
                    filter();
                }
                GIE_UNEXPECTED();
            });
        };


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


        static internal_fuse_file_handle_type to_internal_fuse_handle(file_handle_type const& handle) noexcept {
            BOOST_STATIC_ASSERT(std::is_pointer<file_handle_type>::value);
            BOOST_STATIC_ASSERT(sizeof(internal_fuse_file_handle_type)>=sizeof(file_handle_type));

            return reinterpret_cast<internal_fuse_file_handle_type>(handle); //allowed by c++11
        }

        file_handle_type static from_internal_fuse_handle(internal_fuse_file_handle_type const& handle) noexcept {
            return reinterpret_cast<file_handle_type>(handle);
        }

        internal_fuse_file_handle_type static directory_to_internal_fuse_handle(directory_handle_type const& handle) noexcept {
            BOOST_STATIC_ASSERT(std::is_pointer<file_handle_type>::value);
            BOOST_STATIC_ASSERT(sizeof(internal_fuse_file_handle_type)>=sizeof(directory_handle_type));

            return reinterpret_cast<internal_fuse_file_handle_type>(handle); //allowed by c++11
        }

        directory_handle_type static directory_from_internal_fuse_handle(internal_fuse_file_handle_type const& handle) noexcept {
            return reinterpret_cast<directory_handle_type>(handle);
        }

        static void * fuse_op_init (struct fuse_conn_info *conn) noexcept {
            GIE_DEBUG_TRACE();
            return fuse_void_ptr_ctx_run([&] {

                auto const ctx = fuse_get_context();
                GIE_CHECK(ctx);
                GIE_CHECK(ctx->private_data);

                auto const data = static_cast<self_t *>(ctx->private_data);
                data->is_cookie_valid();

                return data;
            });
        }

        static void fuse_op_destroy (void * private_data) noexcept {
            GIE_DEBUG_TRACE();
            fuse_void_ptr_ctx_run([&] {

                GIE_CHECK(private_data);

                auto const data = static_cast<self_t *>(private_data);
                data->is_cookie_valid();

                return nullptr;
            });
        }

        template <typename OpDef, typename Fun>
        static auto run_with_filter(Fun&& fun){

            static_assert(OpDef::is_implemented::value, "fuse operation must be implemented.");

            if constexpr( OpDef::exception_filter_type::value == fuse_filter_type::no )
            {
                return fuse_ctx_run([&](self_t *const data) {
                    return fun(data);
                });
            } else if constexpr( OpDef::exception_filter_type::value == fuse_filter_type::generic ) {
                static_assert(dependent_false<OpDef>::value);
            } else if constexpr( OpDef::exception_filter_type::value == fuse_filter_type::custom ) {
                static_assert(dependent_false<OpDef>::value);
            } else {
                static_assert(dependent_false<OpDef>::value);
            }
        }


        static int fuse_op_getattr(const char * path, struct stat * st) noexcept {
            GIE_DEBUG_TRACE1(path);

            return run_with_filter<typename FuseImplementation::fuse_op_def_getattr>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(st);

                data->impl().getattr(path, st);
                return 0;
            });
        }


        static int fuse_op_fgetattr(const char * path, struct stat * st, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);
            return run_with_filter<typename FuseImplementation::fuse_op_def_fgetattr>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(st);
                GIE_CHECK(fi);

                data->impl().fgetattr(path, st, fi);

                return 0;
            });
        }

        static int fuse_op_open(const char * path, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);

            return run_with_filter<typename FuseImplementation::fuse_op_def_open>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(fi);
                GIE_CHECK( !(fi->flags & O_CREAT) );

                fi->fh=to_internal_fuse_handle(data->impl().open(path, fi));

                return 0;
            });
        }

        static int fuse_op_release(const char * path, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);
            return run_with_filter<typename FuseImplementation::fuse_op_def_release>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(fi);

                GIE_CHECK(fi->fh);

                data->impl().release(path, fi, from_internal_fuse_handle(fi->fh));

                return 0;
            });
        }


        static int fuse_op_opendir(const char * path, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);
            return run_with_filter<typename FuseImplementation::fuse_op_def_opendir>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(fi);
                GIE_CHECK(!fi->fh);

                fi->fh=directory_to_internal_fuse_handle(data->impl().opendir(path, fi));

                return 0;
            });
        }


        static int fuse_op_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);
            return run_with_filter<typename FuseImplementation::fuse_op_def_readdir>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(fi);

                GIE_CHECK(fi->fh);

                data->impl().readdir(path, buf, filler, offset, fi, directory_from_internal_fuse_handle(fi->fh));

                return 0;
            });
        }

        static int fuse_op_releasedir(const char * path, struct fuse_file_info * fi) noexcept {
            GIE_DEBUG_TRACE1(path);

            return run_with_filter<typename FuseImplementation::fuse_op_def_releasedir>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);
                GIE_CHECK(fi);

                GIE_CHECK(fi->fh);

                data->impl().releasedir(path, fi, directory_from_internal_fuse_handle(fi->fh));

                return 0;
            });
        }

        static int fuse_op_mkdir(const char* path, mode_t mode) noexcept {
            GIE_DEBUG_TRACE1(path);

            return run_with_filter<typename FuseImplementation::fuse_op_def_mkdir>([&](self_t * const data){
                data->assert_cookie_is_valid();

                GIE_CHECK(path);

                data->impl().mkdir(path, mode);

                return 0;
            });

        }


    public:
        fuse_api_mapper_t(FuseImplementation&& fi) : m_fuse_imp(std::move(fi)) {

            m_fuse_ops.init = fuse_op_init;
            m_fuse_ops.destroy = fuse_op_destroy;

            #define GIE_GEN_FUSE_ENRTY(PP_def_name) \
                if constexpr( FuseImplementation::fuse_op_def_ ## PP_def_name::is_implemented::value ){ m_fuse_ops.PP_def_name = fuse_op_ ##PP_def_name; } else {GIE_DEBUG_LOG(#PP_def_name " not implemented.");} \
            /**/

            GIE_GEN_FUSE_ENRTY(getattr);
            GIE_GEN_FUSE_ENRTY(fgetattr);

            GIE_GEN_FUSE_ENRTY(open);
            GIE_GEN_FUSE_ENRTY(release);

            GIE_GEN_FUSE_ENRTY(opendir);
            GIE_GEN_FUSE_ENRTY(releasedir);
            GIE_GEN_FUSE_ENRTY(readdir);
            GIE_GEN_FUSE_ENRTY(mkdir);
        }

    };



}


//================================================================================================================================================
#endif
//================================================================================================================================================
