#ifndef _Run_h_
#define _Run_h_

namespace gem {
    namespace onlinedb {

        /**
         * Groups the data needed to fill the @c <RUN> tags for XML files. How
         * this data is created remains to be understood.
         */
        struct Run
        {
            std::string type;
            std::uint64_t number;
            std::string begin;
            std::string end;
            std::string description;
            std::string location;
            std::string initiatingUser;
        };

    } /* namespace onlinedb */
} /* namespace gem */

#endif // _Run_h_
