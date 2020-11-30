#ifndef __SIMCAN_2_0_HARDWAREMANAGER_H_
#define __SIMCAN_2_0_HARDWAREMANAGER_H_

#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class HardwareManager : public cSimpleModule{

    public:

        int getAvailableCores();

        unsigned int* allocateCores(int numCores);
        int deallocateCores(int numCores);

    private:

       /** Indicates if this module is able to virtualize hardware */
       bool isVirtualHardware;

       /** Number of CPU cores managed by this module */
       unsigned int numCpuCores;

       unsigned int numAvailableCpuCores;

       /** Maximum number of VMs executed in this computer */
       unsigned int maxVMs;

       /** Maximum number of users executed in this computer */
       unsigned int maxUsers;

       bool *freeCoresArrayPtr;






//
//                               /** Array to assign CPU scheduler to different users */
//                               bool *cpuSchedulerUtilization;
//
//       /** List of users currently executing apps in this computer */
//       UserExecution** userVector;






  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
