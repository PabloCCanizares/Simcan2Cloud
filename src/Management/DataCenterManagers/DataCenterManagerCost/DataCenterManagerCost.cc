#include "DataCenterManagerCost.h"

#include "Management/utils/LogUtils.h"


Define_Module(DataCenterManagerCost);

DataCenterManagerCost::~DataCenterManagerCost(){
}

void DataCenterManagerCost::initialize(){

    checkReservedFirst = par("checkReservedFirst");

    // Init super-class
    DataCenterManager::initialize();
}

void DataCenterManagerCost::initializeRequestHandlers() {
    DataCenterManager::initializeRequestHandlers();

    requestHandlers[SM_APP_Req_Resume] = [this](SIMCAN_Message *msg) -> void { return handleExtendVmAndResumeExecution(msg); };
    requestHandlers[SM_APP_Req_End] = [this](SIMCAN_Message *msg) -> void { return handleEndVmAndAbortExecution(msg); };
}

void DataCenterManagerCost::parseConfig() {
    int result;

        // Init module parameters
        showApps = par ("showApps");

        // Parse application list
        result = parseAppList();

        // Something goes wrong...
        if (result == SC_ERROR){
            error ("Error while parsing application list.");
        }
        else if (showApps){
            EV_DEBUG << appsToString ();
        }

        // Init module parameters
        showUsersVms = par ("showUsersVms");

        // Parse VMs list
        result = parseVmsList();

        // Something goes wrong...
        if (result == SC_ERROR){
         error ("Error while parsing VMs list");
        }
        else if (showUsersVms){
           EV_DEBUG << vmsToString ();
        }

        // Init module parameters
        showSlas = par ("showSlas");

        // Parse sla list
        result = parseSlasList();

        //Something goes wrong...
        if (result == SC_ERROR){
           error ("Error while parsing slas list");
        }
        else if (showSlas){
           EV_DEBUG << slasToString ();
        }

        // Parse user list
        result = parseUsersList();

        // Something goes wrong...
        if (result == SC_ERROR){
           error ("Error while parsing users list");
        }
        else if (showUsersVms){
           EV_DEBUG << usersToString ();
        }
}

int DataCenterManagerCost::parseDataCenterConfig (){
    int result;
    const char *dataCentersConfigChr;

    dataCentersConfigChr= par ("dataCenterConfig");
    DataCenterReservationConfigParser dataCenterParser(dataCentersConfigChr);
    result = dataCenterParser.parse();
    if (result == SC_OK) {
        dataCentersBase = dataCenterParser.getResult();
    }
    return result;
}

int DataCenterManagerCost::parseSlasList (){
    int result;
    const char *slaListChr;

    slaListChr= par ("slaList");
    SlaListParser slaParser(slaListChr, &vmTypes);
    result = slaParser.parse();
    if (result == SC_OK) {
        slaTypes = slaParser.getResult();
    }
    return result;
}

int DataCenterManagerCost::parseUsersList (){
    int result;
    const char *userListChr;

    userListChr= par ("userList");
    UserPriorityListParser userParser(userListChr, &vmTypes, &appTypes, &slaTypes);
    result = userParser.parse();
    if (result == SC_OK) {
        userTypes = userParser.getResult();
    }
    return result;
}

std::string DataCenterManagerCost::slasToString (){

    std::ostringstream info;
    int i;

        // Main text for the users of this manager
        info << std::endl << slaTypes.size() << " Slas parsed from ManagerBase in " << getFullPath() << endl << endl;

        for (i=0; i<slaTypes.size(); i++){
            info << "\tSla[" << i << "]  --> " << slaTypes.at(i)->toString() << endl;
        }

        info << "---------------- End of parsed Slas in " << getFullPath() << " ----------------" << endl;

    return info.str();
}

int DataCenterManagerCost::initDataCenterMetadata (){
    int result = SC_OK;
    DataCenterReservation *pDataCenterBase = dynamic_cast<DataCenterReservation*>(dataCentersBase[0]);
    cModule *pRackModule, *pBoardModule, *pNodeModule;
    int numBoards, numNodes, numCores, numTotalCores = 0;
    int numReserverNodes = pDataCenterBase->getReservedNodes();

    for (int nRackIndex=0; nRackIndex < pDataCenterBase->getNumRacks(false); nRackIndex++ ) {
        Rack* pRackBase = pDataCenterBase->getRack(nRackIndex,false);
        //Generate rack name in the data center
        string strRackName = "rackCmp_" + pRackBase->getRackInfo()->getName();
        pRackModule = getParentModule()->getSubmodule(strRackName.c_str(), nRackIndex);

        numBoards =  pRackModule->par("numBoards");

        for (int nBoardIndex=0; nBoardIndex < numBoards; nBoardIndex++) {
            pBoardModule = pRackModule->getSubmodule("board", nBoardIndex);
            numNodes = pBoardModule->par("numBlades");
            numTotalCores+=numNodes;

            for (int nNodeIndex=0; nNodeIndex < numNodes; nNodeIndex++) {
                pNodeModule = pBoardModule->getSubmodule("blade", nNodeIndex);

                if (numReserverNodes>0) {
                    storeReservedNodeMetadata(pNodeModule);
                    numReserverNodes--;
                } else {
                    storeNodeMetadata(pNodeModule);
                }

            }
        }
    }

    nTotalCores = nTotalAvailableCores = numTotalCores;

    return result;
}

int DataCenterManagerCost::storeReservedNodeMetadata(cModule *pNodeModule) {
    int result = SC_OK;
    cModule *pHypervisorModule;
    Hypervisor *pHypervisor;
    int numCores;

    pHypervisorModule = pNodeModule->getSubmodule("osModule")->getSubmodule("hypervisor");

    numCores = pNodeModule->par("numCpuCores");

    pHypervisor = check_and_cast<Hypervisor*>(pHypervisorModule);

    simtime_t **startTimeArray = new simtime_t *[numCores];
    simtime_t *timerArray = new simtime_t[numCores];
    for (int i=0; i<numCores;i++) {
        startTimeArray[i] = nullptr;
        timerArray[i] = SimTime();
    }


    //Store hypervisor pointers by number of cores
    mapHypervisorPerNodesReserved[numCores].push_back(pHypervisor);
    //Initialize cpu utilization timers
    mapCpuUtilizationTimePerHypervisor[pHypervisorModule->getFullPath()] = std::make_tuple(numCores, startTimeArray, timerArray);

    return result;
}

Hypervisor* DataCenterManagerCost::selectNode (SM_UserVM*& userVM_Rq, const VM_Request& vmRequest){
    CloudUserPriority *pCloudUser = dynamic_cast<CloudUserPriority*>(findUserTypeById(userVM_Rq->getUserID()));
    Hypervisor *pHypervisor = nullptr;
    SM_UserVM_Cost *userVM_Rq_Cost = dynamic_cast<SM_UserVM_Cost*>(userVM_Rq);

    if (pCloudUser == nullptr) return nullptr;

    if (!checkReservedFirst) {
        pHypervisor = DataCenterManager::selectNode (userVM_Rq, vmRequest);
    }

    if (pHypervisor == nullptr && pCloudUser->getPriorityType() == Priority)  {
        pHypervisor = selectNodeReserved(userVM_Rq_Cost, vmRequest);
    }

    if (pHypervisor == nullptr && checkReservedFirst) {
        pHypervisor = DataCenterManager::selectNode (userVM_Rq, vmRequest);
    }

     return pHypervisor;
}

Hypervisor* DataCenterManagerCost::selectNodeReserved (SM_UserVM_Cost*& userVM_Rq, const VM_Request& vmRequest){
    VirtualMachine *pVMBase;
    Hypervisor *pHypervisor = nullptr;
    NodeResourceRequest *pResourceRequest;
    int numCoresRequested, numNodeTotalCores, numAvailableCores;
    std::map<int, std::vector<Hypervisor*>>::iterator itMap;
    std::vector<Hypervisor*> vectorHypervisor;
    std::vector<Hypervisor*>::iterator itVector;
    bool bHandled;
    string strUserName;

    if (userVM_Rq==nullptr) return nullptr;

    strUserName = userVM_Rq->getUserID();

    pVMBase = findVirtualMachine(vmRequest.strVmType);
    numCoresRequested = pVMBase->getNumCores();

    bHandled = false;
    for (itMap = mapHypervisorPerNodesReserved.begin(); itMap != mapHypervisorPerNodesReserved.end() && !bHandled; ++itMap){
        numNodeTotalCores = itMap->first;
        if (numNodeTotalCores >= numCoresRequested) {
            vectorHypervisor = itMap->second;
            for (itVector = vectorHypervisor.begin(); itVector != vectorHypervisor.end() && !bHandled; ++itVector) {
                pHypervisor = *itVector;
                numAvailableCores = pHypervisor->getAvailableCores();
                if (numAvailableCores >= numCoresRequested) {
                    pResourceRequest = generateNode(strUserName, vmRequest);
                    // TODO: Probablemente sea mejor mover esto al hypervisor. La asignaci�n al map y que sea el hypervisor el que controle a que VM va.
                    // TODO: Finalmente deber�a devolver la IP del nodo y que el mensaje de la App llegue al nodo.
                    cModule *pVmAppVectorModule = pHypervisor->allocateNewResources(pResourceRequest);
                    if (pVmAppVectorModule!=nullptr) {
                        updateCpuUtilizationTimeForHypervisor(pHypervisor);
                        mapAppsVectorModulePerVm[vmRequest.strVmId] = pVmAppVectorModule;
                        int numMaxApps = pVmAppVectorModule->par("numApps");
                        bool *runningAppsArr = new bool[numMaxApps]{false};
                        mapAppsRunningInVectorModulePerVm[vmRequest.strVmId] = runningAppsArr;
                        userVM_Rq->setBPriorized(true);
                        bHandled = true;
                        return pHypervisor;
                    }

                }
            }

        }
    }

    return nullptr;
}

void DataCenterManagerCost::handleExecVmRentTimeout(cMessage *msg) {
    SM_UserAPP *pUserApp;
    Hypervisor *pHypervisor;

    std::string strUsername,
                strVmId,
                strAppName,
                strIp;

    bool bAlreadyFinished;

    SM_UserVM_Finish *pUserVmFinish;

    std::map<std::string, SM_UserAPP*>::iterator it;

    pUserVmFinish = dynamic_cast<SM_UserVM_Finish*>(msg);
    if (pUserVmFinish == nullptr)
        error ("%s - Unable to cast msg to SM_UserVM_Finish*. Wrong msg name [%s]?", LogUtils::prettyFunc(__FILE__, __func__).c_str(), msg->getName());

    EV_INFO << LogUtils::prettyFunc(__FILE__, __func__) << " - INIT" << endl;
    strVmId = pUserVmFinish->getStrVmId();

    strUsername = pUserVmFinish->getUserID();
    EV_INFO << "The rent of the VM [" << strVmId
                           << "] launched by the user " << strUsername
                           << " has finished" << endl;

    //deallocateVmResources(strVmId);

    pUserApp = getUserAppRequestPerUser(strUsername);

    if (pUserApp == nullptr)
        throw omnetpp::cRuntimeError(("[" + LogUtils::prettyFunc(__FILE__, __func__) + "] There is no app request message from the User!!").c_str());

//    acceptAppRequest(pUserApp, strVmId);

        //Check the Application status


        EV_INFO << "Last id gate: " << pUserApp->getLastGateId() << endl;
        EV_INFO
        << "Checking the status of the applications which are running over this VM"
        << endl;

        //Abort the running applications
        if (!pUserApp->allAppsFinished(strVmId))
          {
            EV_INFO << "Aborting running applications" << endl;
            abortAllApps(strVmId);
            pUserApp->abortAllApps(strVmId);
          } else {
              deallocateVmResources(strVmId);
          }
        // Check the result and send it
        checkAllAppsFinished(pUserApp, strVmId);


        EV_INFO << "Freeing resources..." << endl;

}

void DataCenterManagerCost::handleExtendVmAndResumeExecution(SIMCAN_Message *sm)
{
        string strVmId, strUsername;
        SM_UserAPP *userAPP_Rq;
        SM_UserVM* pUserVmRequest;
        std::map<std::string, SM_UserVM*>::iterator it;
        userAPP_Rq = dynamic_cast<SM_UserAPP*>(sm);
        bool bFound;

        if (userAPP_Rq != nullptr) {
            strVmId = userAPP_Rq->getVmId();
            if (!strVmId.empty()) {
                strUsername = userAPP_Rq->getUserID();
                it = acceptedUsersRqMap.find(strUsername);

                if(it != acceptedUsersRqMap.end())
                  {
                    pUserVmRequest = it->second;
                    bFound = false;
                    for(int j = 0; j < pUserVmRequest->getVmsArraySize() && !bFound; j++)
                      {
                        //Getting VM and scheduling renting timeout
                        VM_Request& vmRequest = pUserVmRequest->getVms(j);
                        //scheduleRentingTimeout(EXEC_VM_RENT_TIMEOUT, strUsername, vmRequest.strVmId, vmRequest.nRentTime_t2);

                        if (strVmId.compare(vmRequest.strVmId) == 0) {
                            bFound = true;
                            vmRequest.pMsg = scheduleRentingTimeout(EXEC_VM_RENT_TIMEOUT, strUsername, strVmId, 3600);
                            handleUserAppRequest(sm);
                        }

                      }

                  }
                else
                  {
                    EV_INFO << "WARNING! [" << LogUtils::prettyFunc(__FILE__, __func__) << "] The user: " << strUsername << "has not previously registered!!"<< endl;
                  }
            }
        }
        else
      {
        error ("%s - Unable to cast msg to SM_UserAPP*. Wrong msg name [%s]?", LogUtils::prettyFunc(__FILE__, __func__).c_str(), sm->getName());
      }
}

void DataCenterManagerCost::handleEndVmAndAbortExecution(SIMCAN_Message *sm)
{
    string strVmId;
    SM_UserAPP *userAPP_Rq;
    userAPP_Rq = dynamic_cast<SM_UserAPP*>(sm);
    if (userAPP_Rq != nullptr) {
        strVmId = userAPP_Rq->getVmId();
        if (!strVmId.empty()) {
            EV_INFO << "Freeing resources..." << endl;

            //Free the VM resources
            deallocateVmResources(strVmId);

            delete sm; //Delete ephemeral message
        }
    }
    else
    {
      error ("%s - Unable to cast msg to SM_UserAPP*. Wrong msg name [%s]?", LogUtils::prettyFunc(__FILE__, __func__).c_str(), sm->getName());
    }
}
