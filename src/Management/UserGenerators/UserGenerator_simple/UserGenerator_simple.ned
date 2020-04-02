package simcan2.Management.UserGenerators.UserGenerator_simple;
import simcan2.Management.UserGenerators.UserGeneratorBase.UserGeneratorBase;

//
// Implementation of a UserGenerator for cloud environments
//
simple UserGenerator_simple extends UserGeneratorBase{
    
     parameters: 
        double maxStartTime_t1 = default (5.0);
		double nRentTime_t2 = default (5.0);
		double maxSubTime_t3 = default (5.0);
		double maxSubscriptionTime_t4 = default (5.0);
             	
    @class(UserGenerator_simple);	
}
