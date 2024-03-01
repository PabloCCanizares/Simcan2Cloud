#ifndef SIMCAN_EX_DC_HYPERVISOR
#define SIMCAN_EX_DC_HYPERVISOR

#include "OperatingSystem/Hypervisors/common.h"
#include "OperatingSystem/Hypervisors/Hypervisor/Hypervisor.h"
#include "OperatingSystem/CpuSchedulers/CpuSchedulerRR/CpuSchedulerRR.h"

class DcResourceManager; // Forward declaration
namespace hypervisor
{
   class DcHypervisor : public Hypervisor
   {
   private:
      void loadVector(std::vector<cModule *> &v, cModule *osModule, cModule *(*accessor)(cModule *, int));

   protected:
      DcResourceManager *resourceManager; //!< Resource manager of the datacentre
      cMessage *powerMessage;             //!< Power on event
      int nPowerOnTime;                   //!< Time to power on (in seconds)
      std::vector<cModule *> schedulers;  //!< Vector that contains the managed schedulers

      void setActive(bool active) { par("active").setBoolValue(active); }

      virtual void initialize(int stage) override;
      virtual int numInitStages() const override { return BLADE + 1; }
      virtual void finish() override;
      virtual void processSelfMessage(cMessage *msg) override;
      virtual void processResponseMessage(SIMCAN_Message *sm) override;

   public:
      int getAvailableCores() const { return hardwareManager->getAvailableResources().cores; }
      int getNumCores() const { return hardwareManager->getTotalResources().cores; }
      bool isActive() const { return par("isActive"); }

      cModule *handleVmRequest(const VM_Request &request, const char *userId);
      void deallocateVmResources(const std::string &vmId);
      virtual void handleVmTimeout(VmControlBlock &vm) override;

      // Hypervisor inheritance section
      virtual cModule *getApplicationModule(uint32_t vmId, uint32_t pid) override { return getParentModule()->getSubmodule("appsVectors", vmId)->getSubmodule("appModule", pid); }
      virtual HardwareManager *locateHardwareManager() override { return check_and_cast<HardwareManager *>(getModuleByPath("^.^.hardwareManager")); }
      virtual uint32_t resolveGlobalVmId(const std::string &vmId) override { return getOrDefault(vmIdMap, vmId, UINT32_MAX); }

      /**
       * Check if there are VMs running in the machine.
       * @return True if a VM is running in the machine. False otherwise.
       */
      bool isInUse() { return vmsControl.getAllocatedIds() > 0; }
      void powerOn(bool active);
   };
};

#endif /*SIMCAN_EX_DC_HYPERVISOR*/