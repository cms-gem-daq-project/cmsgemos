#ifndef GEM_HW_AMC13_MODULE_H
#define GEM_HW_AMC13_MODULE_H
//
// class to keep track of AMC13 modules for command-line tool
// (possibly useful for other purposes)
//
 
#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "amc13/AMC13.hh"

namespace amc13{
  
  class Module {
    
  public:
    Module();
    ~Module();
    void Connect( const std::string file, const std::string addressTablePath, const std::string prefix = "");
    std::string Show();
    
    AMC13* getAMC13(){return amc13;}
    AMC13* amc13;
    std::string connectionFile;
    uint32_t serialNo;
    uint32_t revT1;
    uint32_t revT2;

    std::ofstream& getStream(); 
    bool isFileOpen() { return stream != NULL;}
    void setStream(const char* file);
    void closeStream();
    std::string getFileName() { return fileName; }
    void hostnameToIp(const char *hostname, char *ip);

  private :
    std::ofstream* stream;
    std::string fileName;
  };
}

#endif  // GEM_HW_AMC13_MODULE_H
