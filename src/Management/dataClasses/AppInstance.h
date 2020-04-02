#ifndef APPINSTANCE_H_
#define APPINSTANCE_H_

#include "Application.h"

/**
 *
 * Class that represents the execution of a SIMCAN application requested by a <b>UserInstance</b>.
 *
 */
class AppInstance{

    private:

        /**
         * Name that unequivocally identifies this application instance.
         */
        std::string appInstanceID;

        /**
         * Name of the application in the SIMCAN repository.
         */
        std::string appName;

        /**
         * Instance number of this application generated by <i>userID</i>.
         * This number refers to the position of this instance in the application vector (see <b>UserInstance</b>).
         */
        int instanceNumber;

        /**
         * User that generated this application instance.
         */
        std::string userID;

        /**
         * State of this application instance.
         */
        tApplicationState state;


    public:

       /**
        * Constructor.
        *
        * Generates a unique name for this application instance using the following syntax: <i>appName</i>-<i>userID</i>-[<i>currentInstanceIndex</i>/<i>totalAppInstances</i>]
        *
        * @param appName Name of the application.
        * @param currentInstanceIndex Current instance number. First instance of this application must be 0.
        * @param totalAppInstances Total number of application instances to be created by <i>userID</i>. This parameter is only used to generate the <b>appInstanceID</b>.
        * @param userID User that generated this application instance.
        */
        AppInstance(std::string appName, int currentInstanceIndex, int totalAppInstances, std::string userID);

       /**
        * Destructor.
        */
        virtual ~AppInstance();

        /**
         * Gets the ID that unequivocally identifies this application instance.
         *
         * @return ID that unequivocally identifies this application instance.
         */
        const std::string& getAppInstanceId() const;

        /**
         * Gets the name of this application.
         *
         * @return Name of this application.
         */
        const std::string& getAppName() const;

        /**
         * Gets the number of this instance.
         *
         * @return number of this instance.
         */
        int getInstanceNumber() const;

        /**
         * Gets the state of this application instance.
         *
         * @return State of this application instance.
         */
        tApplicationState getState() const;

        /**
         * Sets a new state for this application instance.
         *
         * @param state New state for this application instance.
         */
        void setState(tApplicationState state);

        /**
         * Gets the user ID that created this application instance.
         *
         * @return user ID that created this application instance.
         */
        const std::string& getUserId() const;


        /**
         * Parses this application instance into string format.
         *
         * @return String containing this application instance into string format.
         */
        std::string toString ();


    private:

        /**
         * Parses the state of this application into string format.
         *
         * @param appState An state of this application instance.
         * @return String containing the state of this application.
         */
        std::string stateToString (tApplicationState appState);

};

#endif /* APPINSTANCE_H_ */
