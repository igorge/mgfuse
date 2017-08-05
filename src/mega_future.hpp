//================================================================================================================================================
// FILE: mega_future.h
// (c) GIE 2016-08-26  17:10
//
//================================================================================================================================================
#ifndef H_GUARD_MEGA_FUTURE_2016_08_26_17_10
#define H_GUARD_MEGA_FUTURE_2016_08_26_17_10
//================================================================================================================================================
//#pragma once
//================================================================================================================================================

#include "gie/future.hpp"
#include "mega_exception.hpp"

#include "gie/util-scope-exit.hpp"
#include "gie/exceptions.hpp"
#include "gie/debug.hpp"

#include "megaapi.h"

#include "boost/thread/future.hpp"

#include <memory>
#include <type_traits>
//================================================================================================================================================
namespace gie {

    template <class OnCompleteFun>
    struct mega_listener_t : mega::MegaRequestListener {

        static_assert(std::is_class<OnCompleteFun>::value);

    private:
        using ResultT = typename std::invoke_result<OnCompleteFun, mega::MegaApi*, mega::MegaRequest *>::type;
        std::promise<ResultT> m_promise;
        bool m_delete_on_finish;
        OnCompleteFun const m_fun;

    public:

        void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request) override {
            GIE_DEBUG_TRACE();

        };

        void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* error) override {
            GIE_DEBUG_TRACE();

            GIE_SCOPE_EXIT([this]{
                if (m_delete_on_finish) {
                    GIE_DEBUG_LOG("Deleting 'mega_listener_t' in 'onRequestFinish()'");
                    delete this;
                }
            });

            gie::fulfill_promise(m_promise, [&]{
                GIE_MEGA_CHECK(*error);

                return m_fun(api, request);
            });
        }

        void onRequestUpdate(mega:: MegaApi*api, mega::MegaRequest *request) override {
            GIE_DEBUG_TRACE();

        }

        void onRequestTemporaryError(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError* error) override {
            GIE_DEBUG_TRACE();

            gie::fulfill_promise(m_promise, [&]() -> ResultT {
                GIE_UNIMPLEMENTED();
            });

        }

        template <class U>
        explicit mega_listener_t(U&& fun, bool delete_on_finish=false)
                : m_delete_on_finish (delete_on_finish)
                , m_fun(std::forward<U>(fun))
        {

        }

        ~mega_listener_t(){}

        decltype(auto) future(){
            return m_promise.get_future();
        }

    };


    template <typename Fun>
    mega_listener_t(Fun fun, bool del_on_exit) -> mega_listener_t< Fun >;

    template <typename Fun>
    mega_listener_t(Fun fun) -> mega_listener_t< Fun >;


    template <class Fun>
    auto make_listener(Fun&& fun, bool delete_on_finish=true){

        using FunT = std::remove_reference_t<Fun>;

        static_assert(std::is_class<FunT>::value);

        return std::make_unique< mega_listener_t<FunT> >( std::forward<Fun>(fun), delete_on_finish);
    }


 }
//================================================================================================================================================
#endif
//================================================================================================================================================
