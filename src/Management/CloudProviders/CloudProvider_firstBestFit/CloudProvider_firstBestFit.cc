#include "CloudProvider_firstBestFit.h"

Define_Module(CloudProvider_firstBestFit);


CloudProvider_firstBestFit::~CloudProvider_firstBestFit(){
    handlingAppsRqMap.clear();
    acceptedUsersRqMap.clear();
}


void CloudProvider_firstBestFit::initialize(){
    int nIndex;

    EV_INFO << "CloudProviderFirstFit::initialize - Init" << endl;

    initializeDataCenterCollection();

    // Init super-class
    CloudProviderBase_firstBestFit::initialize();
}

void CloudProvider_firstBestFit::initializeDataCenterCollection(){
    datacenterCollection = new DataCenterInfoCollectionReservation();
}

void CloudProvider_firstBestFit::loadNodes(){
    int nIndex;
    std::vector<int> reservedNodes;
    std::vector<int>::iterator it;

    for (nIndex=0; nIndex<dataCentersBase.size(); nIndex++){
        DataCenterReservation* dataCenterReservation = dynamic_cast<DataCenterReservation*>(dataCentersBase.at(nIndex));
        if (dataCenterReservation != nullptr)
        {
            it = reservedNodes.begin();
            reservedNodes.insert(it+nIndex, dataCenterReservation->getReservedNodes());
        }
    }

    DataCenterInfoCollectionReservation* datacenterCollectionReservation = dynamic_cast<DataCenterInfoCollectionReservation*>(datacenterCollection);

    if (datacenterCollectionReservation != nullptr)
    {
        datacenterCollectionReservation->setReservedNodes(reservedNodes);
    }

    CloudProviderBase_firstBestFit::loadNodes();
}


int CloudProvider_firstBestFit::parseDataCentersList (){
    int result;
    const char *dataCentersListChr;

    dataCentersListChr= par ("dataCentersList");
    DataCenterReservationListParser dataCenterParser(dataCentersListChr);
    result = dataCenterParser.parse();
    if (result == SC_OK) {
        dataCentersBase = dataCenterParser.getResult();
    }
    return result;
}

//Default
bool CloudProvider_firstBestFit::checkVmUserFit(SM_UserVM*& userVM_Rq)
{
    bool bRet, bAccepted;

    int nTotalRequestedCores;
    int nRequestedVms, nPrice, nAvailableCores, nTotalCores;
    std::string nodeIp, strUserName, strVmId;
    VM_Request vmRequest;
    CloudUser* pCloudUser;
    SM_UserVM_Cost* userVm_Rq_Cost;

    bRet = CloudProviderBase_firstBestFit::checkVmUserFit(userVM_Rq);

    if(!bRet){
        strUserName = userVM_Rq->getUserID();

        pCloudUser = findUserTypeById(strUserName);

        DataCenterInfoCollectionReservation* datacenterCollectionReservation = dynamic_cast<DataCenterInfoCollectionReservation*>(datacenterCollection);

        bAccepted = bRet = true;
        if(userVM_Rq != nullptr && datacenterCollectionReservation != nullptr)
        {
            if (pCloudUser->getPriorityType() == Priority)
            {
                nPrice = 10;
                nRequestedVms = userVM_Rq->getTotalVmsRequests();

                EV_DEBUG << "checkVmUserFit- Init" << endl;
                EV_DEBUG << "checkVmUserFit- checking for free space in reserved nodes, " << nRequestedVms << " Vm(s) for the user" << userVM_Rq->getUserID() << endl;

                userVm_Rq_Cost = dynamic_cast<SM_UserVM_Cost*>(userVM_Rq);
                if (userVm_Rq_Cost!=nullptr)
                    userVm_Rq_Cost->setBPriorized(true);

                //Before starting the process, it is neccesary to check if the
                nTotalRequestedCores = calculateTotalCoresRequested(userVM_Rq);
                nAvailableCores = datacenterCollectionReservation->getTotalReservedAvailableCores();
                nTotalCores = datacenterCollectionReservation->getTotalReservedCores();

                if(nTotalRequestedCores<=nAvailableCores)
                {
                    EV_DEBUG << "checkVmUserFit - There is available space in reserved nodes: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;

                    //Process all the VMs
                    for(int i=0;i<nRequestedVms && bRet;i++)
                    {
                        EV_DEBUG << endl <<"checkVmUserFit - Trying to handle the VM: " << i << " in reserved node" << endl;

                        //Get the VM request
                        vmRequest = userVM_Rq->getVms(i);

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
                        bAccepted = datacenterCollectionReservation->handlePrioritaryVmRequest(pNode);


                        //We need to know the price of the Node.
                        userVM_Rq->createResponse(i, bAccepted, pNode->getStartTime(), pNode->getIp(), pNode->getPrice());
                        bRet &= bAccepted;

                        if(!bRet)
                        {
                            for(int j=0;j<i;j++)
                            {
                                vmRequest = userVM_Rq->getVms(j);
                                strVmId = vmRequest.strVmId;
                                datacenterCollectionReservation->freeVmRequest(strVmId);
                            }
                            EV_DEBUG << "checkVmUserFit - The VM: " << i << "has not been handled, not enough space in reserved nodes, all the request of the user "<< strUserName <<"have been deleted"<< endl;
                        }
                        else
                        {
                            //Update value
                            nAvailableCores = datacenterCollectionReservation->getTotalReservedAvailableCores();
                            EV_DEBUG << "checkVmUserFit - The VM: " << i << " has been handled and stored sucessfully in reserved node, available cores: "<< nAvailableCores << endl;
                        }
                    }
                }
                else
                {
                    EV_DEBUG << "checkVmUserFit - There isnt enough space in reserved nodes: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs "<< nAvailableCores <<endl;
                    bRet = false;
                }

                //Update the data
                nTotalRequestedCores = calculateTotalCoresRequested(userVM_Rq);
                nAvailableCores = datacenterCollectionReservation->getTotalReservedAvailableCores();
                nTotalCores = datacenterCollectionReservation->getTotalReservedCores();

                if(bRet)
                {
                    EV_DEBUG << "checkVmUserFit - Reserved space in reserved nodes for: " << userVM_Rq->getUserID() << endl;
                }
                else
                    EV_DEBUG << "checkVmUserFit - Unable to reserve space in reserved nodes for: " << userVM_Rq->getUserID() << endl;

                EV_DEBUG << "checkVmUserFit - Updated space in reserved nodes#: [" << userVM_Rq->getUserID() << "Requested: "<<nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;

            }
            else
            {
                EV_DEBUG << "checkVmUserFit - User is not prioritary: " << userVM_Rq->getUserID() << endl;
                bRet = false;
            }
        }
        else
        {
            EV_ERROR << "checkVmUserFit - WARNING!! nullpointer detected" <<endl;
            bRet = false;
        }

        EV_DEBUG << "checkVmUserFit- End" << endl;
    }

    return bRet;
}

//Modified
//bool CloudProvider_firstBestFit::checkVmUserFit(SM_UserVM*& userVM_Rq)
//{
//    bool bRet, bAccepted;
//
//    int nTotalRequestedCores;
//    int nRequestedVms, nPrice, nAvailableCores, nTotalCores;
//    std::string nodeIp, strUserName, strVmId;
//    VM_Request vmRequest;
//    CloudUser* pCloudUser;
//
//    bRet = CloudProviderBase_firstBestFit::checkVmUserFit(userVM_Rq);
//
//    if(!bRet){
//        strUserName = userVM_Rq->getUserID();
//
//        pCloudUser = findUserTypeById(strUserName);
//
//        DataCenterInfoCollectionReservation* datacenterCollectionReservation = dynamic_cast<DataCenterInfoCollectionReservation*>(datacenterCollection);
//
//        //bAccepted = bRet = true;
//        /******/
//         bRet=bAccepted = false;
//        /******/
//
//        if(userVM_Rq != nullptr && datacenterCollectionReservation != nullptr && pCloudUser->getPriorityType() == Priority)
//        {
//            nPrice = 10;
//            nRequestedVms = userVM_Rq->getTotalVmsRequests();
//
//            EV_DEBUG << "checkVmUserFit- Init" << endl;
//            EV_DEBUG << "checkVmUserFit- checking for free space, " << nRequestedVms << " Vm(s) for the user" << userVM_Rq->getUserID() << endl;
//
//            //Before starting the process, it is neccesary to check if the
//            nTotalRequestedCores = calculateTotalCoresRequested(userVM_Rq);
//
//            /*********************************************/
//            int nAttendedCores, nRequestedVms;
//            VM_Request vmRequest;
//
////            nAttendedCores=nRequestedVms=0;
////            if(userVM_Rq != NULL)
////            {
////                nRequestedVms = userVM_Rq->getTotalVmsRequests();
////
////                for(int i=0;i<nRequestedVms;i++)
////                {
////                    vmRequest = userVM_Rq->getVms(i);
////
////                    if (datacenterCollectionReservation->isVmAttended(vmRequest.strVmType)) {
////                        nAttendedCores+=getTotalCoresByVmType(vmRequest.strVmType);
////                    }
////
////                }
////            }
//
//
//            /*********************************************/
//
//            nAvailableCores = datacenterCollectionReservation->getTotalReservedAvailableCores();
//            nTotalCores = datacenterCollectionReservation->getTotalReservedCores();
//
//           // if(nTotalRequestedCores<=nAvailableCores)
//           // {
//                EV_DEBUG << "checkVmUserFit - There is available space: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;
//
//                //Process all the VMs
//                for(int i=0;i<nRequestedVms && bRet;i++)
//                {
//                    EV_DEBUG << endl <<"checkVmUserFit - Trying to handle the VM: " << i << endl;
//
//                    //Get the VM request
//                    vmRequest = userVM_Rq->getVms(i);
//
//                    /*****/
//                    if (vmRequest.responseList.empty()){
//
//                        //Create and fill the noderesource  with the VMrequest
//                        NodeResourceRequest* pNode = new NodeResourceRequest();
//                        pNode->setUserName(strUserName);
//                        pNode->setMaxStartTimeT1(vmRequest.maxStartTime_t1);
//                        pNode->setRentTimeT2(vmRequest.nRentTime_t2);
//                        pNode->setMaxSubTimeT3(vmRequest.maxSubTime_t3);
//                        pNode->setMaxSubscriptionTimeT4(vmRequest.maxSubscriptionTime_t4);
//                        pNode->setVmId(vmRequest.strVmId);
//                        fillVmFeatures(vmRequest.strVmType, pNode);
//
//                        //Send the request to the DC
//                        bAccepted = datacenterCollectionReservation->handlePrioritaryVmRequest(pNode);
//
//
//                        //We need to know the price of the Node.
//                        userVM_Rq->createResponse(i, bAccepted, pNode->getStartTime(), pNode->getIp(), pNode->getPrice());
//
//                        bRet |= bAccepted;
//
//                    /****/
//                    }
//                    /****/
//                    //bRet &= bAccepted;
//
////                    if(!bRet)
////                    {
////                        for(int j=0;j<i;j++)
////                        {
////                            vmRequest = userVM_Rq->getVms(j);
////                            strVmId = vmRequest.strVmId;
////                            datacenterCollectionReservation->freeVmRequest(strVmId);
////                        }
////                        EV_DEBUG << "checkVmUserFit - The VM: " << i << "has not been handled, not enough space, all the request of the user "<< strUserName <<"have been deleted"<< endl;
////                    }
////                    else
////                    {
////                        //Update value
////                        nAvailableCores = datacenterCollectionReservation->getTotalReservedAvailableCores();
////                        EV_DEBUG << "checkVmUserFit - The VM: " << i << " has been handled and stored sucessfully, available cores: "<< nAvailableCores << endl;
////                    }
//                }
//           // }
//            //else
//            //{
//            //    EV_DEBUG << "checkVmUserFit - There isnt enough space: [" << userVM_Rq->getUserID() << nTotalRequestedCores<< " vs "<< nAvailableCores <<endl;
//            //    bRet = false;
//            //}
//
//            //Update the data
//            nTotalRequestedCores = calculateTotalCoresRequested(userVM_Rq);
//            nAvailableCores = datacenterCollectionReservation->getTotalReservedAvailableCores();
//            nTotalCores = datacenterCollectionReservation->getTotalReservedCores();
//
//            if(bRet)
//            {
//                EV_DEBUG << "checkVmUserFit - Reserved space for: " << userVM_Rq->getUserID() << endl;
//            }
//            else
//                EV_DEBUG << "checkVmUserFit - Unable to reserve space for: " << userVM_Rq->getUserID() << endl;
//
//            EV_DEBUG << "checkVmUserFit - Updated space#: [" << userVM_Rq->getUserID() << "Requested: "<<nTotalRequestedCores<< " vs Available ["<< nAvailableCores << "/" <<nTotalCores << "]"<<endl;
//        }
//        else
//        {
//            EV_ERROR << "checkVmUserFit - WARNING!! nullpointer detected" <<endl;
//            bRet = false;
//        }
//
//        EV_DEBUG << "checkVmUserFit- End" << endl;
//    }
//
//    return bRet;
//}

CloudUser* CloudProvider_firstBestFit::findUserTypeById (std::string userId){

    std::vector<CloudUser*>::iterator it;
    CloudUser* result;
    bool found;

        // Init
        found = false;
        result = nullptr;
        it = userTypes.begin();

        // Search...
        while((!found) && (it != userTypes.end())){
            if (userId.find((*it)->getType()) != std::string::npos) {
                found = true;
                result = (*it);
            }
            else
                it++;
        }

    return result;
}

//Modified to send a app for VM
void CloudProvider_firstBestFit::abortAllApps(SM_UserAPP* userApp, std::string strVmId)
{
    if(userApp != nullptr)
    {
        userApp->abortAllApps(strVmId);

        cancelAndDeleteAppFinishMsgs(userApp, strVmId);

    }
}

void CloudProvider_firstBestFit::cancelAndDeleteAppFinishMsgs(SM_UserAPP* userApp, std::string strVmId)
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

void CloudProvider_firstBestFit::handleAppExecEndSingle(SM_UserAPP_Finish* pUserAppFinish) {

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

                userApp->changeState(strAppName, strVmId, appFinishedOK);
                userApp->setEndTime(strAppName, strVmId, simTime().dbl());

                userApp->printUserAPP();
            }

            APP_Request appRq;
            int nIndex, nRet;
            bool bFound;

            bFound =  false;
            nIndex = 0;

            while(!bFound && nIndex<userApp->getAppArraySize())
            {
                appRq=userApp->getApp(nIndex);
                if(appRq.strApp.compare(strAppName)== 0 && strVmId.compare(appRq.vmId) == 0)
                {
                    userApp->getApp(nIndex).pMsgTimeout = nullptr;
                    bFound=true;
                }
                nIndex++;
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
                        //TODO: comprobar que realmente es por timeout.
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

void CloudProvider_firstBestFit::handleExecVmRentTimeout(SM_UserVM_Finish* pUserVmFinish) {
    SM_UserAPP *userApp;
    std::string strUsername, strVmId, strAppName, strIp;
    EV_INFO << "processSelfMessage - INIT" << endl;
    strUsername = pUserVmFinish->getUserID();
    strVmId = pUserVmFinish->getStrVmId();
    EV_INFO << "The rent of the VM [" << pUserVmFinish->getStrVmId()
                           << "] launched by the user " << strUsername
                           << " has finished" << endl;

    if (strUsername.compare("(3)User_D[244/1000]")==0)
                        EV_INFO << "eee" << endl;

    //Check the Application status
    if (handlingAppsRqMap.find(strUsername) != handlingAppsRqMap.end()) {
        userApp = handlingAppsRqMap.at(strUsername);

        //Check the application status
        if (userApp != nullptr) {
            if (userApp->getLastGateId()>=0) {

                EV_INFO << "Last id gate: " << userApp->getLastGateId() << endl;
                EV_INFO
                << "Checking the status of the applications which are running over this VM"
                << endl;

                //Abort the running applications
                if (!userApp->allAppsFinished(strVmId)) {
                    EV_INFO << "Aborting running applications" << endl;
                    abortAllApps(userApp, strVmId);
                    //Notify the user the end of the execution
                    EV_INFO << "processSelfMessage - EXEC_VM_RENT_TIMEOUT Init"
                            << endl;

                    //if so, notify this.
                    userApp->setVmId(strVmId.c_str());
                    timeoutAppRunningRequest(userApp);
                } else {
                    EV_INFO << "All the applications have already finished" << endl;
                    //Free the VM resources
                    freeVm(strVmId);

                    //Check the subscription queue
                    updateSubsQueue();
                }
            }
            else
            {
                SM_UserVM_Finish* pUserVmFinishCopy = pUserVmFinish->dup();
                pUserVmFinishCopy->setName(EXEC_VM_RENT_TIMEOUT);
                pendingVmRentTimeoutMessages.insert(pendingVmRentTimeoutMessages.begin(), pUserVmFinishCopy);
            }
        }
    }
    else
    {
        EV_INFO << "All the applications have already finished" << endl;
        //Free the VM resources
        freeVm(strVmId);

        //Check the subscription queue
        updateSubsQueue();
    }
}

void CloudProvider_firstBestFit::timeoutAppRunningRequest(SM_UserAPP* userAPP_Rq)
{
    EV_INFO << "Sending timeout with apps running to the user:" << userAPP_Rq->getUserID() << endl;
    EV_INFO << "Last id gate: " << userAPP_Rq->getLastGateId() << endl;

    //Difference with timeoutAppRequest that end the execution.
    userAPP_Rq->setFinished(false);

    //Fill the message
    userAPP_Rq->setIsResponse(true);
    userAPP_Rq->setOperation(SM_APP_Rsp);
    userAPP_Rq->setResult(SM_APP_Res_Timeout);

    //Send the values
    sendResponseMessage(userAPP_Rq);
}

void CloudProvider_firstBestFit::handleUserAppRequest(SM_UserAPP* userAPP_Rq)
{
    if(userAPP_Rq != nullptr)
    {
        EV_INFO << "CloudProvider_firstBestFit::handleUserAppRequest - Handle AppRequest"  << endl;

        //Handle a VmRequest
        if(userAPP_Rq->getOperation() == SM_APP_Req_Resume)
        {
            extendVmAndResumeApp(userAPP_Rq);
        }
        else if (userAPP_Rq->getOperation() == SM_APP_Req_End)
        {
            endVmAndAbortApp(userAPP_Rq);
        }
        else if (userAPP_Rq->getOperation() == SM_APP_Req)
        {
            //Get the user name, and recover the info about the VmRequests;
            bool bHandle;
            int nIndexVmFound;
            std::string strUsername, strVmId, strIp, strAppName;
            Application* appType;
            SM_UserAPP_Finish* pMsgFinish;
            APP_Request userApp;
            VirtualMachine* vmType;

            int nTotalTime;

            //Las aplicaciones estan relacionadas con las VM
            //Hay que relacionar la APP con la VM para asi poder terminar con ella.
            bHandle = false;

            if(userAPP_Rq != nullptr)
            {
                //APPRequest
                userAPP_Rq->printUserAPP();

                strUsername = userAPP_Rq->getUserID();

                if(acceptedUsersRqMap.find(strUsername) != acceptedUsersRqMap.end())
                {
                    SM_UserVM& userVmRequest = acceptedUsersRqMap.at(strUsername);
                    if(handlingAppsRqMap.find(strUsername) == handlingAppsRqMap.end())
                    {
                        //Registering the appRq
                        handlingAppsRqMap[strUsername] = userAPP_Rq;
                    }
                    EV_INFO << "Executing the VMs corresponding with the user: " << strUsername << " | Total: "<< userVmRequest.getVmsArraySize()<< endl;

                    //First step consists in calculating the total units of time spent in executing the application.
                    if(userAPP_Rq->getArrayAppsSize()>0)
                    {
                        for(int j=0;j<userVmRequest.getVmsArraySize();j++)
                        {
                            //Getting VM and scheduling renting timeout
                            VM_Request& vmRequest = userVmRequest.getVms(j);
                            scheduleRentingTimeout(vmRequest, strUsername);

                            strVmId = vmRequest.strVmId;
                            vmType = searchVmPerType(userVmRequest.getVmRequestType(j));

//                            int totalTimePerCore[vmType->getNumCores()];
//                            for (int i=0; i<vmType->getNumCores();i++)
//                                totalTimePerCore[i]=0;
                            int totalTimePerCore[1] = { 0 };

                            for(int i=0;i<userAPP_Rq->getAppArraySize();i++)
                            {
                                userApp =userAPP_Rq->getApp(i);

                                if(strVmId.compare(userApp.vmId)==0)
                                {
                                    appType = searchAppTypeById(userApp.strAppType);

                                    if(appType != nullptr)
                                    {
                                        //Assing the app to core with less utilization time
                                        //std::sort(totalTimePerCore, totalTimePerCore+vmType->getNumCores());
                                        nTotalTime = TEMPORAL_calculateTotalTime(appType);
                                        totalTimePerCore[0] = totalTimePerCore[0] + nTotalTime;

                                        strAppName = userApp.strApp;

                                        //Create new Message
                                        pMsgFinish = new SM_UserAPP_Finish();
                                        pMsgFinish->setUserID(strUsername.c_str());
                                        pMsgFinish->setStrApp(strAppName.c_str());
                                        pMsgFinish->setStrVmId(strVmId.c_str());
                                        pMsgFinish->setNTotalTime(totalTimePerCore[0]);
                                        pMsgFinish->setName(EXEC_APP_END_SINGLE);

                                        userAPP_Rq->getApp(i).pMsgTimeout = pMsgFinish;
                                        //userApp.vmId =  vmRequest.strVmId;

                                        //Change status to running
                                        userAPP_Rq->changeState(strAppName, strVmId, appRunning);
                                        userAPP_Rq->changeStateByIndex(i, strAppName, appRunning);
                                        userAPP_Rq->setVmIdByIndex(i, userApp.strIp, strVmId);

                                        EV_INFO << "Scheduling time rental Msg, " << userAPP_Rq->getApp(i).pMsgTimeout->getName() << endl;
                                        scheduleAt(simTime().dbl()+totalTimePerCore[0], userAPP_Rq->getApp(i).pMsgTimeout);
                                        bHandle = true;

                                    }
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
            }
            else
            {
                throw omnetpp::cRuntimeError("[CloudProviderFirstFit] Wrong userAPP_Rq. Nullpointer!!");
            }

            if(!bHandle)
            {
                userAPP_Rq->setResult(0);
                rejectAppRequest(userAPP_Rq);
            }
        }
    }
    else
    {
        throw omnetpp::cRuntimeError("[CloudProviderFirstFit] Wrong userAPP_Rq. Nullpointer!!");
    }

    checkPendingVmRentTimeoutMessages();

}

void CloudProvider_firstBestFit::checkPendingVmRentTimeoutMessages()
{
    SM_UserVM_Finish* pUserVmFinish;
    while (!pendingVmRentTimeoutMessages.empty())
    {
        pUserVmFinish = pendingVmRentTimeoutMessages.back();
        pendingVmRentTimeoutMessages.pop_back();
        //processSelfMessage(pUserVmFinish);
        scheduleAt(simTime().dbl()+1, pUserVmFinish);
    }

}

void CloudProvider_firstBestFit::extendVmAndResumeApp(SM_UserAPP* userAPP_Rq)
{

    //Get the user name, and recover the info about the VmRequests;
    bool bHandle, bVmFound;
    std::string strUsername, strAppType, strVmId, strIp, strAppName;
    Application* appType;
    SM_UserVM userVmRequest;
    SM_UserVM_Finish* pMsgVmFinish;
    SM_UserAPP_Finish* pMsgFinish;
    VM_Request vmRequest;
    APP_Request userApp;
    VirtualMachine* vmType;

    int nTotalTime, nStartTime, nFinishTime, nVmIndex;

    //Las aplicaciones estan relacionadas con las VM
    //Hay que relacionar la APP con la VM para asi poder terminar con ella.
    //Suponemos 1 VM por app, en el momento en que nos quedemos sin VM, le damos el ID de la ultima.
    bHandle = false;
    if(userAPP_Rq != nullptr)
    {
        //APPRequest
        userAPP_Rq->printUserAPP();

        strUsername = userAPP_Rq->getUserID();

        if (strUsername.compare("(3)User_D[244/1000]")==0)
                            EV_INFO << "eee" << endl;

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
                //Get the VM
                strVmId = userAPP_Rq->getVmId();

                bVmFound = false;
                for(nVmIndex=0;nVmIndex<userVmRequest.getVmsArraySize() && !bVmFound;nVmIndex++)
                {
                    //Getting VM and scheduling renting timeout
                    vmRequest = userVmRequest.getVms(nVmIndex);

                    if(strVmId.compare(vmRequest.strVmId)==0)
                    {
                        bVmFound = true;

                        pMsgVmFinish = new SM_UserVM_Finish();
                        pMsgVmFinish->setUserID(strUsername.c_str());
                        if(!strVmId.empty())
                            pMsgVmFinish->setStrVmId(strVmId.c_str());
                        pMsgVmFinish->setName(EXEC_VM_RENT_TIMEOUT);
                        userVmRequest.getVms(nVmIndex).pMsg = pMsgVmFinish;

                        EV_INFO << "Scheduling Msg name " << pMsgVmFinish << " at "<< simTime().dbl()+3600 <<endl;
                        scheduleAt(simTime().dbl()+3600, pMsgVmFinish);


                        vmType = searchVmPerType(userVmRequest.getVmRequestType(nVmIndex));
//                        int totalTimePerCore[vmType->getNumCores()];
//                        for (int i=0; i<vmType->getNumCores();i++)
//                            totalTimePerCore[i]=0;
                        int totalTimePerCore[1] = { 0 };

                        for(int i=0;i<userAPP_Rq->getAppArraySize();i++)
                        {
                            //Get the app
                            userApp =userAPP_Rq->getApp(i);

                            if (strVmId.compare(userApp.vmId.c_str())==0)
                            {
                                strAppType = userApp.strAppType;
                                appType = searchAppTypeById(strAppType);

                                if(appType != nullptr)
                                {
                                    //Get the parameters
                                    //Assing the app to core with less utilization time
                                    //std::sort(totalTimePerCore, totalTimePerCore+vmType->getNumCores());
                                    nTotalTime = TEMPORAL_calculateTotalTime(appType);
                                    totalTimePerCore[0] = totalTimePerCore[0] + nTotalTime;

                                    strAppName = userApp.strApp;
                                    //If has not been executed yet
                                    if (!userAPP_Rq->isFinishedOK(strAppName, strVmId))
                                    {
                                        nStartTime = userApp.startTime;
                                        nFinishTime = userApp.finishTime;

                                        //Create new Message
                                        pMsgFinish = new SM_UserAPP_Finish();
                                        pMsgFinish->setUserID(strUsername.c_str());
                                        pMsgFinish->setStrApp(strAppName.c_str());
                                        pMsgFinish->setStrVmId(strVmId.c_str());
                                        pMsgFinish->setNTotalTime(totalTimePerCore[0]);
                                        pMsgFinish->setName(EXEC_APP_END_SINGLE);

                                        userAPP_Rq->getApp(i).pMsgTimeout = pMsgFinish;
                                        //userApp.vmId =  vmRequest.strVmId;

                                        //Change status to running
                                        userAPP_Rq->changeStateByIndex(i, strAppName, appRunning);
                                        userAPP_Rq->decreaseFinishedApps();

                                        EV_INFO << "Scheduling time rental Msg, " << userAPP_Rq->getApp(i).pMsgTimeout->getName() << endl;
                                        scheduleAt(simTime().dbl()+totalTimePerCore[0]-(nFinishTime-nStartTime), userAPP_Rq->getApp(i).pMsgTimeout);

                                    }
                                }
                            }
                        }
                        bHandle = true;
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
    }
    else
    {
        throw omnetpp::cRuntimeError("[CloudProviderFirstFit] Wrong userAPP_Rq. Nullpointer!!");
    }

    if(!bHandle)
    {
        userAPP_Rq->setResult(0);
        rejectAppRequest(userAPP_Rq);
    }
}

void CloudProvider_firstBestFit::endVmAndAbortApp(SM_UserAPP* userAPP_Rq)
{
    std::string strVmId;

    if(userAPP_Rq != nullptr)
    {
        strVmId = userAPP_Rq->getVmId();

        //Free the VM resources
        freeVm(strVmId);

        //Check the subscription queue
        updateSubsQueue();

        userAPP_Rq->setVmId(nullptr);

        if (userAPP_Rq->allAppsFinished())
            timeoutAppRequest(userAPP_Rq);

    }
    else
    {
        throw omnetpp::cRuntimeError("[CloudProviderFirstFit] Wrong userAPP_Rq. Nullpointer!!");
    }
}

void CloudProvider_firstBestFit::scheduleRentingTimeout(VM_Request& vmRequest, std::string strUsername)
{
    if (strUsername.compare("(3)User_D[244/1000]")==0)
                        EV_INFO << "eee" << endl;

    vmRequest.pMsg = new SM_UserVM_Finish();
    vmRequest.pMsg->setUserID(strUsername.c_str());
    if(!vmRequest.strVmId.empty())
        vmRequest.pMsg ->setStrVmId(vmRequest.strVmId.c_str());

    vmRequest.pMsg->setName(EXEC_VM_RENT_TIMEOUT);

    EV_INFO << "Scheduling Msg name " << vmRequest.pMsg << " at "<< simTime().dbl()+vmRequest.nRentTime_t2 <<endl;
    scheduleAt(simTime().dbl()+vmRequest.nRentTime_t2, vmRequest.pMsg);
}
