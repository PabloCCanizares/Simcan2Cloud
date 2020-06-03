#include "CloudUser.h"

CloudUser::CloudUser(std::string type, int numInstances){
    this->type = type;
    this->numInstances = numInstances;
}

CloudUser::~CloudUser() {
    applications.clear();
    virtualMachines.clear();
}

void CloudUser::insertVirtualMachine(VirtualMachine* vmPtr, int numInstances, int nRentTime){

    UserVmReference* newElement;

        newElement = new UserVmReference(vmPtr, numInstances, nRentTime);
        virtualMachines.push_back(newElement);
}

UserVmReference* CloudUser::getVirtualMachine (int index){

    UserVmReference* element;

        element = nullptr;

        if ((index<0) || (index>=virtualMachines.size())){
            throw omnetpp::cRuntimeError("Index out of bounds while accessing application (Element) %d in User:%s", index, type.c_str());
        }
        else
            element = virtualMachines.at(index);

    return element;
}

int CloudUser::getNumVirtualMachines(){
    return virtualMachines.size();
}

std::string CloudUser::toString (){

    std::ostringstream info;
    int i;

//        info << "Type:" << type << " - Priority: " << tPriorityTypeLabel[priorityType] << " - Sla: " << sla->getType() << " -  #Instances:" << numInstances << std::endl;
        info << "Type:" << type << " -  #Instances:" << numInstances << std::endl;

           // Parses applications
           for (i=0; i<applications.size(); i++){
               info << "\t  + App[" << i << "] -> " << applications.at(i)->toString() << std::endl;
           }

           // Parses VMs
           for (i=0; i<virtualMachines.size(); i++){
               info << "\t  + VM[" << i << "] -> " << virtualMachines.at(i)->toString() << std::endl;
           }

    return info.str();
}
void CloudUser::insertApplication(Application* appPtr, int numInstances){

    UserAppReference* newElement;

        newElement = new UserAppReference(appPtr, numInstances);
        applications.push_back(newElement);
}

UserAppReference* CloudUser::getApplication (int index){

    UserAppReference* element;

        element = nullptr;

        if ((index<0) || (index>=applications.size())){
            throw omnetpp::cRuntimeError("Index out of bounds while accessing application (Element) %d in User:%s", index, type.c_str());
        }
        else
            element = applications.at(index);

    return element;
}

int CloudUser::getNumApplications(){
    return applications.size();
}

const std::string& CloudUser::getType() const {
    return type;
}

int CloudUser::getNumInstances() const {
    return numInstances;
}

