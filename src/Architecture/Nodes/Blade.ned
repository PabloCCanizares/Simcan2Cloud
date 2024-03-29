package simcan2.Architecture.Nodes;
import simcan2.Architecture.Nodes.HardwareManagers.HardwareManager.HardwareManager;
import simcan2.Architecture.Nodes.HardwareManagers.IHardwareManager;
import simcan2.Applications.Base.AppsVector;
import simcan2.OperatingSystem.OperatingSystem;
import simcan2.Architecture.Cpu.CpuProcessor.CpuProcessor;

//
// Module that represents a blade. Generally, this module is used in racks, 
// where a set of blades are allocated in a node board and connected to the same switch
//
//	- isVirtualHardware = true?
//		+ numVMSs = 1..numCpuCores
//		+ maxUsers = numVMs
//		
//	- isVirtualHardware = false?
//		+ numVMSs = 1
//		+ maxUsers = 1..N
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2016-05-01
module Blade
{
    parameters:
        @labels(node,ethernet-node);

        int numApps = default(5);									// Number of applications in this vector
        bool staticAppAssignment;									// True if the assignment of applications is static or False in another case
        bool isVirtualHardware;										// True if this blade allows virtualization and False in another case
        int maxUsers;												// Maximum number of users allowed to launch applications (For cloud environments, maxUsers=maxVMs)
        int maxVMs;													// Maximum number of VMs allocated in this blade (For non-cloud environments, use maxVMs=1)
        int numCpuCores;											// Number of CPU cores  

		string hardwareManagerType = default("HardwareManager");	// Hardware manager type
        string hypervisorType = default("Hypervisor");				// Hypervisor type
        string cpuSchedulerType = default("CpuSchedulerRR");		// CPU Scheduler type
        string cpuCoreType = default("CpuCore");					// Type of the CPU cores

        @display("bgb=548,431,,black,3;bgi=circuit,t");
        
        
    submodules:

        cpu: CpuProcessor {
            parameters:
                numCpuCores = numCpuCores;
                cpuCoreType = cpuCoreType;
                staticAppAssignment = staticAppAssignment;
                @display("p=274,40;i=cpu");
        }

        hardwareManager: <hardwareManagerType> like IHardwareManager {
            parameters:
                isVirtualHardware = isVirtualHardware;
                maxVMs = maxVMs;
                maxUsers = maxUsers;
                numCpuCores = numCpuCores;
                @display("p=76,88;i=hardwareConfig");
        }

        osModule: OperatingSystem {
            parameters:
                staticAppAssignment = staticAppAssignment;
                isVirtualHardware = isVirtualHardware;
                maxVMs = maxVMs;
                numCpuCores = numCpuCores;
                hypervisorType = hypervisorType;
                cpuSchedulerType = cpuSchedulerType;
                @display("p=274,189;i=os");

            gates:
                fromApps[maxUsers];
                toApps[maxUsers];
        }

        appsVectors[maxUsers]: AppsVector {
            parameters:
                numApps = numApps;
                staticAppAssignment = staticAppAssignment;
                @display("p=83,373,r,120;i=user");
        }


    connections:

        // Connections between Apps and Operating System
        for i=0..maxUsers-1 {
            appsVectors[i].toOS --> ned.IdealChannel --> osModule.fromApps[i];
            appsVectors[i].fromOS <-- ned.IdealChannel <-- osModule.toApps[i];
        }

        // Connections between Operating System and CPU
        osModule.toCpu --> ned.IdealChannel --> cpu.fromOs;
        osModule.fromCpu <-- ned.IdealChannel <-- cpu.toOs;
}

