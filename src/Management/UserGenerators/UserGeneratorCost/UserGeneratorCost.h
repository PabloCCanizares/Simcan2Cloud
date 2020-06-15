//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __SIMCAN_2_0_USERGENERATORCOST_H_
#define __SIMCAN_2_0_USERGENERATORCOST_H_

#include <algorithm>
#include "Management/UserGenerators/UserGenerator_simple/UserGenerator_simple.h"
#include "Messages/SM_UserVM_Cost_m.h"
#include "Management/dataClasses/CloudUserPriority.h"

using namespace omnetpp;

class UserGeneratorCost : public UserGenerator_simple
{
  protected:
    /**Hasmap to accelerate the management of the users*/
        std::map<std::string, int> extensionTimeHashMap;
        std::map<std::string, bool> priorizedHashMap;
        double offerAcceptanceRate;
        double offerCostIncrease;

    virtual void initialize() override;

    virtual void initializeHashMaps();

    /**
     * Updates the status of a user
     */
    virtual void updateVmUserStatus(SM_UserVM* userVm) override;

    virtual CloudUser* findUserTypeById (std::string userId);

  public:
    /**
     * Builds an App request given a user
     */
    virtual SM_UserAPP* createAppRequest(SM_UserVM* userVm) override;

    /**
     * Handles the App response sent from the CloudProvider
     * @param userApp incoming message
     */
    //virtual void handleUserAppResponse(SIMCAN_Message* userApp) override;

    /**
     * Handles the VM response received from the CloudProvider
     * @param userVm incoming message
     */
    //virtual void handleUserVmResponse(SM_UserVM* userVm) override;

    /**
     * Calculates the statistics of the experiment.
     */
    virtual void calculateStatistics() override;

    virtual bool hasToExtendVm(SM_UserAPP* userApp);

    virtual void resumeExecution(SM_UserAPP* userApp);

    virtual void endExecution(SM_UserAPP* userApp);

    /**
     * Builds the VM request which corresponds with the provided user instance.
     */
    virtual SM_UserVM* createVmRequest(CloudUserInstance* pUserInstance) override;
};

#endif
