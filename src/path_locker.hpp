//================================================================================================================================================
// FILE: path_locker.h
// (c) GIE 2017-07-23  15:12
//
//================================================================================================================================================
#ifndef H_GUARD_PATH_LOCKER_2017_07_23_15_12
#define H_GUARD_PATH_LOCKER_2017_07_23_15_12
//================================================================================================================================================
#pragma once
//================================================================================================================================================
#include "gie/exceptions.hpp"

#include <boost/filesystem/path.hpp>

#include <thread>
#include <mutex>
#include <future>
#include <chrono>
#include <map>
#include <set>
//================================================================================================================================================



namespace gie { namespace locker {

    using path = ::boost::filesystem::path;

    template <typename SelfT, typename Key>
    struct client {
    private:
        using this_type = client;
        auto& self(){ return *static_cast<SelfT*>(this); }

        std::chrono::seconds const timeout{30};


        struct key_users_t{
            explicit key_users_t(std::thread::id p_owner) : owner(p_owner) {}
            key_users_t(key_users_t&&) = default;

            std::thread::id owner;
            std::promise<void>          promise{};
            std::shared_future<void>    future{promise.get_future()};
        };

        std::map<Key, key_users_t> m_key_map;
        std::mutex  m_lock;

        using iter_t = typename decltype(m_key_map)::iterator;

        struct scoped_lock;

        template <class KeyT>
        auto lock_(KeyT&& key) -> scoped_lock {

            for(;;) {
                std::unique_lock lk{m_lock};

                if (auto const &iter = m_key_map.find(key); iter == m_key_map.end())
                {
                    auto const& [inserted_iter, is_inserted] = m_key_map.insert(std::make_pair(std::forward<KeyT>(key), key_users_t{std::this_thread::get_id()}));
                    assert(is_inserted);

                    return {*this, inserted_iter};

                } else {

                    auto &value = iter->second;

                    auto future = value.future;

                    lk.unlock();

                    auto const r = future.wait_for(timeout);
                    GIE_CHECK(r == std::future_status::ready);
                }
            }

        }

        void unlock_(iter_t const& iter){

//            std::this_thread::sleep_for(std::chrono::seconds(32));

            std::lock_guard lk{m_lock};

            auto& value = iter->second;
            GIE_CHECK( std::this_thread::get_id()==value.owner );
            value.promise.set_value();
            m_key_map.erase(iter);
        }



        struct scoped_lock{

            this_type& m_self;
            iter_t m_iter;

            template <class IterT>
            scoped_lock(this_type& self, IterT&& iter)
                : m_self(self), m_iter( std::forward<IterT>(iter) )
            {
            }

            scoped_lock(scoped_lock const&) = delete;
            scoped_lock(scoped_lock&&) = default;


            ~scoped_lock(){
                m_self.unlock_(m_iter);
            }
        private:

        };
    public:

        template <class KeyT>
        auto lock(KeyT&& key){
            return lock_( std::forward<KeyT>(key) );
        }

    };



    struct path_locker_t : client<path_locker_t, path>
    {

    };

} }
//================================================================================================================================================
#endif
//================================================================================================================================================
