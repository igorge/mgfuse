#include "mega_fuse.hpp"

#include "gie_fuse.hpp"

#include "gie/exceptions.hpp"
#include "gie/easy_start/safe_main.hpp"
#include "gie/debug.hpp"

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include <iostream>

int main(int argc, char *argv[]) {

    return ::gie::main([&](){

        auto const old_loc  = std::locale::global(boost::locale::generator().generate(""));
        std::locale loc;
        GIE_DEBUG_LOG(  "The previous locale is: " << old_loc.name( )  );
        GIE_DEBUG_LOG(  "The current locale is: " << loc.name( )  );
        boost::filesystem::path::imbue(std::locale());

        GIE_CHECK(argc==2);

        gie::fuse_api_mapper_t<gie::fuse_impl> fuse_mapper(gie::fuse_impl(22));

//        std::array< char const *, 3 > const fuseargv = { argv[0], "-f", argv[1]};
//        auto const& fuse_stat = fuse_main(fuseargv.size(), const_cast<char**>(fuseargv.data()), &gie::fuse_ops, &fuse_mapper);
//        GIE_CHECK(fuse_stat==0);
    });

}