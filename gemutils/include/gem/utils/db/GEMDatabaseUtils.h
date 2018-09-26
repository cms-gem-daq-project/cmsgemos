#ifndef GEM_UTILS_DB_GEMDATABASEUTILS_H
#define GEM_UTILS_DB_GEMDATABASEUTILS_H

#include "Python.h"
#include <mysql/mysql.h>
#include <string>
#include <memory>

#include "xdata/Bag.h"
#include "xdata/Integer.h"
#include "xdata/String.h"

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

        // hacky hack mc hackerton
        void configure(const std::string& station="CERN904",
                       const std::string& setuptype="teststand",
                       const std::string& runperiod="2016T",
                       const int& runnumber=-1);

        unsigned int query(const std::string& query);

        class GEMDBInfo {

        public:
          GEMDBInfo();
          void registerFields(xdata::Bag<GEMDatabaseUtils::GEMDBInfo>* bag);

          xdata::String   dbName;
          xdata::String   dbHost;
          xdata::Integer  dbPort;
          xdata::String   dbUser;
          xdata::String   dbPass;
          xdata::String   setupTag;
          xdata::String   runPeriod;
          xdata::String   setupLocation;

          inline std::string toString() {
            std::stringstream os;
            os << "dbName:" << dbName.toString() << std::endl
               << "dbHost:" << dbHost.toString() << std::endl
               << "dbPort:" << dbPort.toString() << std::endl
               << "dbUser:" << dbUser.toString() << std::endl
               << "dbPass:" << dbPass.toString() << std::endl

               << "setupTag:"      << setupTag.toString()      << std::endl
               << "runPeriod:"     << runPeriod.toString()     << std::endl
               << "setupLocation:" << setupLocation.toString() << std::endl
               << std::endl;
            return os.str();
          }
        };

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
