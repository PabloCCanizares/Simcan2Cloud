#ifndef __SIMCAN_2_0_CLOUDMANAGER_BASE_H_
#define __SIMCAN_2_0_CLOUDMANAGER_BASE_H_

#include "Core/cSIMCAN_Core.h"
#include "Management/parser/UserListParser.h"
#include "Management/parser/SlaListParser.h"
#include "Management/parser/VmListParser.h"
#include "Management/parser/AppListParser.h"

/**
 * Base class for Cloud Managers.
 *
 * This class parses and manages VMs and users
 */
class CloudManagerBase: public cSIMCAN_Core{

    protected:
        /** Show parsed slas */
        bool showSlas;

        /** Show parsed users and VMs */
        bool showUsersVms;

        /** Shows the parsed applications */
        bool showApps;

        /** Vector that contains the set of application types used in the current simulation */
        std::vector<Application*> appTypes;

        /** Vector that contains the types of VM used in the current simulation */
        std::vector<VirtualMachine*> vmTypes;

        /** Vector that contains the types of slas generated in the current simulation */
        std::vector<Sla*> slaTypes;

        /** Vector that contains the types of users generated in the current simulation */
        std::vector<CloudUser*> userTypes;

        /**
         * Destructor
         */
        ~CloudManagerBase();

        /**
         * Initialize method. Invokes the parsing process to allocate the existing VMs and users in the corresponding data structures.
         */
        virtual void initialize();

        /**
         * Parses each application type used in the simulation.
         * These applications are allocated in the <b>appTypes</b> vector.
         *
         * @return If the parsing process is successfully executed, this method returns SC_OK. In other case, it returns SC_ERROR.
         */
        virtual int parseAppList();

        /**
         * Parses each VM type used in the simulation. These VMs are allocated in the <b>vmTypes</b> vector.
         *
         * @return If the parsing process is successfully executed, this method returns SC_OK. In other case, it returns SC_ERROR.
         */
        virtual int parseVmsList();

        /**
         * Parses each sla type used in the simulation. These slas are allocated in the <b>slaTypes</b> vector.
         *
         * @return If the parsing process is successfully executed, this method returns SC_OK. In other case, it returns SC_ERROR.
         */
        virtual int parseSlasList();

        /**
         * Parses each user type used in the simulation. These users are allocated in the <b>userTypes</b> vector.
         *
         * @return If the parsing process is successfully executed, this method returns SC_OK. In other case, it returns SC_ERROR.
         */
        virtual int parseUsersList();

       /**
        * Converts the parsed applications to string format. Usually, this method is invoked for debugging/logging purposes.
        *
        * @return String containing the parsed applications.
        */
        std::string appsToString();

        /**
         * Converts the parsed VMs into string format.
         *
         * @return A string containing the parsed VMs.
         */
        std::string vmsToString();

        /**
         * Converts the parsed sla into string format.
         *
         * @return A string containing the parsed slas.
         */
        std::string slasToString();

        /**
         * Converts the parsed user into string format.
         *
         * @return A string containing the parsed users.
         */
        std::string usersToString();

        /**
         * Search for the application <b>appName</b>.
         *
         * @param appName Instance name of the requested application.
         * @return If the requested application is located in the vector, then a pointer to its object is returned. In other case, \a nullptr is returned.
         */
         Application* findApplication (std::string appName);

        /**
         * Search for a specific type of VM.
         *
         * @param vmType Type of a VM.
         * @return If the requested type of VM is located in the vmTypes vector, then a pointer to its object is returned. In other case, a \a nullptr is returned.
         */
         VirtualMachine* findVirtualMachine (std::string vmType);

        /**
         * Search for a specific type of CloudUser.
         *
         * @param userType Type of a user.
         * @return If the requested type of user is located in the userTypes vector, then a pointer to its object is returned. In other case, \a nullptr is returned.
         */
         Sla* findSla (std::string slaType);


       /**
        * Search for a specific type of CloudUser.
        *
        * @param userType Type of a user.
        * @return If the requested type of user is located in the userTypes vector, then a pointer to its object is returned. In other case, \a nullptr is returned.
        */
        CloudUser* findUser (std::string userType);

       /**
        * Get the out Gate to the module that sent <b>msg</b>.
        *
        * @param msg Arrived message.
        * @return Gate (out) to module that sent <b>msg</b> or \a nullptr if gate not found.
        */
        virtual cGate* getOutGate (cMessage *msg) = 0;

       /**
        * Process a self message.
        *
        * @param msg Received (self) message.
        */
        virtual void processSelfMessage (cMessage *msg) = 0;

       /**
        * Process a request message.
        *
        * @param sm Incoming message.
        */
        virtual void processRequestMessage (SIMCAN_Message *sm) = 0;

       /**
        * Process a response message from an external module.
        *
        * @param sm Incoming message.
        */
        virtual void processResponseMessage (SIMCAN_Message *sm) = 0;
};

#endif
