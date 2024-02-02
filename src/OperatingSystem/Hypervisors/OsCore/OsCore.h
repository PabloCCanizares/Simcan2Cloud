#ifndef SIMCAN_EX_OS_CORE
#define SIMCAN_EX_OS_CORE

#include "OperatingSystem/Hypervisors/common.h"

namespace hypervisor
{
    // Forward declaration
    class Hypervisor;

    class OsCore
    {
    private:
        DataManager *dataManager;         // The global data manager
        HardwareManager *hardwareManager; // The hardware manager of the node
        Hypervisor *hypervisor;       // The hypervisor module
        ApplicationBuilder appBuilder;    // The application builder (check maybe allowing inheritance?)

    public:
        OsCore() { this->hypervisor = nullptr; }
        void setUp(Hypervisor *h, DataManager *dm, HardwareManager *hm);
        void processSyscall(SM_Syscall *sm);

        void launchApps(SM_UserAPP *request);
        void handleAppTermination(AppControlBlock &app, bool force);
        void handleIOFinish(AppControlBlock &app);
        // void handleSendRequest(AppControlBlock &app, bool completed);
        // void handleBindAndListen(AppControlBlock &app);
    };
};

#endif /* SIMCAN_EX_OS_CORE */