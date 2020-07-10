package simcan2.Management.UserGenerators.UserGenerator_simple;
import simcan2.Management.UserGenerators.UserGeneratorBase.UserGeneratorBase;

//
// Implementation of a UserGenerator for cloud environments
//
simple UserGenerator_simple extends UserGeneratorBase{
    
     parameters:
        @signal[request](type=long);
        @signal[response](type=long);
        @signal[executeRequest](type=long);
        @signal[executeNotify](type=long);
        @signal[appOK](type=long);
        @signal[appTimeout](type=long);
        @signal[subscribeRequest](type=long);
        @signal[subscribeFailure](type=long);
        @signal[notify](type=long);
        @signal[timeout](type=long);
        @statistic[request](record=vector);
        @statistic[response](record=vector);
        @statistic[executeRequest](record=vector);
        @statistic[executeNotify](record=vector);
        @statistic[appOK](record=vector);
        @statistic[appTimeout](record=vector);
        @statistic[subscribeRequest](record=vector);
        @statistic[subscribeFailure](record=vector);
        @statistic[notify](record=vector);
        @statistic[timeout](record=vector);
        double maxStartTime_t1 = default (5.0);
		double nRentTime_t2 = default (5.0);
		double maxSubTime_t3 = default (5.0);
		double maxSubscriptionTime_t4 = default (5.0);
    
    @class(UserGenerator_simple);	
}
