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

    protected:

        /** Vector of virtual machines required by this user */
        std::vector<UserVmReference*> virtualMachines;

        /** Required applications for this user */
        std::vector<UserAppReference*> applications;

        /** User type. Name of the user in the SIMCAN repository. */
        std::string type;

        /** Number of instances of this user to be generated in the simulated environment. */
        int numInstances;

    public:

        /**
         * Constructor.
         *
         * @param type User type.
         * @param numInstances Number of instances of this user to be created in the simulation environment.
         */
        CloudUser(std::string type, int numInstances);

        /**
         * Destructor.
         */
        virtual ~CloudUser();

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


};

#endif /* CLOUDUSER_H_ */
