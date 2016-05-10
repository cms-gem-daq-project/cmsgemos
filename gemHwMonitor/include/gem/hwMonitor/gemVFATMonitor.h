#ifndef GEM_HWMONITOR_GEMHWMONITORWEB_H
#define GEM_HWMONITOR_GEMHWMONITORWEB_H

namespace gem {
  namespace hw {
    namespace vfat {
      class HwVFAT2;
      class VFAT2Settings;
    }
  }
  namespace hwMonitor {
    class gemVFATMonitor: public xdaq::WebApplication
      {
      public:
      private:
        gem::hw::vfat::HwVFAT2* vfatDevice_;
        void printVFAThwParameters(const char* key, const char* value1, const char* value2, xgi::Output* out)
          throw (xgi::exception::Exception);
        void printVFAThwParameters(const char* key, const char* value,  xgi::Output *out)
          throw (xgi::exception::Exception);
        void printVFAThwParameters(const char* key, uint8_t value,  xgi::Output *out)
          throw (xgi::exception::Exception);
        void printVFAThwParameters(const char* key, const char* value1, uint8_t value2, xgi::Output* out)
          throw (xgi::exception::Exception);
      };  // class gemVFATMonitor
  }  // namespace gem::hwMonitor
}  // namespace gem

#endif  // GEM_HWMONITOR_GEMHWMONITORWEB_H
