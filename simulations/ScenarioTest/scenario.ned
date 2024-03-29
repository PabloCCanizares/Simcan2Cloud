package simcan2.simulations.ScenarioTest;

import simcan2.Architecture.Nodes.Cloud.CloudBlade;
import simcan2.Architecture.Nodes.Cloud.CloudBoard;
import simcan2.Architecture.Nodes.Cloud.CloudRack;
import simcan2.Management.DataCentreManagers.DataCentreManagerFirstFit.DataCentreManagerFirstFit;
import simcan2.Management.CloudProviders.*.*;
import simcan2.Architecture.Network.DataCentreNetwork.*;
import simcan2.Management.UserGenerators.*.*;

// ---------------------------------------------------------------
//   Definition of the data-centre DataCentre_A
// ---------------------------------------------------------------
module DataCentre_A
{
    parameters:
        string appList;
        string vmList;
        string slaList;
        string userList;
		int numCmpRacksA;
		int numCmpRacksB;
		int numStoRacks = 0;

        @display("bgb=631,231");
    gates:
        input fromCloudProvider;
        output toCloudProvider;

    // -------------------------------------------------------
    //   Definition of main modules
    // -------------------------------------------------------
    submodules:

        // ---------------------------------------------------------------
        //   Definition of the data-centre manager
        // ---------------------------------------------------------------
        dcManager: DataCentreManagerFirstFit {
            parameters:
                appList = appList;
                vmList = vmList;
                slaList = slaList;
                userList = userList;
                @display("p=193,148");
        }

        // ---------------------------------------------------------------
        //   Definition of Computing Racks
        // ---------------------------------------------------------------
        rackCmp_Rack_A[numCmpRacksA]: CloudRack {
            @display("p=109,34");
        }
        
        rackCmp_Rack_B[numCmpRacksB]: CloudRack {
        }
        
                // ---------------------------------------------------------------
        //   Definition of Computing Racks
        // ---------------------------------------------------------------
        rackSto_Rack_B[numStoRacks]: CloudRack {
            @display("p=109,34");
        }
        

        // ---------------------------------------------------------------
        //   Definition of Network
        // ---------------------------------------------------------------
        dataCentreNetwork: DataCentreNetwork {
            @display("p=299,51");
            gates:
                inComm[numCmpRacksA+numCmpRacksB];
                outComm[numCmpRacksA+numCmpRacksB];
                inStorage[numStoRacks];
                outStorage[numStoRacks];
        }

    // ---------------------------------------------------------------
    //   Connections section
    // ---------------------------------------------------------------
    connections allowunconnected:

        // Connections between Data-Centre manager and Data-centre outside
        dcManager.out --> ned.IdealChannel --> toCloudProvider;
        dcManager.in <-- ned.IdealChannel <-- fromCloudProvider;

        // Connections between computing racks and the network
        for i=0..numCmpRacksA-1 {
        	rackCmp_Rack_A[i].out --> DataCentreEth40G_channel --> dataCentreNetwork.inComm++;
        	rackCmp_Rack_A[i].in <-- DataCentreEth40G_channel <-- dataCentreNetwork.outComm++;
        }
        
        for i=0..numCmpRacksB-1 {
        	rackCmp_Rack_B[i].out --> DataCentreEth40G_channel --> dataCentreNetwork.inComm++;
        	rackCmp_Rack_B[i].in <-- DataCentreEth40G_channel <-- dataCentreNetwork.outComm++;
        }
        
        // Connections between storage racks and the network
        for i=0..numStoRacks-1 {
        	rackSto_Rack_B[i].out --> DataCentreEth40G_channel --> dataCentreNetwork.inStorage++;
        	rackSto_Rack_B[i].in <-- DataCentreEth40G_channel <-- dataCentreNetwork.outStorage++;
        }
}

// ---------------------------------------------------------------
//   Definition of the scenario
// ---------------------------------------------------------------
network ScenarioTest
{
    parameters:
        string appList;
        string vmList;
        string slaList;
        string userList;
        string dataCentresList;
        int numDataCentres = 1;

        @display("bgb=610,278");
    // -------------------------------------------------------
    //   Definition of main modules
    // -------------------------------------------------------
    submodules:

        // ---------------------------------------------------------------
        //   Definition of the data-centres
        // ---------------------------------------------------------------
        dc_DataCentre[numDataCentres]: DataCentre_A {
            parameters:
                appList = appList;
                vmList = vmList;
                slaList = slaList;
                userList = userList;
                @display("p=379,131");
        }

        // -------------------------------------------------------
        //   Definition of the cloud provider
        // -------------------------------------------------------
        cloudProvider: CloudProviderFirstFit {
            parameters:
                appList = appList;
                vmList = vmList;
                slaList = slaList;
                userList = userList;
                dataCentresList = dataCentresList;

                @display("p=240,76");
            gates:
                fromDataCentre[numDataCentres];
                toDataCentre[numDataCentres];
        }

        // -------------------------------------------------------
        //   Definition of the user generator
        // -------------------------------------------------------
        userGenerator: UserGenerator_simple {
            parameters:
                appList = appList;
                vmList = vmList;
                slaList = slaList;
                userList = userList;
                @display("p=60,125");
        }

    // ---------------------------------------------------------------
    //   Connections section
    // ---------------------------------------------------------------
    connections allowunconnected:

        // Connections between the cloud provider and each data-centre manager
        for i=0..numDataCentres-1 {
	        cloudProvider.toDataCentre++ --> ned.IdealChannel --> dc_DataCentre[i].fromCloudProvider;
        	cloudProvider.fromDataCentre++ <-- ned.IdealChannel <-- dc_DataCentre[i].toCloudProvider;
        }


        cloudProvider.toUserGenerator --> ned.IdealChannel --> userGenerator.fromCloudProvider;
        userGenerator.toCloudProvider --> ned.IdealChannel --> cloudProvider.fromUserGenerator;

}
