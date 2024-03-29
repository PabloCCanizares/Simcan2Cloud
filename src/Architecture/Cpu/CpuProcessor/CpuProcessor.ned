package simcan2.Architecture.Cpu.CpuProcessor;
import simcan2.Core.CheckingHubModule.CheckingHubModule;
import simcan2.Architecture.Cpu.CpuCores.ICpuCore;

//
// Module that contains <i>numCpuCores</i> CPU cores.
// These CPU cores simulates the execution, of the requests sent by the applications, to perform computing.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2016-05-01
module CpuProcessor
{
    parameters:
        int numCpuCores;							// Number of CPU cores
        string cpuCoreType;							// Type of the CPU cores
		bool staticAppAssignment;					// True if the assignment of application is static or False in another case
        @display("bgb=499,209");
    gates:
        input fromOs;					// Input gate from the Operating System
        output toOs;  					// Output gate to the Operating System

    submodules:
        cpuCore[numCpuCores]: <cpuCoreType> like ICpuCore {		// CPU cores        
            @display("p=51,71,r,120;i=device/cpu");
        }
        cpuHub: CheckingHubModule {       								// Hub that manages incoming messages 

            parameters:
                staticAppAssignment = staticAppAssignment;
                @display("p=209,156;i=device/hub");
        }

    connections:

        // Connections between the hub and the CPU processor gate
        cpuHub.toInput++ --> ned.IdealChannel --> toOs;
        cpuHub.fromInput++ <-- ned.IdealChannel <-- fromOs;

        // Connections between the Cpu cores and the hub
        for i=0..numCpuCores-1 {
            cpuHub.toOutput++ --> ned.IdealChannel --> cpuCore[i].in;
            cpuHub.fromOutput++ <-- ned.IdealChannel <-- cpuCore[i].out;
        }
}
