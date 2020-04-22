#include "UserGenerator_simple.h"
#include <random>
#include <algorithm>

#define USER_GEN_MSG "USER_GEN"

Define_Module (UserGenerator_simple);

UserGenerator_simple::~UserGenerator_simple() {

}
/**
 * This method initialize the structures and methods
 */
void UserGenerator_simple::initialize() {

    // Init super-class
    UserGeneratorBase::initialize();

    bMaxStartTime_t1_active = false;

    maxStartTime_t1 = par("maxStartTime_t1");
    nRentTime_t2 = par("nRentTime_t2");
    maxSubTime_t3 = par("maxSubTime_t3");
    maxSubscriptionTime_t4 = par("maxSubscriptionTime_t4");

    EV_INFO << "UserGenerator::initialize - Base initialized" << endl;

    m_nUsersSent = 0;

    selfFunctions[Timer_WaitToExecute.c_str()] = &UserGenerator_simple::processWaitMessage;
    selfFunctions[USER_GEN_MSG] = &UserGenerator_simple::processUserGenMessage;

    EV_INFO << "UserGenerator::initialize - End" << endl;
}

/**
 * Shuffle the list of users in order to reproduce the behaviour of the users in a real cloud environment.
 */
void UserGenerator_simple::generateShuffledUsers() {

    CloudUserInstance *pUser;
    int nRandom, nSize;

    EV_INFO << "UserGenerator::generateShuffledUsers - Init" << endl;

    nSize = userInstances.size();

    EV_INFO << "UserGenerator::generateShuffledUsers - instances size: "
            << userInstances.size() << endl;
    //
    for (int i = 0; i < nSize; i++) {
        nRandom = rand() % nSize;
        std::iter_swap(userInstances.begin() + i,
                userInstances.begin() + nRandom);
    }

    EV_INFO << "UserGenerator::generateShuffledUsers - End" << endl;
}

/**
 * This method processes the self messages
 * @param msg
 */
void UserGenerator_simple::processSelfMessage(cMessage *msg) {
    std::map<const char*, void (UserGenerator_simple::*) (cMessage*)>::iterator it;

    it = selfFunctions.find(msg->getName());

    if (it == selfFunctions.end()) {
        error("Unknown self message [%s]", msg->getName());
    }
    else {
        (this->*(it->second))(msg);
    }

    delete (msg);
}

void UserGenerator_simple::processWaitMessage(cMessage *msg) {
    SM_UserVM *userVm;
    CloudUserInstance *pUserInstance;
    double lastTime;
    bool useDistribution = false;

    // Log (INFO)
    EV_INFO << "Starting execution!!!" << endl;

    //TODO:
    m_dInitSim = simTime().dbl();
    lastTime = m_dInitSim;

    if (!useDistribution) {
        // srand((int)33); // TODO
        generateShuffledUsers();
        lastTime += std::numeric_limits<double>::epsilon(); // Evita problemas de enviar al pasado, creo

        for (int i = 0; i < userInstances.size(); i++) {
            // Get current user
            pUserInstance = userInstances.at(i);

            if (userVm != nullptr) {
                pUserInstance->setRequestVmMsg(userVm);
                // Set init and arrival time!
                pUserInstance->setInitTime(lastTime);
                pUserInstance->setArrival2Cloud(lastTime);
            }

            lastTime += intervalBetweenUsers->doubleValue();
        }
    }
    else {
        double timeEpsilon = m_dInitSim + std::numeric_limits<double>::epsilon();
        for (int i = 0; i < userInstances.size(); i++) {
            lastTime = timeEpsilon + intervalBetweenUsers->doubleValue(); // Evita problemas de enviar al pasado, creo

            // Get current user
            pUserInstance = userInstances.at(i);
            userVm = createVmRequest(pUserInstance);

            if (userVm != nullptr) {
                pUserInstance->setRequestVmMsg(userVm);
                // Set init and arrival time!
                pUserInstance->setInitTime(lastTime);
                pUserInstance->setArrival2Cloud(lastTime);
            }
        }

        std::sort(userInstances.begin(), userInstances.end(), compareArrivalTime);
    }

    m_nUsersSent = 0;
    pUserInstance = userInstances.at(m_nUsersSent);
    scheduleAt(pUserInstance->getArrival2Cloud(), new cMessage(USER_GEN_MSG));
}

void UserGenerator_simple::processUserGenMessage(cMessage *msg) {
    SM_UserVM *userVm;
    CloudUserInstance *pUserInstance;

    EV_INFO << "processSelfMessage - USER_GEN_MSG" << endl;
    // Get current user
    pUserInstance = userInstances.at(m_nUsersSent);
    userVm = pUserInstance->getRequestVmMsg();

    if (userVm != nullptr) {
        // Send user to cloud provider
        sendRequestMessage(userVm, toCloudProviderGate);

        //TODO:¿Dejamos este mensaje o lo quitamos?
        EV_FATAL << "#___ini#" << m_nUsersSent << " "
                << (simTime().dbl() - m_dInitSim) / 3600 << "   \n"
                << endl;
    }
    m_nUsersSent++;

    if (m_nUsersSent < userInstances.size()) {
        pUserInstance = userInstances.at(m_nUsersSent);
        scheduleAt(pUserInstance->getArrival2Cloud(), new cMessage(USER_GEN_MSG));
    }
}

void UserGenerator_simple::processRequestMessage(SIMCAN_Message *sm) {

    error("This module cannot process request messages:%s",
            sm->contentsToString(true, false).c_str());
}

void UserGenerator_simple::processResponseMessage(SIMCAN_Message *sm) {

    SM_UserVM *userVM_Rq;
    SM_UserAPP *userAPP_Rq;

    EV_INFO << "processResponseMessage - Received Response Message" << endl;

    //Receive response, check type!
    userVM_Rq = dynamic_cast<SM_UserVM*>(sm);
    userAPP_Rq = dynamic_cast<SM_UserAPP*>(sm);

    if (userVM_Rq != nullptr) {
        EV_INFO << "processResponseMessage - handle UserVmResponse" << endl;
        handleUserVmResponse(userVM_Rq);
    } else if (userAPP_Rq != nullptr) {
        EV_INFO << "processResponseMessage - handle UserAppResponse" << endl;
        handleUserAppResponse(userAPP_Rq);
    } else {
        EV_INFO << "processResponseMessage - Unhandled response" << endl;

    }
}

void UserGenerator_simple::handleUserAppResponse(SM_UserAPP *userApp) {
    bool bFinish;
    CloudUserInstance *pUserInstance;
    //Print a debug trace ...
    userApp->printUserAPP();

    EV_INFO << "handleUserAppResponse - Init" << endl;

    if (userApp->getOperation() == SM_APP_Rsp) {
        EV_INFO << "handleUserAppResponse - SM_APP_Rsp" << endl;
        //Check the response
        if (userApp->getResult() == SM_APP_Res_Accept) {
            EV_INFO << "handleUserAppResponse - SM_APP_Res_Accept" << endl;

            //End of the protocol, exit!!
            pUserInstance = userHashMap.at(userApp->getUserID());
            pUserInstance->setEndTime(simTime().dbl());
            pUserInstance->setFinished(true);
            nUserInstancesFinished++;

            cancelAndDeleteMessages(pUserInstance);

            bFinish = allUsersFinished();
            if (bFinish) {
                sendRequestMessage(new SM_CloudProvider_Control(),
                        toCloudProviderGate);
                printFinal();
            }
        } else if (userApp->getResult() == SM_APP_Res_Reject) {
            //The next step is to send a subscription to the cloudprovider
            //Recover the user instance, and get the VmRequest

            recoverVmAndsubscribe(userApp);
        } else if (userApp->getResult() == SM_APP_Res_Timeout) {
            EV_INFO << "handleUserAppResponse - SM_APP_Res_Timeout" << endl;

            //End of the protocol, exit!!
            pUserInstance = userHashMap.at(userApp->getUserID());
            pUserInstance->setEndTime(simTime().dbl());
            pUserInstance->setFinished(true);
            pUserInstance->setTimeoutMaxRentTime();
            nUserInstancesFinished++;

            bFinish = allUsersFinished();
            cancelAndDeleteMessages(pUserInstance);

            if (bFinish) {
                sendRequestMessage(new SM_CloudProvider_Control(),
                        toCloudProviderGate);
                printFinal();
            }
        }
    }
    EV_INFO << "handleUserAppResponse - End" << endl;

}
void UserGenerator_simple::cancelAndDeleteMessages(
        CloudUserInstance *pUserInstance) {
    SM_UserVM *pVmMessage;
    SM_UserVM *pSubscribeVmMessage;
    SM_UserAPP *pAppMessage;

    pVmMessage = pUserInstance->getRequestVmMsg();
    pAppMessage = pUserInstance->getRequestAppMsg();
    pSubscribeVmMessage = pUserInstance->getSubscribeVmMsg();

    if (pVmMessage) {
        cancelAndDelete(pVmMessage);
        pUserInstance->setRequestVmMsg(nullptr);
    }
    if (pAppMessage) {
        cancelAndDelete(pAppMessage);
        pUserInstance->setRequestApp(nullptr);
    }
    if (pSubscribeVmMessage)
        cancelAndDelete(pSubscribeVmMessage);

}
void UserGenerator_simple::recoverVmAndsubscribe(SM_UserAPP *userApp) {
    bool bSent = false;
    SM_UserVM *userVM_Rq;
    std::string strUserId;
    CloudUserInstance *pUserInstance;

    strUserId = userApp->getUserID();

    if (strUserId.size() > 0) {
        EV_INFO
                << "recoverVmAndsubscribe - Subscribing to the cluster with user: "
                << strUserId << endl;
        pUserInstance = userHashMap.at(strUserId);
        if (pUserInstance != nullptr) {
            userVM_Rq = pUserInstance->getRequestVmMsg();
            if (userVM_Rq != nullptr) {
                bSent = true;
                userVM_Rq->setIsResponse(false);
                userVM_Rq->setOperation(SM_VM_Sub);
                sendRequestMessage(userVM_Rq, toCloudProviderGate);
            }
        }
    }
    if (bSent == false) {
        error("Error sending the subscription message");
    }
}

void UserGenerator_simple::handleUserVmResponse(SM_UserVM *userVm) {
    CloudUserInstance *pUserInstance;
    bool bFinish;

    userVm->printUserVM();

    //Update the status
    updateVmUserStatus(userVm);

    //Check the response and proceed with the next action
    switch (userVm->getOperation()) {
    case SM_VM_Req_Rsp:
        //Check the response!!
        if (userVm->getResult() == SM_VM_Res_Accept) {
            EV_INFO << "VM Response Accepted ... submiting" << endl;
            //The user vm-request has been accepted by the cloudprovider
            //The next step is to submit the required services.
            pUserInstance = userHashMap.at(userVm->getUserID());

            if (pUserInstance != nullptr) {
                pUserInstance->setInitExecTime(simTime().dbl());
                pUserInstance->setSubscribe(false);
                submitService(userVm);
            }
        } else if (userVm->getResult() == SM_VM_Res_Reject) {
            EV_INFO << "VM Response Reject ... subscribing" << endl;
            //The user vm-request has been rejected by the cloudprovider!!
            //Is the moment to subscribe the service.
            pUserInstance = userHashMap.at(userVm->getUserID());
            if (pUserInstance != nullptr) {
                pUserInstance->setInitExecTime(simTime().dbl());
                pUserInstance->setSubscribe(true);
                subscribe(userVm);
            }
        }
        break;
    case SM_VM_Notify:
        if (userVm->getResult() == SM_APP_Sub_Accept) {
            EV_INFO << "Subscription accepted ...  " << endl;
            pUserInstance = userHashMap.at(userVm->getUserID());

            if (pUserInstance != nullptr) {
                pUserInstance->setInitExecTime(simTime().dbl());
                submitService(userVm);
            }
        } else if (userVm->getResult() == SM_APP_Sub_Timeout) {
            // End of the party.
            EV_INFO << "VM timeout ... go home" << endl;
            pUserInstance = userHashMap.at(userVm->getUserID());

            if (pUserInstance != nullptr) {
                EV_INFO << "Set itself finished" << endl;
                pUserInstance->setFinished(true);
                pUserInstance->setEndTime(simTime().dbl());
                pUserInstance->setTimeoutMaxSubscribed();

                cancelAndDeleteMessages(pUserInstance);

                //Check if all the users has ended
                bFinish = allUsersFinished();
                if (bFinish) {
                    sendRequestMessage(new SM_CloudProvider_Control(),
                            toCloudProviderGate);
                    printFinal();
                }
            }
        }
        break;
    }
}

void UserGenerator_simple::updateVmUserStatus(SM_UserVM *userVm) {
    CloudUserInstance *pUserInstance;
    VmInstanceCollection *pVmCollection;
    VmInstance *pVmInstance;
    std::string strUserId;
    int nCollectionNumber, nInstances, nVmRqIndex;

    if (userVm != nullptr) {
        nVmRqIndex = 0;
        strUserId = userVm->getUserID();
        pUserInstance = userHashMap.at(strUserId);

        if (pUserInstance != nullptr && userVm != nullptr) {
            nCollectionNumber = pUserInstance->getNumberVmCollections();
            for (int i = 0; i < nCollectionNumber; i++) {
                pVmCollection = pUserInstance->getVmCollection(i);
                if (pVmCollection != nullptr) {
                    nInstances = pVmCollection->getNumInstances();

                    for (int j = 0; j < nInstances; j++) {
                        pVmInstance = pVmCollection->getVmInstance(j);
                        if (pVmInstance != nullptr) {
                            switch (userVm->getResult()) {
                            case SM_VM_Res_Accept:
                                pVmInstance->setState(vmAccepted);
                                break;
                            case SM_VM_Res_Reject:
                                break;
                            }

                        }
                        nVmRqIndex++;
                    }
                }
            }
        }
    }
}
void UserGenerator_simple::subscribe(SM_UserVM *userVM_Rq) {
    EV_INFO << "UserGenerator::subscribe - Sending Subscribe message" << endl;
    if (userVM_Rq != nullptr) {
        userVM_Rq->setIsResponse(false);
        userVM_Rq->setOperation(SM_VM_Sub);
        userVM_Rq->printUserVM();
        sendRequestMessage(userVM_Rq, toCloudProviderGate);
    } else {
        EV_INFO << "Error sending Subscribe message" << endl;

    }
    EV_INFO << "UserGenerator::subscribe - End" << endl;
}
void UserGenerator_simple::submitService(SM_UserVM *userVm) {
    SM_UserAPP *pAppRq;

    pAppRq = createAppRequest(userVm);

    if (pAppRq != nullptr)
        sendRequestMessage(pAppRq, toCloudProviderGate);
}

CloudUserInstance* UserGenerator_simple::getNextUser() {
    CloudUserInstance *pUserInstance;

    pUserInstance = nullptr;

    if (nUserIndex < userInstances.size()) {
        pUserInstance = userInstances.at(nUserIndex);
        EV_INFO
                << "UserGenerator::getNextUser - The next user to be processed is "
                << pUserInstance->getUserID() << " [" << nUserIndex << " of "
                << userInstances.size() << "]" << endl;
        nUserIndex++;
    } else {
        EV_INFO
                << "UserGenerator::getNextUser - The requested user index is greater than the collection size. ["
                << nUserIndex << " of " << userInstances.size() << "]" << endl;
    }
    return pUserInstance;
}

SM_UserVM* UserGenerator_simple::createVmRequest(
        CloudUserInstance *pUserInstance) {
    int nVmIndex, nCollectionNumber, nInstances;
    double dRentTime;
    std::string userId, vmType, instanceId;
    SM_UserVM *pUserRet;
    VmInstance *pVmInstance;
    VmInstanceCollection *pVmCollection;

    EV_TRACE << "UserGenerator::createNextVmRequest - Init" << endl;

    pUserRet = nullptr;

    nVmIndex = 0;

    if (pUserInstance != nullptr) {
        pUserInstance->setRentTimes(maxStartTime_t1, nRentTime_t2,
                maxSubTime_t3, maxSubscriptionTime_t4);
        nCollectionNumber = pUserInstance->getNumberVmCollections();
        userId = pUserInstance->getUserID();

        EV_TRACE << "UserGenerator::createNextVmRequest - UserId: " << userId
                << " | maxStartTime_t1: " << maxStartTime_t1
                << " | rentTime_t2: " << nRentTime_t2 << " | maxSubTime: "
                << maxSubTime_t3 << " | MaxSubscriptionTime_T4:"
                << maxSubscriptionTime_t4 << endl;

        if (nCollectionNumber > 0 && userId.size() > 0) {
            //Creation of the message
            pUserRet = new SM_UserVM();
            pUserRet->setUserID(userId.c_str());
            pUserRet->setIsResponse(false);
            pUserRet->setOperation(SM_VM_Req);

            //Get all the collections and all the instances!
            for (int i = 0; i < nCollectionNumber; i++) {
                pVmCollection = pUserInstance->getVmCollection(i);
                if (pVmCollection) {
                    nInstances = pVmCollection->getNumInstances();
                    dRentTime = pVmCollection->getRentTime() * 3600;
                    //Create a loop to insert all the instances.
                    vmType = pVmCollection->getVmType();
                    for (int j = 0; j < nInstances; j++) {
                        pVmInstance = pVmCollection->getVmInstance(j);
                        if (pVmInstance != NULL) {
                            instanceId = pVmInstance->getVmInstanceId();
                            pUserRet->createNewVmRequest(vmType, instanceId,
                                    maxStartTime_t1, dRentTime, maxSubTime_t3,
                                    maxSubscriptionTime_t4);
                        }
                    }
                } else {
                    EV_TRACE
                            << "WARNING! [UserGenerator] The VM collection is empty"
                            << endl;
                    throw omnetpp::cRuntimeError(
                            "[UserGenerator] The VM collection is empty!");
                }
            }
        } else {
            EV_TRACE
                    << "WARNING! [UserGenerator] Collection or User-ID malformed"
                    << endl;
            throw omnetpp::cRuntimeError(
                    "[UserGenerator] Collection or User-ID malformed!");
        }
    } else {
        EV_INFO
                << "UserGenerator::createNextVmRequest - The user instance is null"
                << endl;
    }

    EV_TRACE << "UserGenerator::createNextVmRequest - End" << endl;

    return pUserRet;
}

SM_UserAPP* UserGenerator_simple::createAppRequest(SM_UserVM *userVm) {
    VM_Response *pRes;
    VM_Request vmRq;
    SM_UserAPP *userApp;
    CloudUserInstance *pUserInstance;
    AppInstance *pAppInstance;
    std::string strIp, strAppInstance, strUserName, strService, strVmId;
    int nStartRentTime, nPrice, nMaxStarTime, nIndexRes;

    EV_TRACE << "UserGenerator::createNextAppRequest - Init" << endl;

    if (userVm != nullptr) {
        userApp = new SM_UserAPP();
        strUserName = userVm->getUserID();
        userApp->setUserID(strUserName.c_str());

        pUserInstance = userHashMap.at(strUserName);
        pUserInstance->setRequestApp(userApp);

        //Include the Ips and startTime
        for (int i = 0; i < userVm->getTotalVmsRequests(); i++) {
            //TODO: Hay que tomar una decision y enviar cada aplicacion a cada maquina?
            vmRq = userVm->getVms(i);

            pAppInstance = pUserInstance->getAppInstance(i);
            if (pAppInstance != nullptr) {
                strService = pAppInstance->getAppName();
            }

            //In this first approach, we get the first element,
            //in future cases, we can get the cheapest or the VM with a higher performance ratio
            nIndexRes = 0;

            if (userVm->hasResponse(i, nIndexRes)) {
                pRes = userVm->getResponse(i, nIndexRes);

                if (pRes != nullptr) {
                    nMaxStarTime = maxStartTime_t1;

                    //strIp = vmRes.strIp;
                    //nStartRentTime = vmRes.startTime;
                    //nPrice = vmRes.nPrice;

                    strIp = pRes->strIp;
                    nStartRentTime = pRes->startTime;
                    nPrice = pRes->nPrice;
                    strVmId = vmRq.strVmId;

                    //TODO: YA funciona esto, adaptar!!
                    if (bMaxStartTime_t1_active) {
                        //Check if T2 <T3
                        if (nMaxStarTime >= nStartRentTime) {
                            userApp->createNewAppRequest(strService, strIp,
                                    strVmId, nStartRentTime);
                        } else {
                            //The rent time proposed by the server is too high.
                            EV_INFO
                                    << "The maximum start rent time provided by the cloudprovider is greater than the maximum required by the user: "
                                    << nMaxStarTime << " < " << nStartRentTime
                                    << endl;
                        }
                    } else {
                        userApp->createNewAppRequest(strService, strIp, strVmId,
                                nStartRentTime);
                    }
                }

            } else
                EV_INFO << "WARNING! Invalid response in user: "
                        << userVm->getName() << endl;
        }
    }

    EV_TRACE << "UserGenerator::createNextVmRequest - End" << endl;

    return userApp;
}
bool UserGenerator_simple::allUsersFinished() {
    bool bRet;
    int nFinished, nSize;
    std::map<std::string, CloudUserInstance*>::iterator it;
    CloudUserInstance *pUserInstance;

    nFinished = 0;
    bRet = true;

    EV_INFO << "allUsersFinished - Checking if all users have finished" << endl;

    if (userInstances.size() > 0) {
        nSize = userInstances.size();

        for (int i = 0; (i < nSize) && bRet; i++) {
            pUserInstance = userInstances.at(i);
            bRet = pUserInstance->isFinished();
            if (bRet) {
                nFinished++;
            } else {
                EV_INFO << "allUsersFinished - User  "
                        << pUserInstance->toString(false, false)
                        << " | is already running" << endl;
            }
        }
    }
    EV_INFO << "allUsersFinished - Res " << bRet << " | NFinished: "
            << nFinished << " vs Total: " << nUserInstancesFinished << endl;

    return bRet;
}
void UserGenerator_simple::printExperiments_phase1() {
    int nIndex, nSize;
    std::map<std::string, CloudUserInstance*>::iterator it;
    CloudUserInstance *pUserInstance;
    std::vector<CloudUserInstance*> userVector;

    nIndex = 1;

    nSize = userInstances.size();

    double dInitTime, dEndTime, dExecTime, dSubTime, dMaxSub;
    nIndex = 1;
    for (int i = 0; (i < nSize); i++) {
        pUserInstance = userInstances.at(i);
        dInitTime = pUserInstance->getArrival2Cloud();
        dEndTime = pUserInstance->getEndTime();
        dMaxSub = pUserInstance->getT4();
        dExecTime = pUserInstance->getInitExecTime();

        if (dMaxSub != 0)
            dMaxSub = dMaxSub / 1000;

        if (dInitTime != 0)
            dInitTime = dInitTime / 1000;

        if (dEndTime != 0)
            dEndTime = dEndTime / 1000;

        if (dExecTime != 0)
            dExecTime = dExecTime / 1000;

        if (dExecTime > dInitTime)
            dSubTime = dExecTime - dInitTime;
        else
            dSubTime = 0;

        if (pUserInstance->isTimeout())
            EV_FATAL << "#___#Timeout " << nIndex << " -1 " << dMaxSub
                    << "   \n" << endl;
        else
            EV_FATAL << "#___#Success " << nIndex << " " << dSubTime << " -1 "
                    << "   \n" << endl;

        nIndex++;
    }
    EV_INFO << "]" << endl;
}
void UserGenerator_simple::printFinal() {
    /*int nIndex, nSize;
     std::map<std::string, CloudUserInstance*>::iterator it;
     CloudUserInstance* pUserInstance;
     std::vector <CloudUserInstance*> userVector;

     nIndex=1;

     nSize = userInstances.size();

     double dInitTime, dEndTime;
     nIndex=1;
     for(int i=0;(i<nSize);i++)
     {
     pUserInstance = userInstances.at(i);
     dInitTime = pUserInstance->getInitTime();
     dEndTime = pUserInstance->getEndTime();

     if(dInitTime!=0)
     dInitTime = dInitTime/1000;

     if(dEndTime!=0)
     dEndTime = dEndTime/1000;

     if(pUserInstance->isTimeout())
     EV_FATAL << "#___#" << pUserInstance->getType() << " " << nIndex << " " << dInitTime <<  " 0 -1 " << dEndTime <<"   \n" << endl;
     else
     EV_FATAL << "#___#" << pUserInstance->getType() << " " << nIndex << " " << dInitTime <<  " " << dEndTime << " "<< dEndTime-dInitTime << " -1   \n" << endl;

     nIndex++;
     }
     EV_INFO << "]" << endl;*/
    calculateStatistics();

}
void UserGenerator_simple::calculateStatistics() {
    double dInitTime, dEndTime, dExecTime, dSubTime, dMaxSub, dTotalSub,
            dMeanSub, dNoWaitUsers, dWaitUsers;
    int nIndex, nSize, nTotalTimeouts;
    CloudUserInstance *pUserInstance;
    std::vector<CloudUserInstance*> userVector;

    nIndex = 1;
    nTotalTimeouts = 0;
    dInitTime = dEndTime = dExecTime = dSubTime = dMaxSub = dTotalSub =
            dMeanSub = 0;
    dNoWaitUsers = dWaitUsers = 0;
    nSize = userInstances.size();

    for (int i = 0; (i < nSize); i++) {
        pUserInstance = userInstances.at(i);
        dInitTime = pUserInstance->getArrival2Cloud();
        dEndTime = pUserInstance->getEndTime();
        dMaxSub = pUserInstance->getT4();
        dExecTime = pUserInstance->getInitExecTime();

        if (dMaxSub != 0)
            dMaxSub = dMaxSub / 3600;

        if (dInitTime != 0)
            dInitTime = dInitTime / 3600;

        if (dEndTime != 0)
            dEndTime = dEndTime / 3600;

        if (dExecTime != 0)
            dExecTime = dExecTime / 3600;

        if (dExecTime > dInitTime)
            dSubTime = dExecTime - dInitTime;
        else
            dSubTime = 0;

        if (pUserInstance->isTimeout()) {
            EV_FATAL << "#___#Timeout " << nIndex << " -1 " << dMaxSub
                    << "   \n" << endl;
            dSubTime += dMaxSub;
            nTotalTimeouts++;
        } else {
            EV_FATAL << "#___#Success " << nIndex << " " << dSubTime << " -1 "
                    << "   \n" << endl;
            dTotalSub += dSubTime;
        }
        if (pUserInstance->hasSubscribed())
            dWaitUsers++;
        else
            dNoWaitUsers++;
        nIndex++;
    }

    if (nSize > 0) {
        dMeanSub = dTotalSub / nSize;
    }

    //Print the experiments data
    EV_FATAL << "#___3d#" << dMeanSub << "   \n" << endl;
    EV_FATAL << "#___t#" << dMaxSub << " " << dMeanSub << " " << nTotalTimeouts
            << " " << dNoWaitUsers << " " << dWaitUsers << "   \n" << endl;
}
SM_UserVM* UserGenerator_simple::createFakeVmRequest() {
    SM_UserVM *userVm;
    userVm = new SM_UserVM();

    //Simple
    userVm->setUserID("Pepe-hardcore");

    //Insert the VMs requests
    userVm->createNewVmRequest("large", "1", 1000, 50, 10, 10);
    userVm->createNewVmRequest("large", "2", 1000, 70, 10, 10);
    userVm->createNewVmRequest("large", "3", 1000, 57, 10, 10);
    userVm->createNewVmRequest("small", "4", 1000, 75, 10, 10);
    userVm->createNewVmRequest("small", "5", 1000, 75, 10, 10);

    userVm->setIsResponse(false);
    userVm->setOperation(SM_VM_Req);

    return userVm;
}
