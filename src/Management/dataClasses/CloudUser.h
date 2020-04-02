#ifndef CLOUDUSER_H_
#define CLOUDUSER_H_

#include "UserVmReference.h"
#include "UserAppReference.h"
#include "Management/dataClasses/include/SIMCAN2_types.h"
#include "Management/dataClasses/Sla.h"

/**
 *
 * Class that represents a user in cloud environments.
 *
 */
class CloudUser{

    private:

        /** Vector of virtual machines required by this user */
        std::vector<UserVmReference*> virtualMachines;

        /** Required applications for this user */
        std::vector<UserAppReference*> applications;

        /** User type. Name of the user in the SIMCAN repository. */
        std::string type;

        /** Number of instances of this user to be generated in the simulated environment. */
        int numInstances;

        /** Priority type of the user requests in the cloud  */
        tPriorityType priorityType;

        /** Sla signed by the user */
        Sla* sla;

    public:

        /**
         * Constructor.
         *
         * @param type User type.
         * @param numInstances Number of instances of this user to be created in the simulation environment.
         */
        CloudUser(std::string type, int numInstances, tPriorityType priorityType, Sla* slaPtr);

        /**
         * Destructor.
         */
        ~CloudUser();

        /**
         * Assigns a new virtual machine to this user.
         *
         * @param vmPtr Pointer to the new virtual machine assigned to this user.
         * @param numInstances Number of instances of the new virtual machine.
         * @param nRentTime Rental time of instances of the new virtual machine.
         */
        void insertVirtualMachine(VirtualMachine* vmPtr, int numInstances, int nRentTime);

        /**
         * Gets the virtual machine at index position in the <b>virtualMachines</b> vector.
         *
         * @param index Position of the virtual machine.
         *
         * @return If the requested virtual machine is located in the vector, then a pointer to its object is returned. In other case, \a nullptr is returned.
         */
        UserVmReference* getVirtualMachine (int index);

        /**
         * Gets the number of different virtual machines assigned to this user.
         *
         * @return Number of different virtual machines assigned to this user.
         */
        int getNumVirtualMachines();

        /**
         * Gets the type of this user.
         *
         * @return Type of this user.
         */
        const std::string& getType() const;

        /**
         * Gets the number of instances of this user.
         *
         * @return Number of instances of this user.
         */
        int getNumInstances() const;

        /**
         * Assigns a new application to this user.
         *
         * @param appPtr Pointer to the new application assigned to this user.
         * @param numInstances Number of instances of the new application.
         */
        void insertApplication(Application* appPtr, int numInstances);

        /**
         * Gets the application at index position in the <b>applications</b> vector.
         *
         * @param index Position of the application.
         *
         * @return If the requested application is located in the vector, then a pointer to its object is returned. In other case, \a nullptr is returned.
         */
        UserAppReference* getApplication (int index);

        /**
         * Gets the number of applications assigned to this user.
         *
         * @return Number of different applications, allocated in the <b>applications</b> vector, assigned to this user
         */
        int getNumApplications();

        /**
         * Converts the information of this user into a string.
         *
         * @return String containing the information of this user.
         */
        std::string toString ();

        /**
         * Gets the priority type assigned to this user.
         *
         * @return tPriorityType Priority type of the user requests in the cloud
         */
        tPriorityType getPriorityType() const;

        /**
         * Assigns a priority type to this user.
         *
         * @param priorityType Priority type of the user requests in the cloud
         */
        void setPriorityType(tPriorityType priorityType);

        /**
         * Gets the sla assigned to this user.
         *
         * @return Sla signed by the user in the cloud
         */
        Sla* getSla() const;

        /**
         * Assigns a sla to this user.
         *
         * @param sla signed by the user in the cloud
         */
        void setSla(Sla* sla);
};

#endif /* CLOUDUSER_H_ */
