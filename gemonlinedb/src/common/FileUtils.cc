#include "gem/onlinedb/detail/FileUtils.h"

#include <system_error>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace gem {
    namespace onlinedb {
        namespace detail {

            std::string getFileInPath(const std::string &filename,
                                      const std::string &path)
            {
                std::vector<std::string> dirs;
                boost::split(dirs, path, boost::is_any_of(":"));
                for (const auto &dir : dirs) {
                    boost::filesystem::path path = dir;
                    path /= filename;
                    if (boost::filesystem::exists(path)) {
                        return boost::filesystem::absolute(path).string();
                    }
                }
                throw boost::filesystem::filesystem_error(
                    "File not found in path",
                    filename,
                    path,
                    boost::system::errc::make_error_code(
                        boost::system::errc::no_such_file_or_directory));
            }

        } // namespace detail
    } // namespace onlinedb
} // namespace gem
