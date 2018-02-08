//#include <Python.h>

#include <gem/utils/db/GEMDatabaseUtils.h>

#include <gem/utils/GEMLogging.h>

#include "gem/utils/exception/Exception.h"

gem::utils::db::GEMDatabaseUtils::GEMDBInfo::GEMDBInfo()
{
  dbName = "";
  dbHost = "";
  dbPort = 3306;
  dbUser = "";
  dbPass = "";
  setupTag      = "";
  runPeriod     = "";
  setupLocation = "";
}

void gem::utils::db::GEMDatabaseUtils::GEMDBInfo::registerFields(xdata::Bag<gem::utils::db::GEMDatabaseUtils::GEMDBInfo>* bag)
{
  bag->addField("dbName",        &dbName);
  bag->addField("dbHost",        &dbHost);
  bag->addField("dbPort",        &dbPort);
  bag->addField("dbUser",        &dbUser);
  bag->addField("dbPass",        &dbPass);
  bag->addField("setupTag",      &setupTag);
  bag->addField("runPeriod",     &runPeriod);
  bag->addField("setupLocation", &setupLocation);
}

gem::utils::db::GEMDatabaseUtils::GEMDatabaseUtils(std::string const& host, int const& port,
                                                   std::string const& user, std::string const& password) :
  m_gemLogger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("GEMDatabaseUtilsLogger"))),
  p_db(0),
  m_host(host),
  m_user(user),
  m_password(password),
  m_port(port)
{
  //p_db = std::make_shared<MYSQL>(mysql_init(0));
  //m_gemLogger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("GEMDatabaseUtilsLogger"));
  //connect(std::string const& database);
}

gem::utils::db::GEMDatabaseUtils::~GEMDatabaseUtils()
{
  disconnect();
}

bool gem::utils::db::GEMDatabaseUtils::connect(std::string const& database)
{
  //p_db = std::make_shared<MYSQL>(mysql_init(0));
  p_db = mysql_init(0);

  if (mysql_real_connect(p_db,m_host.c_str(),m_user.c_str(),m_password.c_str(),database.c_str(),m_port,0,CLIENT_COMPRESS) == 0) {
    std::string message("Error connecting to database '");
    message += database;
    message += "' : ";
    message += mysql_error(p_db);
    p_db = 0;
    ERROR(message);
    XCEPT_RAISE(gem::utils::exception::DBConnectionError,message);
    // return false;
  }
  return true;
}

void gem::utils::db::GEMDatabaseUtils::disconnect()
{
  if (!p_db)
    return;
  mysql_close(p_db);
  p_db = 0;
}

void gem::utils::db::GEMDatabaseUtils::command(const std::string& command)
{
  int rv = mysql_query(p_db,command.c_str());
  if (rv)
    ERROR("MySQL comand error: " << std::string(mysql_error(p_db)));
  else
    INFO("MySQL command success: " << command);
  MYSQL_RES* res = mysql_use_result(p_db);
  mysql_free_result(res);
}

unsigned int gem::utils::db::GEMDatabaseUtils::query(const std::string& query)
{
  int rv = mysql_query(p_db,query.c_str());
  if (rv)
    ERROR("MySQL query error: " << std::string(mysql_error(p_db)));
  else
    INFO("MySQL query success: " << query);
  MYSQL_RES* res = mysql_use_result(p_db);
  MYSQL_ROW  row = mysql_fetch_row(res);
  if (row == 0) {
    std::string errMsg = "Query result " + query + " empty";
    ERROR("GEMDatabaseUtils::query " << errMsg);
    XCEPT_RAISE(gem::utils::exception::DBEmptyQueryResult, errMsg);
  }

  unsigned int retval = strtoul(row[0],0,10);
  mysql_free_result(res);

  return retval;
}

void gem::utils::db::GEMDatabaseUtils::configure(const std::string& station,
                                                 const std::string& setuptype,
                                                 const std::string& runperiod,
                                                 const int& runnumber)
{
  Py_Initialize();
  std::stringstream cmd;
  cmd << "from gempython.utils.db.query import configure_db" << std::endl;
  cmd << "configure_db(station=\"" << station
      << "\",setuptype=\"" << setuptype
      << "\",runperiod=\"" << runperiod
      << "\",runnumber=\"" << runnumber
      << "\")" << std::endl;

  int retval = PyRun_SimpleString(cmd.str().c_str());

  INFO("GEMDatabaseUtils::configure_db had return value " << retval);

  if (retval) {
    std::string errMsg = "configure_db call failed";
    PyErr_Print();
    ERROR("GEMDatabaseUtils::configure " << errMsg);
    XCEPT_RAISE(gem::utils::exception::DBPythonError, errMsg);
  }

  Py_Finalize();
}
