package simcan2.simulations.ScenarioTest; 

import simcan2.Architecture.Nodes.Cloud.CloudBlade;
import simcan2.Architecture.Nodes.Cloud.CloudBoard;
import simcan2.Architecture.Nodes.Cloud.CloudRack;
import simcan2.Management.DataCenterManager.DataCenterManager;
import simcan2.Management.CloudProviders.*.*;
import simcan2.Management.CloudProviders.*.*.*;
import simcan2.Architecture.Network.DataCenterNetwork.*;
import simcan2.Management.UserGenerators.UserGenerator_simple.*;
import simcan2.Management.UserGenerators.UserGeneratorBase.*;

// ---------------------------------------------------------------
//   Definition of the data-center AdrianDataCenter_100
// ---------------------------------------------------------------
module AdrianDataCenter_100{

	parameters:
		string appList;
		string vmList;
		string userList;

	gates:
		input fromCloudProvider;
		output toCloudProvider;

	// -------------------------------------------------------
	//   Definition of main modules
	// -------------------------------------------------------
	submodules:

		// ---------------------------------------------------------------
		//   Definition of the data-center manager
		// ---------------------------------------------------------------
		dcManager:DataCenterManager{
			parameters:
				appList=appList;
				vmList=vmList;
				userList=userList;
		}

		// ---------------------------------------------------------------
		//   Definition of Computing Racks
		// ---------------------------------------------------------------
		rackCmp_Rack_A_000:CloudRack;

		// ---------------------------------------------------------------
		//   Definition of Network
		// ---------------------------------------------------------------
		dataCenterNetwork:DataCenterNetwork{
			gates:
				inComm[1];
				outComm[1];
				inStorage[0];
				inStorage[0];
		}

	// ---------------------------------------------------------------
	//   Connections section
	// ---------------------------------------------------------------
	connections allowunconnected:

		// Connections between Data-Center manager and Data-center outside
		dcManager.out --> ned.IdealChannel --> toCloudProvider;
		dcManager.in <-- ned.IdealChannel <-- fromCloudProvider;

		// Connections between computing racks and the network
		rackCmp_Rack_A_000.out --> DataCenterEth1G_channel --> dataCenterNetwork.inComm++;
		rackCmp_Rack_A_000.in <-- DataCenterEth1G_channel <-- dataCenterNetwork.outComm++;

}

// ---------------------------------------------------------------
//   Definition of the scenario
// ---------------------------------------------------------------
network ScenarioTest{

	parameters:
		string appList;
		string vmList;
		string userList;
		string dataCentersList;

	// -------------------------------------------------------
	//   Definition of main modules
	// -------------------------------------------------------
	submodules:

		// ---------------------------------------------------------------
		//   Definition of the data-centers
		// ---------------------------------------------------------------
		dc_AdrianDataCenter_100_000:AdrianDataCenter_100{
			parameters:
				appList=appList;
				vmList=vmList;
				userList=userList;
		}

		// -------------------------------------------------------
		//   Definition of the cloud provider
		// -------------------------------------------------------
		cloudProvider:CloudProvider_firstBestFit{
			parameters:
				appList=appList;
				vmList=vmList;
				userList=userList;
				dataCentersList=dataCentersList;

			gates:
				fromDataCenter[1];
				toDataCenter[1];
		}

		// -------------------------------------------------------
		//   Definition of the user generator
		// -------------------------------------------------------
		userGenerator:UserGenerator_simple{
			parameters:
				appList=appList;
				vmList=vmList;
				userList=userList;
		}

	// ---------------------------------------------------------------
	//   Connections section
	// ---------------------------------------------------------------
	connections allowunconnected:

		// Connections between the cloud provider and each data-center manager
		cloudProvider.toDataCenter++ --> ned.IdealChannel --> dc_AdrianDataCenter_100_000.fromCloudProvider;
		cloudProvider.fromDataCenter++ <-- ned.IdealChannel <-- dc_AdrianDataCenter_100_000.toCloudProvider;

		cloudProvider.toUserGenerator --> ned.IdealChannel --> userGenerator.fromCloudProvider;
		userGenerator.toCloudProvider --> ned.IdealChannel --> cloudProvider.fromUserGenerator;

}
