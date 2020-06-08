#include "CloudProviderBase_firstBestFit.h"

//Define_Module(CloudProviderBase_firstBestFit);



CloudProviderBase_firstBestFit::~CloudProviderBase_firstBestFit(){
    handlingAppsRqMap.clear();
    acceptedUsersRqMap.clear();
}


void CloudProviderBase_firstBestFit::initialize(){

    EV_INFO << "CloudProviderFirstFit::initialize - Init" << endl;

    // Init super-class
    CloudProviderBase::initialize();

    // Init data-center structures
    //Fill the meta-structures created to improve the performance of the cloudprovider
    loadNodes();
    initializeRequestHandlers();

    bFinished = false;
    scheduleAt(simTime().dbl()+1, new cMessage(INITIAL_STAGE));
    EV_INFO << "CloudProviderFirstFit::initialize - End" << endl;
}

/**
 * This method initializes the request handlers
 */
void CloudProviderBase_firstBestFit::initializeRequestHandlers() {
    requestHandlers[SM_VM_Req] = [this](SIMCAN_Message *msg) -> void { return handleVmRequestFits(msg); };
    requestHandlers[SM_VM_Sub] = [this](SIMCAN_Message *msg) -> void { return handleVmSubscription(msg); };
    requestHandlers[SM_APP_Req] = [this](SIMCAN_Message *msg) -> void { return handleUserAppRequest(msg); };
}

void CloudProviderBase_firstBestFit::loadNodes()
{
    DataCenter* pDataCenter;
    Rack* pRack;
    RackInfo* pRackInfo;
    int nComputingRacks, nTotalNodes, nDataCenter, nIndex;

    EV_INFO << "CloudProviderFirstFit::loadNodes - Init" << endl;
    //Initialize
    nComputingRacks =  nTotalNodes = nDataCenter = nIndex=0;

    nDataCenter = dataCentersBase.size();

    //Go over the datacenter object: all racks
    if(nDataCenter>0)
    {
        EV_INFO << "CloudProviderFirstFit::loadNodes - handling the DC: " <<  dataCentersToString() << endl;
        for(int nIndex=0;nIndex<nDataCenter;nIndex++)
        {
            EV_TRACE << "CloudProviderFirstFit::loadNodes - loading dataCenter: " << nIndex << endl;
            pDataCenter = dataCentersBase.at(nIndex);
            if(pDataCenter != nullptr)
            {
                nComputingRacks = pDataCenter->getNumRacks(false);
                EV_TRACE << "CloudProviderFirstFit::loadNodes - Number of computing racks: "<< nComputingRacks << endl;
                for(int i=0;i<nComputingRacks; i++)
                {
                    EV_DEBUG << "Getting computing rack: "<< i << endl;

                    pRack = pDataCenter->getRack(i, false);

                    if(pRack != NULL)
                    {
                        nTotalNodes = pRack->getNumNodes();

                        EV_DEBUG << "Total nodes:" <<  nTotalNodes << endl;
                        if(nTotalNodes >0)
                        {
                            //Insert the Rack
                            pRackInfo = pRack->getRackInfo();
                            insertRack(nIndex, i, pRackInfo);
                        }
                        else
                        {
                            EV_INFO << "WARNING!! The rack is empty :<"<< i << endl;
                        }
                    }
                }
            }
        }
    }

    //Print the content of the loaded DC
    datacenterCollection->printDCSizes();

    EV_INFO << "CloudProviderFirstFit::loadNodes - Ende" << endl;
}
void CloudProviderBase_firstBestFit::initDataCenterStructures(){

    DataCenter* currentDataCenter;
    DataCenter_CpuType* newDataCenter_CpuType;
    Rack* currentRack;
    int dataCenterIndex, rackIndex, nodeIndex, numCpus;

    //TODO: Pensar si sustituir este método por loadNodes
    // For each data-center
    for (dataCenterIndex = 0; dataCenterIndex < dataCentersBase.size(); dataCenterIndex++){

        // Get current data-center
        currentDataCenter = dataCentersBase.at(dataCenterIndex);

        // Create a new DataCenter_CpuType
        newDataCenter_CpuType = new DataCenter_CpuType (currentDataCenter->getName());

        // For each computing rack
        for (rackIndex = 0; rackIndex < currentDataCenter->getNumRacks(false); rackIndex++){

            // Get current rack
            currentRack = currentDataCenter->getRack(rackIndex, false);

            // Get number of CPUs
            numCpus = currentRack->getRackInfo()->getNodeInfo()->getNumCpUs();

            // For each node in current rack
            for (nodeIndex=0; nodeIndex < currentRack->getNumNodes(); nodeIndex++){

                // Insert the pointer of the current node into the new dataCenter
                // newDataCenter_CpuType->insertNode(currentRack->getNode(nodeIndex), numCpus);
            }
        }
    }
    // nDataCenterIndex = nNodeIndex = 0;
}
void CloudProviderBase_firstBestFit::abortAllApps(SM_UserAPP* userApp, std::string strVmId)
{
    if(userApp != nullptr)
    {
        userApp->abortAllApps(strVmId);

        // Terminar este método, para cancelar el resto de eventos si se requiere.
    }
}

void CloudProviderBase_firstBestFit::handleManageSubscriptions(cMessage* msg) {
    SM_UserVM* userVmSub;
    std::string strUsername;
    bool bFreeSpace;
    double dWaitingSub, dMaxSubtime;


    bFreeSpace = false;
    if (subscribeQueue.size() > 0) {
        EV_TRACE << "The queue of subscription: " << subscribeQueue.size()
                                << endl;

        //First of all, check the subscriptions timeouts
        for (int i = 0; i < subscribeQueue.size(); i++) {
            userVmSub = subscribeQueue.at(i);

            if (userVmSub != nullptr) {
                strUsername = userVmSub->getUserID();

                dWaitingSub = (simTime().dbl())
                                - (userVmSub->getDStartSubscriptionTime());
                dMaxSubtime = userVmSub->getMaxSubscribetime(0);

                if (dWaitingSub > dMaxSubtime) {
                    EV_INFO << "The subscription of the user:  " << strUsername
                            << " has expired, TIMEOUT! SimTime()"
                            << simTime().dbl() << " - "
                            << userVmSub->getDStartSubscriptionTime()
                            << " = | " << dWaitingSub << " vs "
                            << dMaxSubtime << endl;
                    freeUserVms(strUsername);
                    bFreeSpace = true;
                    subscribeQueue.erase(subscribeQueue.begin() + i);
                    i--;

                    userVmSub->setDEndSubscriptionTime(simTime().dbl());

                    //send the Timeout
                    timeoutSubscription(userVmSub);
                } else {
                    EV_TRACE << "The subscription of the user:  " << strUsername
                            << " has time to wait!" << simTime().dbl()
                            << " - "
                            << userVmSub->getDStartSubscriptionTime()
                            << " = | " << dWaitingSub << " vs "
                            << dMaxSubtime << " || "
                            << dWaitingSub - dMaxSubtime << endl;
                }
            }
        }
        //Check the subscription queue
        updateSubsQueue();
    } else
        EV_TRACE << "The subscription queue is empty" << endl;

}

void CloudProviderBase_firstBestFit::handleAppExecEndSingle(SM_UserAPP_Finish* pUserAppFinish) {

    SM_UserAPP* userApp;
    std::string strUsername, strVmId, strAppName, strIp;

    strUsername = pUserAppFinish->getUserID();
    strVmId = pUserAppFinish->getStrVmId();
    strAppName = pUserAppFinish->getStrApp();
    EV_INFO << "The execution of the App [" << pUserAppFinish->getStrApp()
                           << " / " << pUserAppFinish->getStrVmId()
                           << "] launched by the user " << strUsername
                           << " has finished" << endl;
    //Used only for notify the user.
    if (handlingAppsRqMap.find(strUsername) != handlingAppsRqMap.end()) {
        handlingAppsRqMap.at(strUsername)->increaseFinishedApps();
        userApp = handlingAppsRqMap.at(strUsername);

        if (userApp != nullptr) {
            //Check for a possible timeout
            if (!userApp->isFinishedKO(strAppName, strVmId)) {
                EV_INFO
                << "processSelfMessage - Changing status of the application [ app: "
                << strAppName << " | vmId: " << strVmId << endl;
                userApp->printUserAPP();

                userApp->changeState(strAppName, strVmId, appFinishedOK);
                userApp->setEndTime(strAppName, strVmId, simTime().dbl());
            }
            //if(userApp->getNFinishedApps() >= userApp->getAppArraySize())

            if (userApp->allAppsFinished()) {
                if (userApp->allAppsFinishedOK()) {
                    EV_INFO
                    << "processSelfMessage - All the apps corresponding with the user "
                    << strUsername
                    << " have finished successfully" << endl;

                    userApp->printUserAPP();

                    //Notify the user the end of the execution
                    acceptAppRequest(userApp);
                } else {
                    EV_INFO
                    << "processSelfMessage - All the apps corresponding with the user "
                    << strUsername
                    << " have finished with some errors" << endl;

                    //Check the subscription queue
                    //updateSubsQueue();

                    if (!userApp->getFinished()) {
                        //Notify the user the end of the execution
                        timeoutAppRequest(userApp);
                    }
                }

                //Delete the application on the hashmap
                handlingAppsRqMap.erase(strUsername);
            } else {
                EV_INFO << "processSelfMessage - Total apps finished: "
                        << userApp->getNFinishedApps() << " of "
                        << userApp->getAppArraySize() << endl;
            }
        } else {
            EV_INFO
            << "processSelfMessage - WARNING! I cant found the apps corresponding with the user "
            << strUsername << endl;
        }
    } else {
        EV_INFO
        << "processSelfMessage - WARNING! I cant found the apps corresponding with the user "
        << strUsername << endl;
    }
}

//TODO: asignar la vm que hace el timout al mensaje de la app. Duplicarlo y enviarlo.
void CloudProviderBase_firstBestFit::handleExecVmRentTimeout(SM_UserVM_Finish* pUserVmFinish) {
    SM_UserAPP *userApp;
    std::string strUsername, strVmId, strAppName, strIp;
    bool bAlreadyFinished;
    EV_INFO << "processSelfMessage - INIT" << endl;
    bAlreadyFinished = false;
    strUsername = pUserVmFinish->getUserID();
    strVmId = pUserVmFinish->getStrVmId();
    EV_INFO << "The rent of the VM [" << pUserVmFinish->getStrVmId()
                           << "] launched by the user " << strUsername
                           << " has finished" << endl;
    //Check the Application status
    if (handlingAppsRqMap.find(strUsername) != handlingAppsRqMap.end()) {
        userApp = handlingAppsRqMap.at(strUsername);

        //Check the application status
        if (userApp != nullptr) {

            EV_INFO << "Last id gate: " << userApp->getLastGateId() << endl;
            EV_INFO
            << "Checking the status of the applications which are running over this VM"
            << endl;

            //Abort the running applications
            if (!userApp->allAppsFinished(strVmId)) {
                EV_INFO << "Aborting running applications" << endl;
                abortAllApps(userApp, strVmId);
            } else {
                EV_INFO << "All the applications have already finished" << endl;
                bAlreadyFinished = true;
            }

            EV_INFO << "Freeing resources..." << endl;

            //Check if all the applications of the user have finished
            if (userApp->allAppsFinished() && !userApp->getFinished()
                    && !bAlreadyFinished) {
                //Notify the user the end of the execution
                EV_INFO << "processSelfMessage - EXEC_VM_RENT_TIMEOUT Init"
                        << endl;

                //if so, notify this.
                timeoutAppRequest(userApp);
            }

        }
    }
    //Free the VM resources
    freeVm(strVmId);

    //Check the subscription queue
    updateSubsQueue();
}

void CloudProviderBase_firstBestFit::handleAppExecEnd(SM_UserAPP* userAPP_Rq) {
    std::string strUsername;

    strUsername = userAPP_Rq->getUserID();
    EV_INFO << "The execution of all the Apps launched by the user " << strUsername
                   << " have finished" << endl;
    //Check the subscription queue
    updateSubsQueue();
    //Notify the user the end of the execution
    acceptAppRequest(userAPP_Rq);
}

void CloudProviderBase_firstBestFit::processSelfMessage (cMessage *msg){

    std::string strUsername, strVmId, strAppName, strIp;
    SM_UserAPP* userAPP_Rq;
    SM_UserAPP_Finish* pUserAppFinish;
    SM_UserVM_Finish* pUserVmFinish;
    SM_UserVM userVmRequest;

    EV_TRACE << "processSelfMessage - Init" << endl;

    // Start execution?
    if(!strcmp(msg->getName(), INITIAL_STAGE))
    {
        EV_INFO << "processSelfMessage - INITIAL_STAGE" << endl;
        scheduleAt(simTime().dbl()+1, new cMessage(MANAGE_SUBSCRIBTIONS));
        delete (msg);
    }
    else if(!strcmp(msg->getName(), EXEC_VM_RENT_TIMEOUT))
    {
        //Timeout of a VM
        pUserVmFinish = dynamic_cast<SM_UserVM_Finish *>(msg);
        if(pUserVmFinish != nullptr)
        {
            handleExecVmRentTimeout(pUserVmFinish);
            cancelAndDelete(pUserVmFinish);
        }
    }
    else if (!strcmp(msg->getName(), EXEC_APP_END)){

        EV_INFO << "processSelfMessage - EXEC_APP_END" << endl;
        userAPP_Rq = dynamic_cast<SM_UserAPP *>(msg);
        if(userAPP_Rq != nullptr)
        {
            handleAppExecEnd(userAPP_Rq);
        }
    }
    else if (!strcmp(msg->getName(), EXEC_APP_END_SINGLE)){

        EV_INFO << "processSelfMessage - EXEC_APP_END_SINGLE" << endl;
        pUserAppFinish = dynamic_cast<SM_UserAPP_Finish *>(msg);
        if(pUserAppFinish != nullptr)
        {
            handleAppExecEndSingle(pUserAppFinish);
            cancelAndDelete(pUserAppFinish);
        }
    }
    else if(!strcmp(msg->getName(), USER_SUBSCRIPTION_TIMEOUT))
    {
        EV_INFO << "processSelfMessage - RECEIVED TIMEOUT " << endl;
        manageSubscriptionTimeout(msg);
        cancelAndDelete(msg);
    }
    else if(!strcmp(msg->getName(), MANAGE_SUBSCRIBTIONS))
    {
        handleManageSubscriptions(msg);
        delete (msg);
    }
    else
        error ("Unknown self message [%s]", msg->getName());

    EV_TRACE << "processSelfMessage - End" << endl;
}
void CloudProviderBase_firstBestFit::manageSubscriptionTimeout(cMessage *msg)
{
    int nIndex;
    double dWaitingSub, dMaxSubTime;
    SM_UserVM_Finish* pUserSubFinish;
    SM_UserVM* userVmSub;
    std::string strUsername;

    EV_TRACE << "manageSubscriptionTimeout - Init" << endl;

    nIndex = 0;
    dWaitingSub = dMaxSubTime = 0;
    userVmSub = nullptr;

    pUserSubFinish = dynamic_cast<SM_UserVM_Finish *>(msg);
    if(pUserSubFinish != nullptr)
    {
        strUsername = pUserSubFinish->getUserID();

        nIndex = searchUserInSubQueue(strUsername);
        if(nIndex != -1)
        {
            EV_TRACE << "manageSubscriptionTimeout - User found at position:" << nIndex << endl;
            freeUserVms(strUsername);
            userVmSub = subscribeQueue.at(nIndex);

            dWaitingSub = (simTime().dbl())-(userVmSub->getDStartSubscriptionTime());
            dMaxSubTime = userVmSub->getMaxSubscribetime(0);

            if(userVmSub != nullptr)
            {
                subscribeQueue.erase(subscribeQueue.begin()+nIndex);

                if(abs((int)dWaitingSub - (int)dMaxSubTime)<=1)
                {
                    EV_INFO << "The subscription of the user:  " << strUsername << " has expired, TIMEOUT! SimTime()" << simTime().dbl()<< " - " <<
                            userVmSub->getDStartSubscriptionTime() << " = | " << dWaitingSub << " vs " << dMaxSubTime  << endl;

                    userVmSub->setDEndSubscriptionTime(simTime().dbl());
                    freeUserVms(strUsername);
                    timeoutSubscription(userVmSub);
                }
                else
                    EV_TRACE << "The subscription of the user:  " << strUsername << " has time waiting!"
                    << simTime().dbl()<< " - " <<
                    userVmSub->getDStartSubscriptionTime() << " = | " << dWaitingSub << " vs " << dMaxSubTime  << endl;
                //send the Timeout
            }
        }
        else
        {
            EV_TRACE << "manageSubscriptionTimeout - User " << strUsername << " not found in queue list" << endl;

        }
        //Check the subscription queue
        updateSubsQueue();
    }
    EV_TRACE << "processSelfMessage - USER_SUBSCRIPTION_TIMEOUT End" << endl;
}
int CloudProviderBase_firstBestFit::searchUserInSubQueue(std::string strUsername)
{
    int nRet, nIndex;
    SM_UserVM* pUserVM;
    bool bFound;

    EV_TRACE << "searchUserInSubQueue - Init" << endl;
    nRet = -1;
    bFound= false;
    nIndex=0;

    EV_TRACE << "searchUserInSubQueue - Searching for user: " << strUsername << endl;
    if(strUsername.size()>0)
    {
        EV_TRACE << "searchUserInSubQueue - Queue size: " << subscribeQueue.size() << endl;
        while(!bFound && nIndex < subscribeQueue.size())
        {
            pUserVM = subscribeQueue.at(nIndex);

            if(pUserVM != nullptr)
            {
                if(strUsername.compare(pUserVM->getUserID()) == 0)
                {
                    bFound = true;
                }
                else
                {
                    EV_TRACE << "searchUserInSubQueue - [nIndex: " << nIndex << " " << strUsername << " vs " <<pUserVM->getUserID()<<" ]" << endl;
                    nIndex++;
                }
            }
            else
            {
                EV_INFO << "searchUserInSubQueue - WARNING! null pointer at position[nIndex: " << nIndex << " ]" << endl;
                nIndex++;
            }
        }
    }
    if(bFound)
        nRet = nIndex;

    EV_TRACE << "CloudProviderFirstFit::searchUserInSubQueue - End" << endl;

    return nRet;
}
void CloudProviderBase_firstBestFit::updateSubsQueue()
{
    SM_UserVM* userVmSub;

    //Finally, notify if there is enough space to handle a new notification
    for(int i=0;i<subscribeQueue.size();i++)
    {
        userVmSub = subscribeQueue.at(i);

        if(checkVmUserFit(userVmSub))
        {
            EV_INFO << "Notifying subscription of user: "<< userVmSub->getUserID()<<endl;
            notifySubscription(userVmSub);

            //Remove from queue
            subscribeQueue.erase(subscribeQueue.begin()+i);
            acceptedUsersRqMap[userVmSub->getUserID()] = *userVmSub;
            i--;
        }
    }
}
void CloudProviderBase_firstBestFit::freeUserVms(std::string strUsername)
{
    std::string strVmId;
    SM_UserVM pVmRequest;

    VM_Request vmRequest;

    EV_TRACE << "CloudProviderFirstFit::freeUserVms - Init" << endl;

    //Mark the user VMs as free
    if(acceptedUsersRqMap.find(strUsername) != acceptedUsersRqMap.end())
    {
        pVmRequest = acceptedUsersRqMap.at(strUsername);

        for(int i=0;i<pVmRequest.getVmsArraySize();i++)
        {
            vmRequest = pVmRequest.getVms(i);
            strVmId = vmRequest.strVmId;
            freeVm(strVmId);
        }
        acceptedUsersRqMap.erase(strUsername);
    }

    EV_TRACE << "CloudProviderFirstFit::freeUserVms - End" << endl;
}
void CloudProviderBase_firstBestFit::freeVm(std::string strVmId)
{
    EV_TRACE << "CloudProviderFirstFit::freeVm - Init" << endl;
    EV_TRACE << "CloudProviderFirstFit::freeVm - Releasing the Vm:  "<< strVmId << endl;

    if(datacenterCollection->freeVmRequest(strVmId))
    {
        //
        EV_DEBUG << "the Vm has been released sucessfully: "<< strVmId << endl;
    }
    else
    {
        //Error freeing the VM
        EV_INFO << "Error releasing the VM: "<< strVmId << endl;
    }

    EV_TRACE << "CloudProviderFirstFit::freeVm - End" << endl;
}
void CloudProviderBase_firstBestFit::insertRack(int nIndex, int nRack, RackInfo* pRackInfo)
{
    NodeInfo* pNodeInfo;
    NodeResourceInfo* pNodeResInfo;
    int nTotalCpus, nTotalMemory, cpuSpeed, nTotalNodes;
    double totalDiskGB, totalMemoryGB;
    bool storage;

    storage= false;
    nTotalCpus = nTotalMemory = cpuSpeed = nTotalNodes = 0;

    EV_INFO << "CloudProviderFirstFit::insertRack - Init "<< endl;

    if(pRackInfo != NULL)
    {
        pNodeInfo = pRackInfo->getNodeInfo();
        if(pNodeInfo != NULL)
        {
            //Retrieve all the values
            nTotalCpus =  pNodeInfo->getNumCpUs();
            nTotalMemory = pNodeInfo->getTotalMemoryGb();
            cpuSpeed = pNodeInfo->getCpuSpeed();
            totalDiskGB = pNodeInfo->getTotalDiskGb();
            totalMemoryGB = pNodeInfo -> getTotalMemoryGb();
            storage = pNodeInfo->isStorage();
            nTotalNodes =pRackInfo->getNumBoards()*pRackInfo->getNodesPerBoard();

            for(int j=0;j<nTotalNodes;j++)
            {
                EV_TRACE << "Adding computing node : "<< j << " | rack: " << nRack << endl;

                pNodeResInfo = new NodeResourceInfo();
                pNodeResInfo->setNumCpUs(nTotalCpus);
                pNodeResInfo->setAvailableCpUs(nTotalCpus);
                pNodeResInfo->setTotalMemoryGb(nTotalMemory);
                pNodeResInfo->setAvailableMemory(nTotalMemory);
                pNodeResInfo->setCpuSpeed(cpuSpeed);
                pNodeResInfo->setTotalDiskGb(totalDiskGB);
                pNodeResInfo->setAvailableDiskGb(totalDiskGB);

                pNodeResInfo->setIp("dc:"+std::to_string(nIndex)+"_rack:"+std::to_string(nRack)+"_node:"+std::to_string(j));
                pNodeResInfo->setDataCenter(nIndex);

                EV_TRACE << "CloudProviderFirstFit::insertRack - Inserting node "<< j << " at datacenter: " << nIndex << endl;
                //Insert the node info into the corresponding DC
                datacenterCollection->insertNode(nIndex, pNodeResInfo);
            }
        }
    }

    EV_INFO << "CloudProviderFirstFit::insertRack - End "<< endl;
}

void CloudProviderBase_firstBestFit::handleVmRequestFits(SIMCAN_Message *sm)
{
    SM_UserVM *userVM_Rq;

    userVM_Rq = dynamic_cast<SM_UserVM *>(sm);
    EV_INFO << "CloudProviderFirstFit::processRequestMessage - Handle VMrequest"  << endl;

    if(userVM_Rq != nullptr)
      {
        userVM_Rq->printUserVM();
        //Check if is a VmRequest or a subscribe
        if (checkVmUserFit(userVM_Rq))
          {
            acceptVmRequest(userVM_Rq);
          }
        else
          {
            rejectVmRequest(userVM_Rq);
          }
      }
}

void CloudProviderBase_firstBestFit::handleVmSubscription(SIMCAN_Message *sm)
{
    SM_UserVM *userVM_Rq;

    userVM_Rq = dynamic_cast<SM_UserVM *>(sm);
    EV_INFO << "CloudProviderFirstFit::processRequestMessage - Received Subscribe operation"  << endl;

    if(userVM_Rq != nullptr)
      {
        if (checkVmUserFit(userVM_Rq))
          {
            notifySubscription(userVM_Rq);
          }
        else
          {
            //Store the vmRequest
            storeVmSubscribe(userVM_Rq);
          }
      }
}

void CloudProviderBase_firstBestFit::processRequestMessage (SIMCAN_Message *sm){
    SM_CloudProvider_Control* userControl;
    std::map<int, std::function<void(SIMCAN_Message*)>>::iterator it;

    EV_INFO << "processResponseMessage - Received Request Message" << endl;

    it = requestHandlers.find(sm->getOperation());

    if (it == requestHandlers.end())
      {
        userControl = dynamic_cast<SM_CloudProvider_Control *>(sm);

        if(userControl != nullptr)
          {
            EV_INFO << "CloudProviderFirstFit::processRequestMessage - Received end of party"  << endl;
            //Stop the checking process.
            bFinished = true;
            cancelAndDelete(userControl);
          }
        else
          {
            EV_INFO << "Unknown message:"  << sm->getName() << endl;
          }
      }
    else
      {
        //Perform the operations...
        it->second(sm);
      }
}

void CloudProviderBase_firstBestFit::handleUserAppRequest(SIMCAN_Message *sm)
{
    //Get the user name, and recover the info about the VmRequests;
    bool bHandle;
    std::string strUsername, strAppType, strVmId, strIp, strAppName;
    Application* appType;
    SM_UserVM userVmRequest;
    SM_UserAPP_Finish* pMsgFinish;
    VM_Request vmRequest;
    APP_Request userApp;

    int nTotalTime;
    SM_UserAPP *userAPP_Rq;

    EV_INFO << "CloudProviderFirstFit::processRequestMessage - Handle AppRequest"  << endl;
    userAPP_Rq = dynamic_cast<SM_UserAPP *>(sm);

    if(userAPP_Rq != nullptr)
      {

        //Las aplicaciones estan relacionadas con las VM
        //Hay que relacionar la APP con la VM para asi poder terminar con ella.
        //Suponemos 1 VM por app, en el momento en que nos quedemos sin VM, le damos el ID de la ultima.
        bHandle = false;

        //APPRequest
        userAPP_Rq->printUserAPP();

        strUsername = userAPP_Rq->getUserID();
        if(acceptedUsersRqMap.find(strUsername) != acceptedUsersRqMap.end())
          {
            userVmRequest = acceptedUsersRqMap.at(strUsername);
            if(handlingAppsRqMap.find(strUsername) == handlingAppsRqMap.end())
              {
                //Registering the appRq
                handlingAppsRqMap[strUsername] = userAPP_Rq;
              }
            EV_INFO << "Executing the VMs corresponding with the user: " << strUsername << " | Total: "<< userVmRequest.getVmsArraySize()<< endl;

            //First step consists in calculating the total units of time spent in executing the application.
            if(userAPP_Rq->getArrayAppsSize()>0)
              {
                for(int i=0;i<userAPP_Rq->getAppArraySize();i++)
                  {
                    //Get the app
                    userApp =userAPP_Rq->getApp(i);
                    strAppType = userApp.strApp;

                    //Get the VM
                    if(i<userVmRequest.getVmsArraySize())
                      {
                        //Getting VM and scheduling renting timeout
                        vmRequest = userVmRequest.getVms(i);
//                        vmRequest.pMsg = new SM_UserVM_Finish();
//                        vmRequest.pMsg->setUserID(strUsername.c_str());
//
//                        strVmId = vmRequest.strVmId;
//                        if(strVmId.empty())
//                            strVmId="emptyVmId";
//
//                        if(!strVmId.empty())
//                            vmRequest.pMsg ->setStrVmId(strVmId.c_str());
//
//                        vmRequest.pMsg->setName(EXEC_VM_RENT_TIMEOUT);
//
//                        EV_INFO << "Scheduling Msg name " << vmRequest.pMsg << " at "<< simTime().dbl()+vmRequest.nRentTime_t2 <<endl;
//                        scheduleAt(simTime().dbl()+vmRequest.nRentTime_t2, vmRequest.pMsg);
                      }
                    else
                      {
                        vmRequest = userVmRequest.getVms(0);
                      }

                    appType = searchAppTypeById(strAppType);

                    if(appType != nullptr)
                      {
                        //Get the parameters
                        if(strAppType.compare(appType->getAppName())==0)
                          {
                            nTotalTime = TEMPORAL_calculateTotalTime(appType);

                            strAppName = userApp.strApp;

                            //Create new Message
                            pMsgFinish = new SM_UserAPP_Finish();
                            pMsgFinish->setUserID(strUsername.c_str());
                            pMsgFinish->setStrApp(strAppName.c_str());
                            pMsgFinish->setStrVmId(strVmId.c_str());
                            pMsgFinish->setNTotalTime(nTotalTime);
                            pMsgFinish->setName(EXEC_APP_END_SINGLE);

                            userApp.pMsgTimeout = pMsgFinish;
                            //userApp.vmId =  vmRequest.strVmId;

                            //Change status to running
                            userAPP_Rq->changeState(strAppName, strVmId, appRunning);
                            userAPP_Rq->changeStateByIndex(i, strAppName, appRunning);
                            userAPP_Rq->setVmIdByIndex(i, userApp.strIp, strVmId);

                            EV_INFO << "Scheduling time rental Msg, " << userApp.pMsgTimeout->getName() << endl;
                            scheduleAt(simTime().dbl()+nTotalTime, userApp.pMsgTimeout);
                            bHandle = true;
                          }
                      }
                  }
              }
            else
              {
                EV_INFO << "WARNING! [CloudProviderFirstFit] The user: " << strUsername << "has the application list empty!"<< endl;
                throw omnetpp::cRuntimeError("[CloudProviderFirstFit] The list of applications of a the user is empty!!");
              }
          }
        else
          {
            EV_INFO << "WARNING! [CloudProviderFirstFit] The user: " << strUsername << "has not previously registered!!"<< endl;
          }

        if(!bHandle)
          {
            userAPP_Rq->setResult(0);
            rejectAppRequest(userAPP_Rq);
          }
      }
    else
      {
        throw omnetpp::cRuntimeError("[CloudProviderFirstFit] Wrong userAPP_Rq. Nullpointer!!");
      }
}
Application* CloudProviderBase_firstBestFit::searchAppTypeById(std::string strAppType)
{
    Application* appTypeRet;
    bool bFound;
    int nIndex;

    appTypeRet = nullptr;
    bFound = false;
    nIndex = 0;

    EV_DEBUG << "searchAppTypeById - Init" << endl;

    while(!bFound && nIndex<appTypes.size())
    {
        appTypeRet = appTypes.at(nIndex);
        if(strAppType.compare(appTypeRet->getAppName())==0)
        {
            bFound = true;
        }

        EV_DEBUG << "searchAppTypeById - " << strAppType << " vs " << appTypeRet->getAppName() << " Found="<< bFound << endl;

        nIndex++;
    }

    if(!bFound)
        appTypeRet = nullptr;

    return appTypeRet;
}
int CloudProviderBase_firstBestFit::TEMPORAL_calculateTotalTime(Application* appType)
{
    int nInputDataSize, nOutputDataSize, nMIs, nIterations, nTotalTime;
    int nReadTime, nWriteTime, nProcTime;
    AppParameter* paramInputDataSize, *paramOutputDataSize, *paramMIs, *paramIterations;

    nTotalTime = nInputDataSize = nOutputDataSize = nMIs = nIterations = nTotalTime = 0;

    //TODO: Cuidado con esto a ver si no peta.
    //Esto es un apaño temporal para no ejecutarlo en los datacenters reales
    if(appType!=NULL && appType->getAppName().compare("AppDataIntensive")==0)
    {
        //DatasetInput
        paramInputDataSize = appType->getParameterByName("inputDataSize");
        paramOutputDataSize = appType->getParameterByName("outputDataSize");
        paramMIs = appType->getParameterByName("MIs");
        paramIterations = appType->getParameterByName("iterations");

        if(paramInputDataSize != nullptr)
        {
            nInputDataSize = std::stoi(paramInputDataSize->getValue());
        }
        if(paramOutputDataSize != nullptr)
        {
            nOutputDataSize = std::stoi(paramOutputDataSize->getValue());
        }
        if(paramMIs != nullptr)
        {
            nMIs = std::stoi(paramMIs->getValue());
        }
        if(paramIterations != nullptr)
        {
            nIterations = std::stoi(paramIterations->getValue());
        }

        //TODO: Esto está hecho a fuego con 1 sola app. Diversificar.
        //Leer 10 GB (transmision por red+carga en disco)
        nReadTime = nInputDataSize*1024/SPEED_R_DISK;

        //Escribir Gb (escritura en disco y envío por red);
        nWriteTime = nOutputDataSize*1024/SPEED_W_DISK;

        //Procesar 10GB
        nProcTime = nMIs/CPU_SPEED;

        nTotalTime = (nReadTime+nWriteTime+nProcTime)*nIterations;

        EV_INFO << "The total executing, R: " << nReadTime << " | W: " << nWriteTime << " | P: " << nProcTime << " | Total: " << nTotalTime<< endl;
    }
    else if(appType!=NULL && appType->getAppName().compare("otraApp"))
    {

    }

    return nTotalTime;
}
void CloudProviderBase_firstBestFit::notifySubscription(SM_UserVM* userVM_Rq)
{
    SM_UserVM_Finish* pMsgTimeout;
    EV_INFO << "Notifying request from user: " << userVM_Rq->getUserID() << endl;
    EV_INFO << "Last id gate: " <<userVM_Rq->getLastGateId()<< endl;

    //Fill the message
    userVM_Rq->setIsResponse(true);
    userVM_Rq->setOperation(SM_VM_Notify);
    userVM_Rq->setResult(SM_APP_Sub_Accept);

    //Cancel the timeout event
    pMsgTimeout = userVM_Rq->getTimeoutSubscribeMsg();
    if(pMsgTimeout != nullptr)
    {
        cancelAndDelete(pMsgTimeout);
        userVM_Rq->setTimeoutSubscribeMsg(nullptr);
    }

    //Send the values
    sendResponseMessage(userVM_Rq);
}
void CloudProviderBase_firstBestFit::timeoutSubscription(SM_UserVM* userVM_Rq)
{
    EV_INFO << "Notifying timeout from user:" << userVM_Rq->getUserID() << endl;
    EV_INFO << "Last id gate: " << userVM_Rq->getLastGateId() << endl;

    //Fill the message
    userVM_Rq->setIsResponse(true);
    userVM_Rq->setOperation(SM_VM_Notify);
    userVM_Rq->setResult(SM_APP_Sub_Timeout);

    //Send the values
    sendResponseMessage(userVM_Rq);
}
void CloudProviderBase_firstBestFit::storeVmSubscribe(SM_UserVM* userVM_Rq)
{
    double dMaxSubscribeTime;
    SM_UserVM_Finish* pMsg;
    std::string strUserName;

    if(userVM_Rq != nullptr)
    {
        strUserName = userVM_Rq->getUserID();
        dMaxSubscribeTime = userVM_Rq->getMaxSubscribetime(0);
        EV_INFO << "Subscribing the VM request from user:" << strUserName << " | max sub time: "<<dMaxSubscribeTime<<endl;

        pMsg = new SM_UserVM_Finish();
        pMsg->setName(USER_SUBSCRIPTION_TIMEOUT);
        pMsg->setUserID(userVM_Rq->getUserID());

        //Store the VM subscription until there exist the Vms necessaries
        userVM_Rq->setDStartSubscriptionTime(simTime().dbl());
        userVM_Rq->setTimeoutSubscribeMsg(pMsg);
        subscribeQueue.push_back(userVM_Rq);

        scheduleAt(simTime().dbl()+dMaxSubscribeTime, pMsg);
    }
}

void CloudProviderBase_firstBestFit::processResponseMessage (SIMCAN_Message *sm){

}

bool CloudProviderBase_firstBestFit::checkVmUserFit(SM_UserVM*& userVM_Rq)
{
    bool bRet, bAccepted;

    int nTotalRequestedCores;
    int nRequestedVms, nPrice, nAvailableCores, nTotalCores;
    std::string nodeIp, strUserName, strVmId;
    VM_Request vmRequest;


    bAccepted = bRet = true;
    if(userVM_Rq != nullptr)
    {
        nPrice = 10;
        nRequestedVms = userVM_Rq->getTotalVmsRequests();

        EV_DEBUG << "checkVmUserFit- Init" << endl;
        EV_DEBUG << "checkVmUserFit- checking for free space, " << nRequestedVms << " Vm(s) for the user" << userVM_Rq->getUserID() << endl;

        //Before starting the process, it is neccesary to check if the
        nTotalRequestedCores = calculateTotalCoresRequested(userVM_Rq);
        nAvailableCores = datacenterCollection->getTotalAvailableCores();
        nTotalCores = datacenterCollection->getTotalCores();

        if(nTotalRequestedCores<=nAvailableCores)
        {
            EV_DEBUG << "checkVmUserFit - There is available space: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;

            //Process all the VMs
            for(int i=0;i<nRequestedVms && bRet;i++)
            {
                EV_DEBUG << endl <<"checkVmUserFit - Trying to handle the VM: " << i << endl;

                //Get the VM request
                vmRequest = userVM_Rq->getVms(i);
                strUserName = userVM_Rq->getUserID();

                //Create and fill the noderesource  with the VMrequest
                NodeResourceRequest* pNode = new NodeResourceRequest();
                pNode->setUserName(strUserName);
                pNode->setMaxStartTimeT1(vmRequest.maxStartTime_t1);
                pNode->setRentTimeT2(vmRequest.nRentTime_t2);
                pNode->setMaxSubTimeT3(vmRequest.maxSubTime_t3);
                pNode->setMaxSubscriptionTimeT4(vmRequest.maxSubscriptionTime_t4);
                pNode->setVmId(vmRequest.strVmId);
                fillVmFeatures(vmRequest.strVmType, pNode);

                //Send the request to the DC
                bAccepted = datacenterCollection->handleVmRequest(pNode);

                //We need to know the price of the Node.
                userVM_Rq->createResponse(i, bAccepted, pNode->getStartTime(), pNode->getIp(), pNode->getPrice());
                bRet &= bAccepted;

                if(!bRet)
                {
                    for(int j=0;j<i;j++)
                    {
                        vmRequest = userVM_Rq->getVms(j);
                        strVmId = vmRequest.strVmId;
                        cancelAndDelete(vmRequest.pMsg);
                        datacenterCollection->freeVmRequest(strVmId);
                    }
                    EV_DEBUG << "checkVmUserFit - The VM: " << i << "has not been handled, not enough space, all the request of the user "<< strUserName <<"have been deleted"<< endl;
                }
                else
                {
                    //Getting VM and scheduling renting timeout
                    vmRequest.pMsg = new SM_UserVM_Finish();
                    vmRequest.pMsg->setUserID(strUsername.c_str());
                    strVmId = vmRequest.strVmId;
                    vmRequest.pMsg ->setStrVmId(strVmId.c_str());
                    vmRequest.pMsg->setName(EXEC_VM_RENT_TIMEOUT);

                    EV_INFO << "Scheduling Msg name " << vmRequest.pMsg << " at "<< simTime().dbl()+vmRequest.nRentTime_t2 <<endl;
                    scheduleAt(simTime()+SimTime(vmRequest.nRentTime_t2), vmRequest.pMsg);

                    //Update value
                    nAvailableCores = datacenterCollection->getTotalAvailableCores();
                    EV_DEBUG << "checkVmUserFit - The VM: " << i << " has been handled and stored sucessfully, available cores: "<< nAvailableCores << endl;
                }
            }
        }
        else
        {
            EV_DEBUG << "checkVmUserFit - There isnt enough space: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs "<< nAvailableCores <<endl;
            bRet = false;
        }

        //Update the data
        nTotalRequestedCores = calculateTotalCoresRequested(userVM_Rq);
        nAvailableCores = datacenterCollection->getTotalAvailableCores();
        nTotalCores = datacenterCollection->getTotalCores();

        if(bRet)
        {
            EV_DEBUG << "checkVmUserFit - Reserved space for: " << userVM_Rq->getUserID() << endl;
        }
        else
            EV_DEBUG << "checkVmUserFit - Unable to reserve space for: " << userVM_Rq->getUserID() << endl;

        EV_DEBUG << "checkVmUserFit - Updated space#: [" << userVM_Rq->getUserID() << "Requested: "<<nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;
    }
    else
    {
        EV_ERROR << "checkVmUserFit - WARNING!! nullpointer detected" <<endl;
        bRet = false;
    }

    EV_DEBUG << "checkVmUserFit- End" << endl;

    return bRet;
}
int CloudProviderBase_firstBestFit::getPriceByVmType(std::string strPrice)
{
    int nRet;
    VirtualMachine* pVmMachine;

    nRet=0;
    if(strPrice.size()>0)
    {
        for(int i=0;i<vmTypes.size();i++)
        {
            pVmMachine = vmTypes.at(i);
            if(strPrice.compare(pVmMachine->getType()))
            {
                nRet = pVmMachine->getCost();
            }
        }
    }

    return nRet;
}
void  CloudProviderBase_firstBestFit::acceptVmRequest(SM_UserVM* userVM_Rq)
{
    EV_INFO << "Accepting request from user:" << userVM_Rq->getUserID() << endl;

    if(acceptedUsersRqMap.find(userVM_Rq->getUserID()) == acceptedUsersRqMap.end())
    {
        //Accepting new user
        EV_INFO << "Registering new user in the system:" << userVM_Rq->getUserID() << endl;
        acceptedUsersRqMap[userVM_Rq->getUserID()] = *userVM_Rq;
    }

    //Fill the message
    //userVM_Rq->setIsAccept(true);
    userVM_Rq->setIsResponse(true);
    userVM_Rq->setOperation(SM_VM_Req_Rsp);
    userVM_Rq->setResult(SM_VM_Res_Accept);

    //Send the values
    sendResponseMessage(userVM_Rq);

}
void  CloudProviderBase_firstBestFit::acceptAppRequestWithTimeout(SM_UserAPP* userAPP_Rq)
{
    EV_INFO << "Sending accept to the user:" << userAPP_Rq->getUserID() << endl;

    //Fill the message
    userAPP_Rq->setIsResponse(true);
    userAPP_Rq->setOperation(SM_APP_Rsp);
    userAPP_Rq->setResult(SM_APP_Res_Timeout);

    //Send the values
    sendResponseMessage(userAPP_Rq);

}
void  CloudProviderBase_firstBestFit::acceptAppRequest(SM_UserAPP* userAPP_Rq)
{
    EV_INFO << "Sending accept to the user:" << userAPP_Rq->getUserID() << endl;

    //Fill the message
    userAPP_Rq->setIsResponse(true);
    userAPP_Rq->setOperation(SM_APP_Rsp);
    userAPP_Rq->setResult(SM_APP_Res_Accept);

    //Send the values
    sendResponseMessage(userAPP_Rq);

}
void  CloudProviderBase_firstBestFit::timeoutAppRequest(SM_UserAPP* userAPP_Rq)
{
    EV_INFO << "Sending timeout to the user:" << userAPP_Rq->getUserID() << endl;
    EV_INFO << "Last id gate: " << userAPP_Rq->getLastGateId() << endl;

    userAPP_Rq->setFinished(true);

    //Fill the message
    userAPP_Rq->setIsResponse(true);
    userAPP_Rq->setOperation(SM_APP_Rsp);
    userAPP_Rq->setResult(SM_APP_Res_Timeout);

    //Send the values
    sendResponseMessage(userAPP_Rq);

}
void  CloudProviderBase_firstBestFit::rejectVmRequest(SM_UserVM* userVM_Rq)
{
    //Create a request_rsp message

    EV_INFO << "Reject VM request from user:" << userVM_Rq->getUserID() << endl;

    userVM_Rq->setIsResponse(true);
    userVM_Rq->setOperation(SM_VM_Req_Rsp);
    userVM_Rq->setResult(SM_VM_Res_Reject);

    //Send the values
    sendResponseMessage(userVM_Rq);
}
void  CloudProviderBase_firstBestFit::rejectAppRequest(SM_UserAPP* userAPP_Rq)
{
    //Create a request_rsp message

    EV_INFO << "Rejecting app request from user:" << userAPP_Rq->getUserID() << endl;

    //Fill the message
    userAPP_Rq->setIsResponse(true);
    userAPP_Rq->setOperation(SM_APP_Req);
    userAPP_Rq->setResult(SM_APP_Res_Reject);

    //Send the values
    sendResponseMessage(userAPP_Rq);
}
void  CloudProviderBase_firstBestFit::fillVmFeatures(std::string strVmType, NodeResourceRequest*& pNode)
{
    VirtualMachine* pVmType;

    pVmType = searchVmPerType(strVmType);

    if(pVmType != NULL)
    {
        EV_INFO << "fillVmFeatures - Vm:" << strVmType << " cpus: "<< pVmType->getNumCores() << " mem: " << pVmType->getMemoryGb() <<endl;

        pNode->setTotalCpUs(pVmType->getNumCores());
        pNode->setTotalMemory(pVmType->getMemoryGb());
    }

}
VirtualMachine* CloudProviderBase_firstBestFit::searchVmPerType(std::string strVmType)
{
    VirtualMachine* pVM;
    int nIndex;
    bool bFound;

    bFound=false;
    nIndex=0;

    //It is neccesary to search the corresponding VM
    while(nIndex<vmTypes.size()&&!bFound)
    {
        pVM = vmTypes.at(nIndex);
        if(strVmType.compare(pVM->getType()) ==0)
        {
            EV_TRACE << "Vm found: " << pVM->getType() << " vs: "<< strVmType << endl;
            bFound = true;
        }
        nIndex++;
    }
    if(!bFound)
        pVM =NULL;

    return pVM;
}
int CloudProviderBase_firstBestFit::getTotalCoresByVmType(std::string strVmType)
{
    int nRet;
    VirtualMachine* pVmType;

    nRet=0;

    pVmType = searchVmPerType(strVmType);

    if(pVmType != NULL)
    {
        nRet = pVmType->getNumCores();
    }

    return nRet;
}
int CloudProviderBase_firstBestFit::calculateTotalCoresRequested(SM_UserVM* userVM_Rq)
{
    int nRet, nRequestedVms;
    VM_Request vmRequest;

    nRet=nRequestedVms=0;
    if(userVM_Rq != NULL)
    {
        nRequestedVms = userVM_Rq->getTotalVmsRequests();

        for(int i=0;i<nRequestedVms;i++)
        {
            vmRequest = userVM_Rq->getVms(i);

            nRet+=getTotalCoresByVmType(vmRequest.strVmType);
        }
    }
    EV_DEBUG << "User:" << userVM_Rq->getUserID() << " has requested: "<< nRet << " cores" << endl;

    return nRet;
}
