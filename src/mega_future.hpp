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
    struct mega_request_listener_t
            : mega::MegaRequestListener
            , gie::cookie_checker<>
    {

        static_assert(std::is_class<OnCompleteFun>::value);

    private:
        using ResultT = typename std::invoke_result<OnCompleteFun, mega::MegaApi*, mega::MegaRequest *>::type;
        std::promise<ResultT> m_promise;
        OnCompleteFun const m_fun;
    public:

        void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request) override {
            GIE_DEBUG_TRACE();
            assert_cookie_is_valid();

        };

        void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* error) override {
            GIE_DEBUG_TRACE();
            assert_cookie_is_valid();

            GIE_SCOPE_EXIT([this]{
                GIE_DEBUG_LOG("Deleting 'mega_request_listener_t' in 'onRequestFinish()'");
                delete this;
            });

            gie::fulfill_promise(m_promise, [&]{
                GIE_MEGA_CHECK(*error);

                return m_fun(api, request);
            });
        }

        void onRequestUpdate(mega:: MegaApi*api, mega::MegaRequest *request) override {
            GIE_DEBUG_TRACE();
            assert_cookie_is_valid();

        }

        void onRequestTemporaryError(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError* error) override {
            GIE_DEBUG_TRACE();
            assert_cookie_is_valid();

            gie::fulfill_promise(m_promise, [&]() -> ResultT { GIE_UNIMPLEMENTED(); });
        }

        template <class U>
        explicit mega_request_listener_t(U&& fun)
                : m_fun(std::forward<U>(fun))
        {
        }

        ~mega_request_listener_t(){
            assert_cookie_is_valid();
        }

        auto get_future(){
            assert_cookie_is_valid();

            return m_promise.get_future();
        }

    };



    template <class Fun>
    auto make_request_listener(Fun &&fun){

        using FunT = std::remove_reference_t<Fun>;

        static_assert(std::is_class<FunT>::value);

        return std::make_unique< mega_request_listener_t<FunT> >( std::forward<Fun>(fun));
    }


//================================================================================================================================================


    template <typename OnCompleteFun>
    struct mega_transfer_listener_t
            : gie::cookie_checker<>
            , mega::MegaTransferListener {

        static_assert(std::is_class<OnCompleteFun>::value);

        void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer) override {
            assert_cookie_is_valid();
        }

        void onTransferFinish(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError *error) override {
            assert_cookie_is_valid();

            GIE_SCOPE_EXIT([this]{
                GIE_DEBUG_LOG("Deleting 'mega_transfer_listener_t' in 'onRequestFinish()'");
                delete this;
            });

            gie::fulfill_promise(m_promise, [&]{
                GIE_MEGA_CHECK(*error);

                return m_fun(api, transfer);
            });
        }

        void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer) override {
            assert_cookie_is_valid();
        }

        void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError *error) override {
            assert_cookie_is_valid();
        }

        ~mega_transfer_listener_t() override {
        }

        bool onTransferData(mega::MegaApi *api, mega::MegaTransfer *transfer, char *buffer, size_t size) override {
            assert_cookie_is_valid();

            return true;
        }

        auto get_future(){
            assert_cookie_is_valid();

            return m_promise.get_future();
        }

        template <class U>
        explicit mega_transfer_listener_t(U&& fun)
            : m_fun(std::forward<U>(fun))
        {
        }

    private:
        using ResultT = typename std::invoke_result<OnCompleteFun, mega::MegaApi*, mega::MegaTransfer*>::type;

        std::promise<ResultT> m_promise;
        OnCompleteFun const m_fun;

    };


    template <class Fun>
    auto make_transfer_listener(Fun &&fun){

        using FunT = std::remove_reference_t<Fun>;

        static_assert(std::is_class<FunT>::value);

        return std::make_unique< mega_transfer_listener_t<FunT> >( std::forward<Fun>(fun));
    }


 }
//================================================================================================================================================
#endif
//================================================================================================================================================
