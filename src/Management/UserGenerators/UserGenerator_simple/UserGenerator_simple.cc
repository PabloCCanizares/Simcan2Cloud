#include "UserGenerator_simple.h"

#define USER_REQ_GEN_MSG "USER_REQ_GEN"

Define_Module(UserGenerator_simple);

UserGenerator_simple::~UserGenerator_simple() {

}

/**
 * This method initializes the structures and methods
 */
void UserGenerator_simple::initialize() {

    // Init super-class
    UserGeneratorBase::initialize();

    //Signals initialization
    initializeSignals();


    bMaxStartTime_t1_active = false;

    maxStartTime_t1 = par("maxStartTime_t1");
    nRentTime_t2 = par("nRentTime_t2");
    maxSubTime_t3 = par("maxSubTime_t3");
    maxSubscriptionTime_t4 = par("maxSubscriptionTime_t4");

    EV_INFO << "UserGenerator::initialize - Base initialized" << endl;

    m_nUsersSent = 0;
    numUsersFinished = 0;

    initializeSelfHandlers();
    initializeResponseHandlers();
    //initializeAppResultHandlers();
    //initializeVMResultHandlers();

    EV_INFO << "UserGenerator::initialize - End" << endl;
}


/**
 * This method initializes the signals
 */
void UserGenerator_simple::initializeSignals() {
    requestSignal   = registerSignal("request");
    responseSignal  = registerSignal("response");
    executeIpSignal   = registerSignal("executeIp");
    executeNotSignal   = registerSignal("executeNot");
    okSignal        = registerSignal("ok");
    failSignal      = registerSignal("fail");
    subscribeNoipSignal = registerSignal("subscribeNoip");
    subscribeFailSignal = registerSignal("subscribeFail");
    notifySignal    = registerSignal("notify");
    timeOutSignal   = registerSignal("timeOut");
}


/**
 * This method initializes the self message handlers
 */
void UserGenerator_simple::initializeSelfHandlers() {
    //selfMessageHandlers[Timer_WaitToExecute] = std::bind(&UserGenerator_simple::processWaitToExecuteMessage, this, std::placeholders::_1);
    //selfMessageHandlers[USER_REQ_GEN_MSG] = std::bind(&UserGenerator_simple::processUserReqGenMessage, this, std::placeholders::_1);
    selfMessageHandlers[Timer_WaitToExecute] = [this](cMessage *msg) { handleWaitToExecuteMessage(msg); };
    selfMessageHandlers[USER_REQ_GEN_MSG] = [this](cMessage *msg) { handleUserReqGenMessage(msg); };
}


/**
 * This method initializes the response handlers
 */
void UserGenerator_simple::initializeResponseHandlers() {
    //responseHandlers[SM_VM_Req_Rsp] = std::bind(&UserGenerator_simple::processUserVmResponse, this, std::placeholders::_1);;
    //responseHandlers[SM_VM_Notify] = std::bind(&UserGenerator_simple::processUserVmResponse, this, std::placeholders::_1);;
    //responseHandlers[SM_APP_Rsp] = std::bind(&UserGenerator_simple::processUserAppResponse, this, std::placeholders::_1);

    responseHandlers[SM_VM_Res_Accept] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleResponse(msg); };
    responseHandlers[SM_VM_Res_Reject] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleResponse(msg); };
    responseHandlers[SM_APP_Res_Accept] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleAppOk(msg); };
    responseHandlers[SM_APP_Res_Reject] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleAppTimeout(msg); };
    responseHandlers[SM_APP_Res_Timeout] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleAppTimeout(msg); };
    responseHandlers[SM_APP_Sub_Accept] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleSubNotify(msg); };
    responseHandlers[SM_APP_Sub_Timeout] = [this](SIMCAN_Message *msg) -> CloudUserInstance* { return handleSubTimeout(msg); };
}

/**
 * Shuffle the list of users in order to reproduce the behaviour of the users in a real cloud environment.
 */
void UserGenerator_simple::generateShuffledUsers() {
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
    std::map<std::string, std::function<void(cMessage*)>>::iterator it;

    it = selfMessageHandlers.find(msg->getName());

    if (it == selfMessageHandlers.end()) {
        error("Unknown self message [%s]", msg->getName());
    } else {
        it->second(msg);
    }

    delete (msg);
}

void UserGenerator_simple::handleWaitToExecuteMessage(cMessage *msg) {
    SM_UserVM *userVm;
    CloudUserInstance *pUserInstance;
    double lastTime;

    // Log (INFO)
    EV_INFO << "Starting execution!!!" << endl;

    //TODO:
    m_dInitSim = simTime().dbl();
    lastTime = 0;

    // srand((int)33); // TODO
    //generateShuffledUsers();

    for (int i = 0; i < userInstances.size(); i++) {
        lastTime = getNextTime(m_dInitSim, lastTime);

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

    if (!intervalBetweenUsers) {
        std::sort(userInstances.begin(), userInstances.end(), [](CloudUserInstance* cloudUser1, CloudUserInstance* cloudUser2){*cloudUser1<*cloudUser2});
    }

    m_nUsersSent = 0;

    scheduleNextReqGenMessage();
}

double UserGenerator_simple::getNextTime(double init, double last) {
    double next;

    if (intervalBetweenUsers) {
        if (last > 0)
            next = distribution->doubleValue() + last;
        else
            next = init;
    }
    else {
        next = distribution->doubleValue() + init;
    }

    return next;
}

void UserGenerator_simple::handleUserReqGenMessage(cMessage *msg) {
    SM_UserVM *userVm;
    CloudUserInstance *pUserInstance;

    EV_INFO << "processSelfMessage - USER_REQ_GEN_MSG" << endl;
    // Get current user
    pUserInstance = userInstances.at(m_nUsersSent);
    userVm = pUserInstance->getRequestVmMsg();

    if (userVm != nullptr) {
        // Send user to cloud provider
        emit(requestSignal, pUserInstance->getId());
        sendRequestMessage(userVm, toCloudProviderGate);

        //TODO: ¿Dejamos este mensaje o lo quitamos?
        EV_FATAL << "#___ini#" << m_nUsersSent << " "
                        << (simTime().dbl() - m_dInitSim) / 3600 << "   \n"
                        << endl;
    }
    m_nUsersSent++;

    scheduleNextReqGenMessage();
}

void UserGenerator_simple::scheduleNextReqGenMessage() {
    CloudUserInstance *pUserInstance;
    simtime_t nextArrivalTime;

    //Check if there are more users
    if (m_nUsersSent < userInstances.size()) {
        pUserInstance = userInstances.at(m_nUsersSent);

        //Check if next arrival time is in the future
        nextArrivalTime = SimTime(pUserInstance->getArrival2Cloud());
        if (nextArrivalTime < simTime())
            error("Vector of user instances is not sorted by arrival time");

        scheduleAt(SimTime(nextArrivalTime), new cMessage(USER_REQ_GEN_MSG));
    }
}

void UserGenerator_simple::processRequestMessage(SIMCAN_Message *sm) {
    error("This module cannot process request messages:%s",
            sm->contentsToString(true, false).c_str());
}

void UserGenerator_simple::processResponseMessage(SIMCAN_Message *sm) {
    CloudUserInstance *pUserInstance;
    std::map<int, std::function<CloudUserInstance*(SIMCAN_Message*)>>::iterator it;

    EV_INFO << "processResponseMessage - Received Response Message" << endl;

    it = responseHandlers.find(sm->getResult());

    if (it == responseHandlers.end())
        EV_INFO << "processResponseMessage - Unhandled response" << endl;
    else
        pUserInstance = it->second(sm);

    if (pUserInstance != nullptr && pUserInstance->isFinished()) {
        numUsersFinished++;

        //Check if all the users have ended
        if (allUsersFinished())
            onFinish();
    }
}

void UserGenerator_simple::execute(CloudUserInstance *pUserInstance, SM_UserVM *userVm) {
    emit(notifySignal, pUserInstance->getId());
    emit(executeNotSignal, pUserInstance->getId());
    pUserInstance->setInitExecTime(simTime().dbl());
    submitService(userVm);
}

void UserGenerator_simple::trySubscribe(CloudUserInstance *pUserInstance, SM_UserAPP *userApp) {
    if (hasToSubscribeVm(userApp)) {
        recoverVmAndsubscribe(userApp);
    }
    else {
        //End of the protocol, exit!!
        finishUser(pUserInstance, REASON_APP_TIMEOUT);

        cancelAndDeleteMessages(pUserInstance);
    }
}

void UserGenerator_simple::finishUser(CloudUserInstance *pUserInstance, int reason) {
    pUserInstance->setEndTime(simTime().dbl());
    pUserInstance->setFinished(true);

    switch (reason) {
        case REASON_APP_TIMEOUT:
            pUserInstance->setTimeoutMaxRentTime();
        case REASON_OK:
            nUserInstancesFinished++;
            break;
        case REASON_SUB_TIMEOUT:
            pUserInstance->setTimeoutMaxSubscribed();
            break;
    }
}

CloudUserInstance* UserGenerator_simple::handleResponse(SIMCAN_Message *userVm_RAW) {
    CloudUserInstance *pUserInstance;
    bool rejected;
    SM_UserVM *userVm = dynamic_cast<SM_UserVM*>(userVm_RAW);

    if (userVm != nullptr) {
        EV_INFO << "handleResponse - Response message" << endl;

        userVm->printUserVM();

        //Update the status
        updateVmUserStatus(userVm);

        //Check the response and proceed with the next action
        pUserInstance = userHashMap.at(userVm->getUserID());

        if (pUserInstance != nullptr) {
            rejected = userVm->getResult() == SM_VM_Res_Reject;
            emit(responseSignal, pUserInstance->getId());
            pUserInstance->setInitExecTime(simTime().dbl());

            // If the vm-request has been rejected by the cloudprovider
            // we have to subscribe the service
            pUserInstance->setSubscribe(rejected);
            if (rejected) { // IPs = {}
                emit(subscribeNoipSignal, pUserInstance->getId());
                subscribe(userVm);
            // otherwise the next step is to submit the required services.
            }
            else { // IPs != {}
                emit(executeIpSignal, pUserInstance->getId());
                submitService(userVm);
            }
        }
    }
    else {
        error("Could not cast SIMCAN_Message to SM_UserVM (wrong operation code?)");
    }

    return pUserInstance;
}

CloudUserInstance* UserGenerator_simple::handleAppOk(SIMCAN_Message *userApp_RAW) {
    CloudUserInstance *pUserInstance;
    SM_UserAPP *userApp = dynamic_cast<SM_UserAPP*>(userApp_RAW);

    if (userApp != nullptr) {
        //Print a debug trace ...
        userApp->printUserAPP();

        EV_INFO << "handleAppOk - Init" << endl;

        //End of the protocol, exit!!
        pUserInstance = userHashMap.at(userApp->getUserID());
        if (pUserInstance != nullptr)
            finishUser(pUserInstance, REASON_OK);

        emit(okSignal, pUserInstance->getId());

        cancelAndDeleteMessages(pUserInstance);

        EV_INFO << "handleAppOk - End" << endl;
    }
    else {
        error("Could not cast SIMCAN_Message to SM_UserAPP (wrong operation code?)");
    }

    return pUserInstance;
}

CloudUserInstance* UserGenerator_simple::handleAppTimeout(SIMCAN_Message *userApp_RAW) {
    CloudUserInstance *pUserInstance;
    SM_UserAPP *userApp = dynamic_cast<SM_UserAPP*>(userApp_RAW);

    if (userApp != nullptr) {
        //Print a debug trace ...
        userApp->printUserAPP();

        EV_INFO << "handleAppTimeout - Init" << endl;

        //The next step is to send a subscription to the cloudprovider
        //Recover the user instance, and get the VmRequest
        pUserInstance = userHashMap.at(userApp->getUserID());
        if (pUserInstance != nullptr) {
            emit(failSignal, pUserInstance->getId());

            trySubscribe(pUserInstance, userApp);
        }

        EV_INFO << "handleAppTimeout - End" << endl;
    }
    else {
        error("Could not cast SIMCAN_Message to SM_UserAPP (wrong operation code?)");
    }

    return pUserInstance;
}

CloudUserInstance* UserGenerator_simple::handleSubNotify(SIMCAN_Message *userVm_RAW) {
    CloudUserInstance *pUserInstance;
    SM_UserVM *userVm = dynamic_cast<SM_UserVM*>(userVm_RAW);

    if (userVm != nullptr) {

        userVm->printUserVM();

        //Update the status
        updateVmUserStatus(userVm);

        pUserInstance = userHashMap.at(userVm->getUserID());
        EV_INFO << "Subscription accepted ...  " << endl;

        if (pUserInstance != nullptr) {
            execute(pUserInstance, userVm);
        }
    }
    else {
        error("Could not cast SIMCAN_Message to SM_UserVM (wrong operation code?)");
    }

    return pUserInstance;
}



CloudUserInstance* UserGenerator_simple::handleSubTimeout(SIMCAN_Message *userVm_RAW) {
    CloudUserInstance *pUserInstance;
    SM_UserVM *userVm = dynamic_cast<SM_UserVM*>(userVm_RAW);

    if (userVm != nullptr) {

        userVm->printUserVM();

        //Update the status
        updateVmUserStatus(userVm);

        pUserInstance = userHashMap.at(userVm->getUserID());

        // End of the party.
        EV_INFO << "VM timeout ... go home" << endl;

        if (pUserInstance != nullptr) {
            emit(timeOutSignal, pUserInstance->getId());
            EV_INFO << "Set itself finished" << endl;
            finishUser(pUserInstance, REASON_SUB_TIMEOUT);

            cancelAndDeleteMessages(pUserInstance);
        }
    }
    else {
        error("Could not cast SIMCAN_Message to SM_UserVM (wrong operation code?)");
    }

    return pUserInstance;
}

bool UserGenerator_simple::hasToSubscribeVm(SM_UserAPP* userApp)
{
    double dRandom;

    dRandom = ((double) rand() / (RAND_MAX));

    return dRandom<=1;
}

void UserGenerator_simple::cancelAndDeleteMessages(CloudUserInstance *pUserInstance) {
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
            emit(subscribeFailSignal, pUserInstance->getId());
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

class cCustomNotification : public cObject, noncopyable
{
  public:
    SM_UserVM *userVm;
};

//TODO: Esto en princpio esta bien y lo dejamos así, más adelante debemos discutir si es preferible así
// o con encapsulación. La ecapsulación nos prodría traer el problema de que si queremos modificar la clase
// VmInstance tengamos que modificar todas en cascada. Porque tal vez no. si pasamos el mensaje completo de una a otra.
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
    CloudUserInstance *pUserInstance;
    std::string strUserName;

    strUserName = userVm->getUserID();
    pUserInstance = userHashMap.at(strUserName);
    pAppRq = pUserInstance->getRequestAppMsg();

    if (pAppRq == nullptr)
        pAppRq = createAppRequest(userVm);

    if (pAppRq != nullptr) {
        pAppRq->setIsResponse(false);
        pAppRq->setOperation(SM_APP_Req);
        sendRequestMessage(pAppRq, toCloudProviderGate);
    }
}

CloudUserInstance* UserGenerator_simple::getNextUser() {
    CloudUserInstance *pUserInstance;

    pUserInstance = nullptr;

    if (nUserIndex < userInstances.size()) {
        pUserInstance = userInstances.at(nUserIndex);
        EV_INFO
                       << "UserGenerator::getNextUser - The next user to be processed is "
                       << pUserInstance->getUserID() << " [" << nUserIndex
                       << " of " << userInstances.size() << "]" << endl;
        nUserIndex++;
    } else {
        EV_INFO
                       << "UserGenerator::getNextUser - The requested user index is greater than the collection size. ["
                       << nUserIndex << " of " << userInstances.size() << "]"
                       << endl;
    }
    return pUserInstance;
}

//TODO: Aquí tenemos el mismo problema de la encapsulación. Aquí tendriamos el mismo problema
//Aqui no podemos pasar el mensaje. Entonces o se hace una estrucutra que se pase y se encapsula o se queda así
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
                        << " | rentTime_t2: " << nRentTime_t2
                        << " | maxSubTime: " << maxSubTime_t3
                        << " | MaxSubscriptionTime_T4:"
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

//TODO: Mismo problema de encapsulación
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
                                           << nMaxStarTime << " < "
                                           << nStartRentTime << endl;
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

//TODO: Esto tal vez deberiamos cambiarlo por un contador total que se actualize cada vez que acabe un user,
//así no hay que hacer un bucle cada vez.
bool UserGenerator_simple::allUsersFinished() {
    bool bRet;

    EV_INFO << "allUsersFinished - Checking if all users have finished" << endl;

    bRet = numUsersFinished >= userInstances.size();

    EV_INFO << "allUsersFinished - Res " << bRet << " | NFinished: "
                   << numUsersFinished << " vs Total: " << userInstances.size() // Antes comparaba con nUserInstancesFinished. No se actualiza en los timeout
                   << endl;

    return bRet;
}

void UserGenerator_simple::onFinish() {
    sendRequestMessage(new SM_CloudProvider_Control(), toCloudProviderGate);
    printFinal();
}

//TODO: Las salidas se podrian hacer con omnet. Como hago lo de los emit, así se puede analizar facil con python y apliaciones van añadiendo no
// tienen que implmentar todo el metodo de nuevo.
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
                    << " " << dNoWaitUsers << " " << dWaitUsers << "   \n"
                    << endl;
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
