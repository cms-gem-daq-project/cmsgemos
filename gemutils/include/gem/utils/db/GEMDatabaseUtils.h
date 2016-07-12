#ifndef GEM_UTILS_DB_GEMDATABASEUTILS_H
#define GEM_UTILS_DB_GEMDATABASEUTILS_H

#include "Python.h"
#include <mysql/mysql.h>
#include <string>
#include <memory>

#include "log4cplus/logger.h"

#include "toolbox/string.h"

namespace gem {
  namespace utils {
    namespace db {

      class GEMDatabaseUtils
      {

      public:
        GEMDatabaseUtils(std::string const& host, int const& port,
                         std::string const& user, std::string const& password);
        ~GEMDatabaseUtils();

        bool connect(std::string const& database);

        void disconnect();

        void command(const std::string& command);

        void configure();

        unsigned int query(const std::string& query);

      protected:

      private:
        log4cplus::Logger m_gemLogger;

        //std::shared_ptr<MYSQL> p_db;
        MYSQL *p_db;

        std::string m_host, m_user, m_password;
        int m_port;
      };
    }  // end namespace gem::utils::db
  }  // end namespace gem::utils
}  // end namespace gem

#endif  // GEM_UTILS_DB_GEMDATABASEUTILS_H
