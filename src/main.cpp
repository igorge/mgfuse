#include "gie_fuse.hpp"


#include "gie/exceptions.hpp"
#include "gie/easy_start/safe_main.hpp"
#include "gie/debug.hpp"


#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include <iostream>

struct fuse_impl : public gie::fuse_i {
    fuse_impl(fuse_impl&&) = default;
    fuse_impl(const fuse_impl&) = delete;
    fuse_impl& operator=(const fuse_impl&) = delete;

    explicit fuse_impl(int ){
        GIE_DEBUG_TRACE();

    }

    int a = 10;


};

int main(int argc, char *argv[]) {

    return ::gie::main([&](){
        auto const old_loc  = std::locale::global(boost::locale::generator().generate(""));
        std::locale loc;
        GIE_DEBUG_LOG(  "The previous locale is: " << old_loc.name( )  );
        GIE_DEBUG_LOG(  "The current locale is: " << loc.name( )  );
        boost::filesystem::path::imbue(std::locale());


        gie::fuse_api_mapper_t<fuse_impl> fuse_mapper(fuse_impl(22));

        auto const& fuse_stat = fuse_main(argc, argv, &gie::fuse_ops, &fuse_mapper); // this forks, use -f !
        GIE_CHECK(fuse_stat==0);
    });

}