package simcan2.Management.UserGenerators.UserGenerator_simple;
import simcan2.Management.UserGenerators.UserGeneratorBase.UserGeneratorBase;

//
// Implementation of a UserGenerator for cloud environments
//
simple UserGenerator_simple extends UserGeneratorBase{
    
     parameters:
        @signal[request](type=long);
        @signal[response](type=long);
        @signal[executeIp](type=long);
        @signal[executeNot](type=long);
        @signal[ok](type=long);
        @signal[fail](type=long);
        @signal[subscribeNoip](type=long);
        @signal[subscribeFail](type=long);
        @signal[notify](type=long);
        @signal[timeOut](type=long);
        @statistic[request](record=vector);
        @statistic[response](record=vector);
        @statistic[executeIp](record=vector);
        @statistic[executeNot](record=vector);
        @statistic[ok](record=vector);
        @statistic[fail](record=vector);
        @statistic[subscribeNoip](record=vector);
        @statistic[subscribeFail](record=vector);
        @statistic[notify](record=vector);
        @statistic[timeOut](record=vector);
        double maxStartTime_t1 = default (5.0);
		double nRentTime_t2 = default (5.0);
		double maxSubTime_t3 = default (5.0);
		double maxSubscriptionTime_t4 = default (5.0);
             	
    @class(UserGenerator_simple);	
}
