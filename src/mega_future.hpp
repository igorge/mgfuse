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

#include "mega_exception.hpp"

#include "gie/util-scope-exit.hpp"
#include "gie/exceptions.hpp"
#include "gie/debug.hpp"

#include "megaapi.h"

#include "boost/thread/future.hpp"

#include <memory>
//================================================================================================================================================
namespace gie {

    template <class OnCompleteFun>
    struct mega_listener_t : mega::MegaRequestListener {

        static_assert(std::is_class<OnCompleteFun>::value);

    private:
        using ResultT = typename std::invoke_result<OnCompleteFun, mega::MegaApi*, mega::MegaRequest *>::type;
        boost::promise<ResultT> m_promise;
        bool m_delete_on_finish;
        OnCompleteFun m_fun;

    public:

        void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request) override {
            GIE_DEBUG_TRACE();

        };
        virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* error){
            GIE_DEBUG_TRACE();

            GIE_SCOPE_EXIT([this]{
                if (m_delete_on_finish) {
                    GIE_DEBUG_LOG("Deleting 'mega_listener_t' in 'onRequestFinish()'");
                    delete this;
                }
            });



            try {
                GIE_MEGA_CHECK(*error);

                m_promise.set_value(m_fun(api, request));

            }catch (...){
                m_promise.set_exception(boost::current_exception());
            }
        }
        virtual void onRequestUpdate(mega:: MegaApi*api, mega::MegaRequest *request){
            GIE_DEBUG_TRACE();

        }
        virtual void onRequestTemporaryError(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError* error){
            GIE_DEBUG_TRACE();

        }

        template <class U>
        explicit mega_listener_t(U&& fun, bool delete_on_finish=true)
                : m_delete_on_finish (delete_on_finish)
                , m_fun(std::forward<U>(fun))
        {

        }

        ~mega_listener_t(){
            //m_promise.
        }

        auto future(){
            return m_promise.get_future();
        }



    };


    template <class Fun>
    std::unique_ptr< mega_listener_t<Fun> > make_listener(Fun&& fun, bool delete_on_finish=true){
        static_assert(std::is_class<Fun>::value);

        return std::make_unique< mega_listener_t<Fun> >( std::forward<Fun>(fun), delete_on_finish);
    }




}
//================================================================================================================================================
#endif
//================================================================================================================================================
