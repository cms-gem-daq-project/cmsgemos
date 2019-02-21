#ifndef GEM_ONLINEDB_DETAIL_FILEUTILS_H
#define GEM_ONLINEDB_DETAIL_FILEUTILS_H

#include <string>

namespace gem {
    namespace onlinedb {
        namespace detail {

            /**
             * @brief Finds a file in a colon-separated list of directories.
             * @throws boost::filesystem::filesystem_error If no file is found.
             */
            std::string getFileInPath(const std::string &filename,
                                      const std::string &path);

        } // namespace detail
    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_DETAIL_FILEUTILS_H
