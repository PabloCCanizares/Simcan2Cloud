package simcan2.Management.CloudManagerBase;
import simcan2.Management.CloudManagerBase.CloudManagerBase;
import simcan2.Core.cSIMCAN_Core;

//
// Base module for Cloud Managers. This module parses and manages VMs and users.
//
simple CloudManagerBase extends cSIMCAN_Core{
    
    parameters:      	
        bool showUsersVms = default (true);		// Show parsed Users and VMs? 
        bool showApps = default (true);			// Show the parsed applications?   
        bool showSlas = default (true);			// Show the parsed slas?  
        string vmList;							// List of VMs							         
        string userList;						// List of users                
        string appList;							// List of applications    	
        string slaList;							// List of slas  	
} 