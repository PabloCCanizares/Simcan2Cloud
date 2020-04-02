#ifndef __SIMCAN_2_0_USERGENERATOR_H_
#define __SIMCAN_2_0_USERGENERATOR_H_

#include "Management/UserGenerators/UserGeneratorBase/UserGeneratorBase.h"
#include "Messages/SM_UserVM.h"
#include "Messages/SM_UserAPP.h"
#include "Messages/SM_CloudProvider_Control_m.h"


/**
 * Class that implements a User generator for cloud environments.
 *
 */
class UserGenerator_simple: public UserGeneratorBase{

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
        * Processes a self message.
        *
        * @param msg Received (self) message.
        */
        virtual void processSelfMessage (cMessage *msg);

       /**
        * Processes a request message.
        *
        * @param sm Incoming message.
        */
        virtual void processRequestMessage (SIMCAN_Message *sm);

       /**
        * Processes a response message from an external module.
        *
        * @param sm Incoming message.
        */
        virtual void processResponseMessage (SIMCAN_Message *sm);

        //###############################################
        //API
        /**
         * Returns the next cloud user to be processed
         */
        virtual CloudUserInstance* getNextUser();

        /**
         * Shuffles the arrival time of the generated users
         */
        virtual void generateShuffledUsers ();

        /**
         * Builds the VM request which corresponds with the provided user instance.
         */
        virtual SM_UserVM* createVmRequest(CloudUserInstance* pUserInstance);

        /**
         * Builds an App request given a user
         */
        virtual SM_UserAPP* createAppRequest(SM_UserVM* userVm);

        /**
         * Handles the VM response received from the CloudProvider
         * @param userVm incoming message
         */
        virtual void handleUserVmResponse(SM_UserVM* userVm);

        /**
         * Handles the App response sent from the CloudProvider
         * @param userApp incoming message
         */
        virtual void handleUserAppResponse(SM_UserAPP* userApp);

        /**
         * Sends a request of service submission
         */
        void submitService(SM_UserVM* userVm);

        /**
         * Sends a subscribe message to the cloudprovider
         * @param userVm
         */
        void subscribe(SM_UserVM* userVm);

        /**
         * Updates the status of a user
         */
        virtual void updateVmUserStatus(SM_UserVM* userVm);

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
        void recoverVmAndsubscribe(SM_UserAPP* userApp);

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
        void cancelAndDeleteMessages(CloudUserInstance* pUserInstance);
};

#endif
