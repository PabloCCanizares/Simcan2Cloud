#include <algorithm>
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
    initializeSelfHandlers();
    initializeRequestHandlers();

    bFinished = false;
    scheduleAt(SimTime(), new cMessage(INITIAL_STAGE));
    EV_INFO << prettyFunc(__FILE__, __func__) << " - End" << endl;
}

/**
 * This method initializes the self message handlers
 */
void CloudProviderBase_firstBestFit::initializeSelfHandlers() {
    // ADAA
    selfHandlers[INITIAL_STAGE] = [this](cMessage *msg) -> void { return handleInitialStage(msg); };
    selfHandlers[EXEC_VM_RENT_TIMEOUT] = [this](cMessage *msg) -> void { return handleExecVmRentTimeout(msg); };
    selfHandlers[EXEC_APP_END] = [this](cMessage *msg) -> void { return handleAppExecEnd(msg); };
    selfHandlers[EXEC_APP_END_SINGLE] = [this](cMessage *msg) -> void { return handleAppExecEndSingle(msg); };
    selfHandlers[USER_SUBSCRIPTION_TIMEOUT] = [this](cMessage *msg) -> void { return manageSubscriptionTimeout(msg); };
    selfHandlers[MANAGE_SUBSCRIBTIONS] = [this](cMessage *msg) -> void { return handleManageSubscriptions(msg); };
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

    EV_INFO << prettyFunc(__FILE__, __func__) << " - Init" << endl;
    //Initialize
    nComputingRacks =  nTotalNodes = nDataCenter = nIndex=0;

    nDataCenter = dataCentersBase.size();

    //Go over the datacenter object: all racks
    if(nDataCenter>0)
    {
        EV_INFO << prettyFunc(__FILE__, __func__) << " - handling the DC: " <<  dataCentersToString() << endl;
        for(int nIndex=0;nIndex<nDataCenter;nIndex++)
        {
            EV_TRACE << prettyFunc(__FILE__, __func__) << " - loading dataCenter: " << nIndex << endl;
            pDataCenter = dataCentersBase.at(nIndex);
            if(pDataCenter != nullptr)
            {
                nComputingRacks = pDataCenter->getNumRacks(false);
                EV_TRACE << prettyFunc(__FILE__, __func__) << " - Number of computing racks: "<< nComputingRacks << endl;
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

    EV_INFO << prettyFunc(__FILE__, __func__) << " - Ende" << endl;
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

        cancelAndDeleteAppFinishMsgs(userApp, strVmId);
    }
}

////Todo: tiene una nota interna. Supongo que se refiere a eliminar el mensaje de otras apps que esten en otro estado.
void CloudProviderBase_firstBestFit::cancelAndDeleteAppFinishMsgs(SM_UserAPP* userApp, std::string strVmId)
{
    // Terminar este método, para cancelar el resto de eventos si se requiere.
    for (unsigned int nIndex=0; nIndex<userApp->getAppArraySize(); nIndex++)
    {
        APP_Request& userAppReq = userApp->getApp(nIndex);
        if (userAppReq.vmId.compare(strVmId)==0 && userAppReq.eState == appFinishedTimeout && userAppReq.pMsgTimeout!=nullptr)
        {
            cancelAndDelete(userAppReq.pMsgTimeout);
            userAppReq.pMsgTimeout = nullptr;
        }
    }
}

void CloudProviderBase_firstBestFit::handleInitialStage(cMessage* msg) { // ADAA
    EV_INFO << prettyFunc(__FILE__, __func__) << " - INITIAL_STAGE" << endl;
    scheduleAt(simTime() + SimTime(1), new cMessage(MANAGE_SUBSCRIBTIONS));
}

void CloudProviderBase_firstBestFit::handleManageSubscriptions(cMessage* msg)
{
    SM_UserVM* userVmSub;
    std::string strUsername;
    bool bFreeSpace;
    double dWaitingSub, dMaxSubtime;


    bFreeSpace = false;
    if (subscribeQueue.size() > 0)
      {
        EV_TRACE << "The queue of subscription: " << subscribeQueue.size()
                                << endl;

        //First of all, check the subscriptions timeouts
        for (int i = 0; i < subscribeQueue.size(); i++)
          {
            userVmSub = subscribeQueue.at(i);

            if (userVmSub != nullptr)
              {
                strUsername = userVmSub->getUserID();

                dWaitingSub = (simTime().dbl())
                                - (userVmSub->getDStartSubscriptionTime());
                dMaxSubtime = userVmSub->getMaxSubscribetime(0);

                if (dWaitingSub > dMaxSubtime)
                  {
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
                  }
                else
                  {
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
      }
    else
      {
        EV_TRACE << "The subscription queue is empty" << endl;
      }
}

void CloudProviderBase_firstBestFit::handleAppExecEndSingle(cMessage *msg) {
    SM_UserAPP_Finish *pUserAppFinish;
    SM_UserAPP *pUserApp;
    std::string strUsername,
                strVmId,
                strAppName,
                strIp;
    std::map<std::string, SM_UserAPP*>::iterator it;

    EV_INFO << prettyFunc(__FILE__, __func__) << " - EXEC_APP_END_SINGLE" << endl;
    if ((pUserAppFinish = dynamic_cast<SM_UserAPP_Finish *>(msg)))
      {
        strUsername = pUserAppFinish->getUserID();
        strVmId = pUserAppFinish->getStrVmId();
        strAppName = pUserAppFinish->getStrApp();
        EV_INFO << "The execution of the App [" << pUserAppFinish->getStrApp()
                               << " / " << pUserAppFinish->getStrVmId()
                               << "] launched by the user " << strUsername
                               << " has finished" << endl;
        //Used only for notify the user.
        it = handlingAppsRqMap.find(strUsername);
        if (it != handlingAppsRqMap.end())
          {
            pUserApp = it->second;

            if (pUserApp != nullptr)
              {
                pUserApp->increaseFinishedApps();
                //Check for a possible timeout
                if (!pUserApp->isFinishedKO(strAppName, strVmId))
                  {
                    EV_INFO << prettyFunc(__FILE__, __func__)
                    << " - Changing status of the application [ app: "
                    << strAppName << " | vmId: " << strVmId << endl;
                    pUserApp->printUserAPP();

                    pUserApp->changeState(strAppName, strVmId, appFinishedOK);
                    pUserApp->setEndTime(strAppName, strVmId, simTime().dbl());
                  }
                //if(userApp->getNFinishedApps() >= userApp->getAppArraySize())
                checkAllAppsFinished(pUserApp);

              }
            else
              {
                EV_INFO << prettyFunc(__FILE__, __func__)
                << " - WARNING! I cant found the apps corresponding with the user "
                << strUsername << endl;
              }
          }
        else
          {
            EV_INFO << prettyFunc(__FILE__, __func__)
            << " - WARNING! I cant found the apps corresponding with the user "
            << strUsername << endl;
          }
      }
    else
      {
        error ("%s - Unable to cast msg to SM_UserAPP_Finish*. Wrong msg name [%s]?", prettyFunc(__FILE__, __func__).c_str(), msg->getName());
      }
}

void CloudProviderBase_firstBestFit::checkAllAppsFinished(SM_UserAPP* pUserApp) {
    std::string strUsername;

    if (pUserApp != nullptr)
      {
        strUsername = pUserApp->getUserID();
        if (pUserApp->allAppsFinished())
              {
                if (pUserApp->allAppsFinishedOK())
                  {
                    EV_INFO << prettyFunc(__FILE__, __func__)
                    << " - All the apps corresponding with the user "
                    << strUsername
                    << " have finished successfully" << endl;

                    pUserApp->printUserAPP();

                    //Notify the user the end of the execution
                    acceptAppRequest(pUserApp);
                  }
                else
                  {
                    EV_INFO << prettyFunc(__FILE__, __func__)
                    << " - All the apps corresponding with the user "
                    << strUsername
                    << " have finished with some errors" << endl;

                    //Check the subscription queue
                    //updateSubsQueue();

                    if (!pUserApp->getFinished())
                        timeoutAppRequest(pUserApp);  //Notify the user the end of the execution
                  }

                //Delete the application on the hashmap
                handlingAppsRqMap.erase(strUsername);
              }
            else
              {
                EV_INFO << prettyFunc(__FILE__, __func__)
                        << " - Total apps finished: "
                        << pUserApp->getNFinishedApps() << " of "
                        << pUserApp->getAppArraySize() << endl;
              }
      }
    else
    {
        EV_INFO << prettyFunc(__FILE__, __func__)
        << " - WARNING! Null pointer parameter "
        << strUsername << endl;
    }

}

//TODO: asignar la vm que hace el timout al mensaje de la app. Duplicarlo y enviarlo.
void CloudProviderBase_firstBestFit::handleExecVmRentTimeout(cMessage *msg) {
    SM_UserAPP *pUserApp;

    std::string strUsername,
                strVmId,
                strAppName,
                strIp;

    bool bAlreadyFinished;

    SM_UserVM_Finish *pUserVmFinish;

    std::map<std::string, SM_UserAPP*>::iterator it;


    if ((pUserVmFinish = dynamic_cast<SM_UserVM_Finish*>(msg)))
      {
        EV_INFO << prettyFunc(__FILE__, __func__) << " - INIT" << endl;
        bAlreadyFinished = false;
        strUsername = pUserVmFinish->getUserID();
        strVmId = pUserVmFinish->getStrVmId();
        EV_INFO << "The rent of the VM [" << pUserVmFinish->getStrVmId()
                               << "] launched by the user " << strUsername
                               << " has finished" << endl;
        //Check the Application status
        it = handlingAppsRqMap.find(strUsername);
        if (it != handlingAppsRqMap.end())
          {
            pUserApp = it->second;

            //Check the application status
            if (pUserApp != nullptr)
              {

                EV_INFO << "Last id gate: " << pUserApp->getLastGateId() << endl;
                EV_INFO
                << "Checking the status of the applications which are running over this VM"
                << endl;

                //Abort the running applications
                if (!pUserApp->allAppsFinished(strVmId))
                  {
                    EV_INFO << "Aborting running applications" << endl;
                    abortAllApps(pUserApp, strVmId);
                    timeoutAppRequest(pUserApp, strVmId);
                    checkAllAppsFinished(pUserApp);
                  }


//                else
//                  {
//                    EV_INFO << "All the applications have already finished" << endl;
//                    bAlreadyFinished = true;
//                  }


                //Check if all the applications of the user have finished
//                if (pUserApp->allAppsFinished() && !pUserApp->getFinished() && !bAlreadyFinished)
//                  {
//                    //Notify the user the end of the execution
//                    EV_INFO << prettyFunc(__FILE__, __func__) << " - EXEC_VM_RENT_TIMEOUT Init" << endl;
//
//                    //if so, notify this.
//                    //timeoutAppRequest(pUserApp);
//                    //if so. Delete the application on the hashmap
//                    handlingAppsRqMap.erase(strUsername);
//                  }
              }
          }

        EV_INFO << "Freeing resources..." << endl;

        //Free the VM resources
        freeVm(strVmId);

        //Check the subscription queue
        updateSubsQueue();
      }
    else
      {
        error ("%s - Unable to cast msg to SM_UserVM_Finish*. Wrong msg name [%s]?", prettyFunc(__FILE__, __func__).c_str(), msg->getName());
      }
}

std::string CloudProviderBase_firstBestFit::prettyFunc(const char *fileName, const char *funcName)
{
    const char *lastBar;

    char *className;

    int startPos,
        length;

    std::string strFile,
                prettyName;

    lastBar = strrchr(fileName, '/');
    if (lastBar == nullptr)
        lastBar = strrchr(fileName, '\\');

    startPos = lastBar - fileName + 1;
    length = strrchr(fileName, '.') - fileName - startPos;
    strFile = fileName;
    prettyName = strFile.substr(startPos, length) + "::" + funcName;

    return prettyName;
}

void CloudProviderBase_firstBestFit::handleAppExecEnd(cMessage *msg) {
    std::string strUsername;
    SM_UserAPP *userAPP_Rq;

    EV_INFO << prettyFunc(__FILE__, __func__) << " - EXEC_APP_END" << endl;
    if ((userAPP_Rq = dynamic_cast<SM_UserAPP*>(msg)))
      {
        strUsername = userAPP_Rq->getUserID();
        EV_INFO << "The execution of all the Apps launched by the user " << strUsername
                       << " have finished" << endl;
        //Check the subscription queue
        updateSubsQueue();
        //Notify the user the end of the execution
        acceptAppRequest(userAPP_Rq);
      }
    else
      {
        error ("%s - Unable to cast msg to SM_UserAPP*. Wrong msg name [%s]?", prettyFunc(__FILE__, __func__).c_str(), msg->getName());
      }
}

void CloudProviderBase_firstBestFit::processSelfMessage (cMessage *msg){
    std::map<std::string, std::function<void(cMessage*)>>::iterator it;

    EV_INFO << prettyFunc(__FILE__, __func__) << " - Received Request Message" << endl;

    it = selfHandlers.find(msg->getName());

    if (it != selfHandlers.end())
        it->second(msg);  //Perform the operations...
    else
        error ("Unknown self message [%s]", msg->getName());

    cancelAndDelete(msg);

    EV_TRACE << prettyFunc(__FILE__, __func__) << " - End" << endl;
}

void CloudProviderBase_firstBestFit::manageSubscriptionTimeout(cMessage *msg)
{
    int nIndex;
    double dWaitingSub, dMaxSubTime;
    SM_UserVM_Finish *pUserSubFinish;
    SM_UserVM *userVmSub;
    std::string strUsername;

    EV_INFO << prettyFunc(__FILE__, __func__) << " - RECEIVED TIMEOUT " << endl;
    EV_TRACE << "manageSubscriptionTimeout - Init" << endl;

    nIndex = 0;
    dWaitingSub = dMaxSubTime = 0;
    userVmSub = nullptr;

    if((pUserSubFinish = dynamic_cast<SM_UserVM_Finish*>(msg)))
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
                  {
                    EV_TRACE << "The subscription of the user:  " << strUsername << " has time waiting!"
                    << simTime().dbl()<< " - " <<
                    userVmSub->getDStartSubscriptionTime() << " = | " << dWaitingSub << " vs " << dMaxSubTime  << endl;
                  }
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
    else
      {
        error ("%s - Unable to cast msg to SM_UserVM_Finish*. Wrong msg name [%s]?", prettyFunc(__FILE__, __func__).c_str(), msg->getName());
      }
    EV_TRACE << prettyFunc(__FILE__, __func__) << " - USER_SUBSCRIPTION_TIMEOUT End" << endl;
}

int CloudProviderBase_firstBestFit::searchUserInSubQueue(std::string strUsername)
{
    int nRet, nIndex;
    SM_UserVM* pUserVM;
    bool bFound;

    EV_TRACE << prettyFunc(__FILE__, __func__) << " - Init" << endl;
    nRet = -1;
    bFound= false;
    nIndex=0;

    EV_TRACE << __func__ << " - Searching for user: " << strUsername << endl;
    if(strUsername.size()>0)
    {
        EV_TRACE << __func__ << " - Queue size: " << subscribeQueue.size() << endl;
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
                    EV_TRACE << __func__ << " - [nIndex: " << nIndex << " " << strUsername << " vs " <<pUserVM->getUserID()<<" ]" << endl;
                    nIndex++;
                }
            }
            else
            {
                EV_INFO << __func__ << " - WARNING! null pointer at position[nIndex: " << nIndex << " ]" << endl;
                nIndex++;
            }
        }
    }
    if(bFound)
        nRet = nIndex;

    EV_TRACE << prettyFunc(__FILE__, __func__) << " - End" << endl;

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
    std::map<std::string, SM_UserVM>::iterator it;
    VM_Request vmRequest;

    EV_TRACE << prettyFunc(__FILE__, __func__) << " - Init" << endl;

    //Mark the user VMs as free
    it = acceptedUsersRqMap.find(strUsername);
    if(it != acceptedUsersRqMap.end())
      {
        pVmRequest = it->second;

        for(int i=0;i<pVmRequest.getVmsArraySize();i++)
          {
            vmRequest = pVmRequest.getVms(i);
            strVmId = vmRequest.strVmId;
            freeVm(strVmId);
          }

        acceptedUsersRqMap.erase(strUsername);
      }

    EV_TRACE << prettyFunc(__FILE__, __func__) << " - End" << endl;
}

void CloudProviderBase_firstBestFit::freeVm(std::string strVmId)
{
    EV_TRACE << prettyFunc(__FILE__, __func__) << " - Init" << endl;
    EV_TRACE << prettyFunc(__FILE__, __func__) << " - Releasing the Vm:  "<< strVmId << endl;

    if(datacenterCollection->freeVmRequest(strVmId))
        EV_DEBUG << "the Vm has been released sucessfully: "<< strVmId << endl;
    else //Error freeing the VM
        EV_INFO << "Error releasing the VM: "<< strVmId << endl;

    EV_TRACE << prettyFunc(__FILE__, __func__) << " - End" << endl;
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

    EV_INFO << prettyFunc(__FILE__, __func__) << " - Init "<< endl;

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

                EV_TRACE << prettyFunc(__FILE__, __func__) << " - Inserting node "<< j << " at datacenter: " << nIndex << endl;
                //Insert the node info into the corresponding DC
                datacenterCollection->insertNode(nIndex, pNodeResInfo);
            }
        }
    }

    EV_INFO << prettyFunc(__FILE__, __func__) << " - End "<< endl;
}

void CloudProviderBase_firstBestFit::handleVmRequestFits(SIMCAN_Message *sm)
{
    SM_UserVM *userVM_Rq;

    userVM_Rq = dynamic_cast<SM_UserVM*>(sm);
    EV_INFO << prettyFunc(__FILE__, __func__) << " - Handle VM_Request"  << endl;

    if(userVM_Rq != nullptr)
      {
        userVM_Rq->printUserVM();
        //Check if is a VmRequest or a subscribe
        if (checkVmUserFit(userVM_Rq))
            acceptVmRequest(userVM_Rq);
        else
            rejectVmRequest(userVM_Rq);
      }
    else
      {
        throw omnetpp::cRuntimeError(("[" + prettyFunc(__FILE__, __func__) + "] Wrong userVM_Rq. Null pointer or bad operation code!").c_str());
      }
}

void CloudProviderBase_firstBestFit::handleVmSubscription(SIMCAN_Message *sm)
{
    SM_UserVM *userVM_Rq;

    userVM_Rq = dynamic_cast<SM_UserVM*>(sm);
    EV_INFO << prettyFunc(__FILE__, __func__) << " - Received Subscribe operation"  << endl;

    if(userVM_Rq != nullptr)
      {
        if (checkVmUserFit(userVM_Rq))
            notifySubscription(userVM_Rq);
        else
            storeVmSubscribe(userVM_Rq); //Store the vmRequest
      }
    else
      {
        throw omnetpp::cRuntimeError(("[" + prettyFunc(__FILE__, __func__) + "] Wrong userVM_Rq. Null pointer or bad operation code!").c_str());
      }
}

void CloudProviderBase_firstBestFit::processRequestMessage (SIMCAN_Message *sm)
{
    SM_CloudProvider_Control* userControl;
    std::map<int, std::function<void(SIMCAN_Message*)>>::iterator it;

    EV_INFO << prettyFunc(__FILE__, __func__) << " - Received Request Message" << endl;

    it = requestHandlers.find(sm->getOperation());

    if (it == requestHandlers.end())
      {
        userControl = dynamic_cast<SM_CloudProvider_Control *>(sm);

        if(userControl != nullptr)
          {
            EV_INFO << prettyFunc(__FILE__, __func__) << " - Received end of party"  << endl;
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
    SM_UserAPP *userAPP_Rq;

    bool bHandle;

    std::string strUsername,
                strVmId,
                strIp,
                strAppName;

    SimTime appStartTime,
            appExecutedTime,
            nTotalTime;

    Application *appType;

    SM_UserVM userVmRequest;

    SM_UserAPP_Finish *pMsgFinish;

    VM_Request vmRequest;

    APP_Request userApp;

    VirtualMachine *vmType;

    std::map<std::string, SM_UserVM>::iterator it;

    EV_INFO << prettyFunc(__FILE__, __func__) << " - Handle AppRequest"  << endl;
    userAPP_Rq = dynamic_cast<SM_UserAPP *>(sm);

    if(userAPP_Rq != nullptr)
      {
        //Las aplicaciones estan relacionadas con las VM
        //Hay que relacionar la APP con la VM para asi poder terminar con ella.
        bHandle = false;

        //APPRequest
        userAPP_Rq->printUserAPP();

        strUsername = userAPP_Rq->getUserID();
        it = acceptedUsersRqMap.find(strUsername);

        if(it != acceptedUsersRqMap.end())
          {
            userVmRequest = it->second;

            if(handlingAppsRqMap.find(strUsername) == handlingAppsRqMap.end())
              {
                //Registering the appRq
                handlingAppsRqMap[strUsername] = userAPP_Rq;
              }
            EV_INFO << "Executing the VMs corresponding with the user: " << strUsername << " | Total: "<< userVmRequest.getVmsArraySize() << endl;

            //First step consists in calculating the total units of time spent in executing the application.
            if(userAPP_Rq->getArrayAppsSize() > 0)
              {
                for(int j = 0; j < userVmRequest.getVmsArraySize(); j++)
                  {
                    //Getting VM and scheduling renting timeout
                    vmRequest = userVmRequest.getVms(j);
                    //scheduleRentingTimeout(EXEC_VM_RENT_TIMEOUT, strUsername, vmRequest.strVmId, vmRequest.nRentTime_t2);

                    strVmId = vmRequest.strVmId;
                    vmType = searchVmPerType(userVmRequest.getVmRequestType(j));

                    double totalTimePerCore[vmType->getNumCores()];

                    for (int i = 0; i < vmType->getNumCores(); i++)
                        totalTimePerCore[i] = 0;

                    for(int i = 0; i < userAPP_Rq->getAppArraySize(); i++)
                      {
                        //Get the app
                        userApp = userAPP_Rq->getApp(i);

                        if (userApp.eState != appFinishedOK && userApp.eState != appFinishedError) {


                            if(strVmId.compare(userApp.vmId)==0)
                              {
                                appType = searchAppTypeById(userApp.strAppType);

                                if(appType != nullptr)
                                  {
                                    //Assing the app to core with less utilization time
                                    //std::sort(totalTimePerCore, totalTimePerCore+vmType->getNumCores());
                                    int minIndex = std::min_element(totalTimePerCore, totalTimePerCore+vmType->getNumCores()) - totalTimePerCore;
                                    nTotalTime = SimTime(TEMPORAL_calculateTotalTime(appType));
                                    appStartTime = simTime() + SimTime(totalTimePerCore[minIndex]);

                                    if (userApp.eState == appFinishedTimeout)
                                      {
                                        userAPP_Rq->decreaseFinishedApps();
                                        appExecutedTime = SimTime(userApp.finishTime - userApp.startTime);
                                        if (appExecutedTime>0)
                                          {
                                            nTotalTime -= appExecutedTime;
                                            appStartTime -= appExecutedTime;
                                          }


                                      }



                                    totalTimePerCore[minIndex] = totalTimePerCore[minIndex] + nTotalTime.dbl();

                                    strAppName = userApp.strApp;

                                    //Create new Message
                                    pMsgFinish = scheduleAppTimeout(EXEC_APP_END_SINGLE, strUsername, strAppName, strVmId, totalTimePerCore[minIndex]);

                                    userAPP_Rq->getApp(i).startTime = appStartTime.dbl();
                                    userAPP_Rq->getApp(i).pMsgTimeout = pMsgFinish;
                                    //userApp.vmId =  vmRequest.strVmId;

                                    //Change status to running
                                    userAPP_Rq->changeState(strAppName, strVmId, appRunning);
                                    userAPP_Rq->changeStateByIndex(i, strAppName, appRunning);
                                    userAPP_Rq->setVmIdByIndex(i, userApp.strIp, strVmId);


                                    bHandle = true;

                                  }
                              }
                            else
                            {
                                error ("%s - Unable to find App. Wrong AppType [%s]?", prettyFunc(__FILE__, __func__).c_str(), appType);
                            }
                          }
                      }
                  }
              }
            else
              {
                EV_INFO << "WARNING! [" << prettyFunc(__FILE__, __func__) << "] The user: " << strUsername << "has the application list empty!"<< endl;
                throw omnetpp::cRuntimeError(("[" + prettyFunc(__FILE__, __func__) + "] The list of applications of a the user is empty!!").c_str());
              }
          }
        else
          {
            EV_INFO << "WARNING! [" << prettyFunc(__FILE__, __func__) << "] The user: " << strUsername << "has not previously registered!!"<< endl;
          }

        if(!bHandle)
          {
            userAPP_Rq->setResult(0);
            rejectAppRequest(userAPP_Rq);
          }
      }
    else
      {
        throw omnetpp::cRuntimeError(("[" + prettyFunc(__FILE__, __func__) + "] Wrong userAPP_Rq. Nullpointer!!").c_str());
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

    while(!bFound && nIndex < appTypes.size())
      {
        appTypeRet = appTypes.at(nIndex);
        if(strAppType.compare(appTypeRet->getAppName()) == 0)
            bFound = true;

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
    EV_INFO << "Last id gate: " << userVM_Rq->getLastGateId() << endl;

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
    SM_UserVM_Finish *pMsg;
    std::string strUserName;

    if(userVM_Rq != nullptr)
    {
        strUserName = userVM_Rq->getUserID();
        dMaxSubscribeTime = userVM_Rq->getMaxSubscribetime(0);
        EV_INFO << "Subscribing the VM request from user:" << strUserName << " | max sub time: " << dMaxSubscribeTime<<endl;

        pMsg = scheduleRentingTimeout(USER_SUBSCRIPTION_TIMEOUT, userVM_Rq->getUserID(), "", dMaxSubscribeTime);

        //Store the VM subscription until there exist the Vms necessaries
        userVM_Rq->setDStartSubscriptionTime(simTime().dbl());
        userVM_Rq->setTimeoutSubscribeMsg(pMsg);
        subscribeQueue.push_back(userVM_Rq);

        //scheduleAt(simTime() + SimTime(dMaxSubscribeTime), pMsg);
    }
}

void CloudProviderBase_firstBestFit::processResponseMessage (SIMCAN_Message *sm){

}

NodeResourceRequest* CloudProviderBase_firstBestFit::generateNode(std::string strUserName, VM_Request vmRequest)
{
    NodeResourceRequest *pNode = new NodeResourceRequest();

    pNode->setUserName(strUserName);
    pNode->setMaxStartTimeT1(vmRequest.maxStartTime_t1);
    pNode->setRentTimeT2(vmRequest.nRentTime_t2);
    pNode->setMaxSubTimeT3(vmRequest.maxSubTime_t3);
    pNode->setMaxSubscriptionTimeT4(vmRequest.maxSubscriptionTime_t4);
    pNode->setVmId(vmRequest.strVmId);
    fillVmFeatures(vmRequest.strVmType, pNode);

    return pNode;
}

SM_UserVM_Finish* CloudProviderBase_firstBestFit::scheduleRentingTimeout (std::string name, std::string strUserName, std::string strVmId, double rentTime)
{
    SM_UserVM_Finish *pMsg = new SM_UserVM_Finish();

    pMsg->setUserID(strUserName.c_str());
    pMsg->setName(name.c_str());

    if(!strVmId.empty())
        pMsg ->setStrVmId(strVmId.c_str());

    EV_INFO << "Scheduling Msg name " << pMsg << " at "<< simTime().dbl() + rentTime << endl;
    scheduleAt(simTime() + SimTime(rentTime), pMsg);

    return pMsg;
}

SM_UserAPP_Finish* CloudProviderBase_firstBestFit::scheduleAppTimeout (std::string name, std::string strUserName, std::string strAppName, std::string strVmId, double totalTime)
{
    SM_UserAPP_Finish *pMsgFinish = new SM_UserAPP_Finish();

    pMsgFinish->setUserID(strUserName.c_str());
    pMsgFinish->setStrApp(strAppName.c_str());

    if(!strVmId.empty())
        pMsgFinish ->setStrVmId(strVmId.c_str());

    pMsgFinish->setNTotalTime(totalTime);
    pMsgFinish->setName(name.c_str());

    EV_INFO << "Scheduling time rental Msg, " << strAppName << " at " << simTime().dbl() + totalTime << "s" << endl;
    scheduleAt(simTime() + SimTime(totalTime), pMsgFinish);

    return pMsgFinish;
}

void CloudProviderBase_firstBestFit::clearVMReq (SM_UserVM*& userVM_Rq, int lastId)
{
    VM_Request vmRequest;

    for(int i = 0; i < lastId ; i++)
      {
        vmRequest = userVM_Rq->getVms(i);
        cancelAndDelete(vmRequest.pMsg);
        datacenterCollection->freeVmRequest(vmRequest.strVmId);
      }
}

bool CloudProviderBase_firstBestFit::checkVmUserFit(SM_UserVM*& userVM_Rq)
{
    bool bRet,
         bAccepted;

    int nTotalRequestedCores,
        nRequestedVms,
        nPrice,
        nAvailableCores,
        nTotalCores;

    std::string nodeIp,
                strUserName,
                strVmId;

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

        if(nTotalRequestedCores<=nAvailableCores)
          {
            nTotalCores = datacenterCollection->getTotalCores();
            EV_DEBUG << "checkVmUserFit - There is available space: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;

            strUserName = userVM_Rq->getUserID();
            //Process all the VMs
            for(int i=0;i<nRequestedVms && bRet;i++)
              {
                EV_DEBUG << endl <<"checkVmUserFit - Trying to handle the VM: " << i << endl;

                //Get the VM request
                vmRequest = userVM_Rq->getVms(i);

                //Create and fill the noderesource  with the VMrequest
                NodeResourceRequest *pNode = generateNode(strUserName, vmRequest);

                //Send the request to the DC
                bAccepted = datacenterCollection->handleVmRequest(pNode);

                //We need to know the price of the Node.
                userVM_Rq->createResponse(i, bAccepted, pNode->getStartTime(), pNode->getIp(), pNode->getPrice());
                bRet &= bAccepted;

                if(!bRet)
                  {
                    clearVMReq (userVM_Rq, i);
                    EV_DEBUG << "checkVmUserFit - The VM: " << i << "has not been handled, not enough space, all the request of the user " << strUserName << "have been deleted" << endl;
                  }
                else
                  {
                    //Getting VM and scheduling renting timeout
                    vmRequest.pMsg = scheduleRentingTimeout(EXEC_VM_RENT_TIMEOUT, strUserName, vmRequest.strVmId, vmRequest.nRentTime_t2);

                    //Update value
                    nAvailableCores = datacenterCollection->getTotalAvailableCores();
                    EV_DEBUG << "checkVmUserFit - The VM: " << i << " has been handled and stored sucessfully, available cores: "<< nAvailableCores << endl;
                  }
              }
            //Update the data
            nAvailableCores = datacenterCollection->getTotalAvailableCores();
            nTotalCores = datacenterCollection->getTotalCores();

            EV_DEBUG << "checkVmUserFit - Updated space#: [" << userVM_Rq->getUserID() << "Requested: "<< nTotalRequestedCores << " vs Available [" << nAvailableCores << "/" << nTotalCores << "]" << endl;
          }
        else
          {
            EV_DEBUG << "checkVmUserFit - There isnt enough space: [" << userVM_Rq->getUserID() << nTotalRequestedCores << " vs Available [" << nAvailableCores << "/" << nTotalCores << "]" << endl;
            bRet = false;
          }

        if(bRet)
            EV_DEBUG << "checkVmUserFit - Reserved space for: " << userVM_Rq->getUserID() << endl;
        else
            EV_DEBUG << "checkVmUserFit - Unable to reserve space for: " << userVM_Rq->getUserID() << endl;
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
void  CloudProviderBase_firstBestFit::timeoutAppRequest(SM_UserAPP* userAPP_Rq, std::string strVmId)
{
    EV_INFO << "Sending timeout to the user:" << userAPP_Rq->getUserID() << endl;
    EV_INFO << "Last id gate: " << userAPP_Rq->getLastGateId() << endl;

    SM_UserAPP* userAPP_Res = userAPP_Rq->dup();

    userAPP_Res->setVmId(strVmId.c_str());
    userAPP_Res->setFinished(true);

    //Fill the message
    userAPP_Res->setIsResponse(true);
    userAPP_Res->setOperation(SM_APP_Rsp);
    userAPP_Res->setResult(SM_APP_Res_Timeout);

    //Send the values
    sendResponseMessage(userAPP_Res);

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
