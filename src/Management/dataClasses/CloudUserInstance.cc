#include "CloudUserInstance.h"


CloudUserInstance::CloudUserInstance(CloudUser *ptrUser, unsigned int userNumber, int currentInstanceIndex, int totalUserInstances)
                  : UserInstance((CloudUser*) ptrUser, userNumber, currentInstanceIndex, totalUserInstances){

    UserVmReference* vmReference;
    int currentVm;

        // Include VM instances
        for (currentVm = 0; currentVm < ptrUser->getNumVirtualMachines(); currentVm++){

            // Get current application
            vmReference = ptrUser->getVirtualMachine(currentVm);

            // Insert a new collection of application instances
            insertNewVirtualMachineInstances (vmReference->getVmBase(), vmReference->getNumInstances(), vmReference->getRentTime());
        }

    processApplicationCollection();

    requestVmMsg = nullptr;
    requestAppMsg = nullptr;
    subscribeVmMsg = nullptr;

    bTimeout_t2 = bTimeout_t4 = bFinished = false;
    dInitTime = dEndTime = 0.0;
}



CloudUserInstance::~CloudUserInstance() {
    virtualMachines.clear();
}


void CloudUserInstance::insertNewVirtualMachineInstances (VirtualMachine* vmPtr, int numInstances, int nRentTime){

    VmInstanceCollection* newVmCollection;

        newVmCollection = new VmInstanceCollection(vmPtr, this->userID, numInstances,nRentTime);
        virtualMachines.push_back(newVmCollection);
}


std::string CloudUserInstance::toString (bool includeAppsParameters, bool includeVmFeatures){

    std::ostringstream info;
    int i;

        info << userID << std::endl;

            // Parses applications
            for (i=0; i<applications.size(); i++){
                info << "\t\tAppCollection[" << i << "] -> " << applications.at(i)->toString(includeAppsParameters);
            }

            // Parses VMs
            for (i=0; i<virtualMachines.size(); i++){
                info << "\t\tVM set[" << i << "] -> " << virtualMachines.at(i)->toString(includeVmFeatures);
            }

    return info.str();
}
AppInstance* CloudUserInstance::getAppInstance(int nIndex)
{
    AppInstance* pAppRet;

    pAppRet = nullptr;

    if(nIndex<appInstances.size())
    {
        pAppRet = appInstances.at(nIndex);
    }

    return pAppRet;
}
VmInstanceCollection* CloudUserInstance::getVmCollection(int nCollection)
{
    VmInstanceCollection* pVmCollection;

    pVmCollection = nullptr;

    if(nCollection<virtualMachines.size())
    {
        pVmCollection = virtualMachines.at(nCollection);
    }

    return pVmCollection;
}
void CloudUserInstance::setRentTimes(int maxStartTime_t1, int nRentTime_t2, int maxSubTime_t3, int maxSubscriptionTime_t4)
{
    this->maxStartTime_t1 = maxStartTime_t1;
    this->nRentTime_t2 =  nRentTime_t2;
    this->maxSubTime_t3 = maxSubTime_t3;
    this->maxSubscriptionTime_t4 = maxSubscriptionTime_t4;
}

void CloudUserInstance::processApplicationCollection()
{
    AppInstanceCollection* pCol;
    AppInstance* pApp;

    for(int i=0;i<applications.size();i++)
    {
        pCol = applications.at(i);
        for(int j=0;j<pCol->getNumInstances();j++)
        {
            pApp = pCol->getInstance(j);
            appInstances.push_back(pApp);
        }

    }
}
double CloudUserInstance::getEndTime() const
{
   return dEndTime;
}

void CloudUserInstance::setEndTime(double endTime)
{
   dEndTime = endTime;
}

double CloudUserInstance::getInitTime() const
{
   return dInitTime;
}

void CloudUserInstance::setInitTime(double initTime)
{
   dInitTime = initTime;
}

bool CloudUserInstance::isFinished() const
{
   return bFinished;
}

void CloudUserInstance::setFinished(bool finished)
{
   bFinished = finished;
}

double CloudUserInstance::getArrival2Cloud() const
{
   return dArrival2Cloud;
}

void CloudUserInstance::setArrival2Cloud(double arrival2Cloud)
{
   dArrival2Cloud = arrival2Cloud;
}

void CloudUserInstance::setTimeoutMaxStart()
{
   bTimeout_t2 = true;
}

void CloudUserInstance::setTimeoutMaxRentTime()
{
   bTimeout_t2 = true;
}
void CloudUserInstance::setTimeoutMaxSubscribed()
{
   bTimeout_t4 = true;
}
bool CloudUserInstance::isTimeout()
{
   return  (bTimeout_t2 || bTimeout_t4);
}
bool CloudUserInstance::isTimeoutMaxRent()
{
   return bTimeout_t2;
}

bool CloudUserInstance::isTimeoutSubscribed()
{
   return bTimeout_t4;
}
double CloudUserInstance::getInitExecTime() const
{
    return this->dInitExec;
}
void CloudUserInstance::setInitExecTime(double dExec)
{
    this->dInitExec = dExec;
}
void CloudUserInstance::setSubscribe(bool bSubscribe)
{
    this->bSubscribe = bSubscribe;
}
bool CloudUserInstance::hasSubscribed()
{
    return this->bSubscribe;
}

bool CloudUserInstance::operator <(const CloudUserInstance &other) const {
    return this->getArrival2Cloud() < other.getArrival2Cloud() ;
}
