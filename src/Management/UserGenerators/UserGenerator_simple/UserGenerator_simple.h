#ifndef __SIMCAN_2_0_USERGENERATOR_H_
#define __SIMCAN_2_0_USERGENERATOR_H_

#include "Management/UserGenerators/UserGeneratorBase/UserGeneratorBase.h"
#include "Messages/SM_UserVM.h"
#include "Messages/SM_UserAPP.h"
#include "Messages/SM_CloudProvider_Control_m.h"
#include <algorithm>
#include <random>
#include <functional>

/**
 * Class that implements a User generator for cloud environments.
 *
 */
class UserGenerator_simple: public UserGeneratorBase {

protected:
    //Timeouts active
    bool bMaxStartTime_t1_active;

    //TODO: Delete
    double m_dInitSim;
    int m_nUsersSent;

    // Timeouts
    double maxStartTime_t1;
    double nRentTime_t2;
    double maxSubTime_t3;
    double maxSubscriptionTime_t4;

    // Handlers hashMap
    std::map<std::string, std::function<void(cMessage*)>> selfMessageHandlers;
    std::map<std::string, std::function<void(cMessage*)>> requestHandlers;
    std::map<int, std::function<void(SIMCAN_Message*)>> responseHandlers;

    // Signals
    simsignal_t requestSignal;
    simsignal_t responseSignal;
    simsignal_t executeIpSignal;
    simsignal_t executeNotSignal;
    simsignal_t okSignal;
    simsignal_t failSignal;
    simsignal_t subscribeNoipSignal;
    simsignal_t subscribeFailSignal;
    simsignal_t notifySignal;
    simsignal_t timeOutSignal;


    /** Iterators */
    /**
     * Destructor
     */
    ~UserGenerator_simple();

    /**
     * Initialize method. Invokes the parsing process to allocate the existing cloud users in the corresponding data structures.
     */
    virtual void initialize();

    /**
     * Initializes the signals.
     */
    virtual void initializeSignals();

    /**
     * Initializes the self message handlers.
     */
    virtual void initializeSelfHandlers();

    /**
     * Initializes the response handlers.
     */
    virtual void initializeResponseHandlers();

    /**
     * Processes a self message.
     *
     * @param msg Received (self) message.
     */
    virtual void processSelfMessage(cMessage *msg);

    /**
     * Returns the time the next user will arrive to the cloud
     *
     * @param init Simulation start time.
     * @param last Last user arrival time.
     */
    virtual double getNextTime(double init, double last);

    /**
     * Processes a self message of type WaitToExecute.
     *
     * @param msg Received (WaitToExecute) message.
     */
    virtual void handleWaitToExecuteMessage(cMessage *msg);

    /**
     * Processes a self message of type USER_GEN_MSG.
     *
     * @param msg Received (USER_REQ_GEN_MSG) message.
     */
    virtual void handleUserReqGenMessage(cMessage *msg);

    /**
     * Schedules the next user generation by scheduling an USER_GEN_MSG.
     *
     */
    virtual void scheduleNextReqGenMessage();
    /**
     * Processes a request message.
     *
     * @param sm Incoming message.
     */
    virtual void processRequestMessage(SIMCAN_Message *sm);

    /**
     * Processes a response message from an external module.
     *
     * @param sm Incoming message.
     */
    virtual void processResponseMessage(SIMCAN_Message *sm);

    //###############################################
    //API
    /**
     * Returns the next cloud user to be processed
     */
    virtual CloudUserInstance* getNextUser();

    /**
     * Shuffles the arrival time of the generated users
     */
    virtual void generateShuffledUsers();

    /**
     * Builds the VM request which corresponds with the provided user instance.
     */
    virtual SM_UserVM* createVmRequest(CloudUserInstance *pUserInstance);

    /**
     * Builds an App request given a user
     */
    virtual SM_UserAPP* createAppRequest(SM_UserVM *userVm);

    /**
     * Handles the VM response received from the CloudProvider
     * @param userVm incoming message
     */
    virtual void handleResponse(SIMCAN_Message *userVm);
    virtual void handleAppOk(SIMCAN_Message *userVm);
    virtual void handleAppTimeout(SIMCAN_Message *userVm);
    virtual void handleSubNotify(SIMCAN_Message *userVm);
    virtual void handleSubTimeout(SIMCAN_Message *userVm);

    /**
     * Handles the App response sent from the CloudProvider
     * @param userApp incoming message
     */
    //virtual void handleUserAppResponse(SIMCAN_Message *userApp_RAW);
    //virtual void handleAppResAccept(SM_UserAPP *userApp);
    //virtual void handleAppResReject(SM_UserAPP *userApp);
    //virtual void handleAppResTimeout(SM_UserAPP *userApp);

    virtual bool hasToSubscribeVm(SM_UserAPP *userApp);

    /**
     * Sends a request of service submission
     */
    void submitService(SM_UserVM *userVm);

    /**
     * Sends a subscribe message to the cloudprovider
     * @param userVm
     */
    void subscribe(SM_UserVM *userVm);

    /**
     * Updates the status of a user
     */
    virtual void updateVmUserStatus(SM_UserVM *userVm);

    inline static bool compareArrivalTime(CloudUserInstance *a, CloudUserInstance *b) {
        return a->getArrival2Cloud() < b->getArrival2Cloud();
    }

private:

    /**
     * This method generates a vm request filled with pre-defined data. Designed for initial debugging
     *
     * @return Object that represents a VM request
     */
    SM_UserVM* createFakeVmRequest();

    /**
     * Recovers a VM given a user name, and sends a subscribe message to the cloudmanager
     * @param userApp
     */
    void recoverVmAndsubscribe(SM_UserAPP *userApp);

    /**
     *  Prints the final parameters.
     */
    void printFinal();

    /**
     * Print the results obtained during the first phase of the experiments.
     */
    void printExperiments_phase1();

    /**
     * Checks if all the users have finished.
     * @return
     */
    bool allUsersFinished();

    /**
     * Calculates the statistics of the experiment.
     */
    virtual void calculateStatistics();

    /**
     * Cancels and deletes all the messages corresponding with an specific user instance
     * @param pUserInstance User instance
     */
    void cancelAndDeleteMessages(CloudUserInstance *pUserInstance);
};

#endif
