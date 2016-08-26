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
//================================================================================================================================================
namespace gie {

    struct mega_listener_t : mega::MegaRequestListener {

        private:
            boost::promise<int> m_promise;
            bool m_delete_on_finish;

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

            explicit mega_listener_t(bool delete_on_finish=true) : m_delete_on_finish (delete_on_finish) {

            }

            ~mega_listener_t(){
                //m_promise.
            }

            auto future(){
                return m_promise.get_future();
            }



        };



}
//================================================================================================================================================
#endif
//================================================================================================================================================
