/*
 * DataCentreInfoCollection.h
 *
 *  Created on: Jun 27, 2017
 *      Author: pablo
 */

#ifndef DATACENTREINFOCOLLECTION_H_
#define DATACENTREINFOCOLLECTION_H_

#include "DataCentre_CpuType.h"
#include "IDataCentreCollection.h"
/**
 * This class is used to handle and manage the VMs that are requested to this datacentre. For this, this class contains a simplified version, that contains the information necessary,
 * of the datacentre, nodes and its resources.
 */
class DataCentreInfoCollection: public IDataCentreCollection {

private:

    /** Map used to store the (accepted) requests performed by each user*/
    std::map<std::string, NodeResourceInfo*> id2nodeMap;

    /** Vector that contains a collection of DataCentres that structures Nodes by CPU type */
    std::vector<DataCentre_CpuType*> dataCentres;

public:

    /**
     * Constructor.
     */
    DataCentreInfoCollection();

    /**
     * Destructor.
     */
    virtual ~DataCentreInfoCollection();


    /**
     * Initialise the datacentre
     */
    virtual void initialize();

    /**
     * Checks if all the datacentres are full.
     * @return
     */
    virtual bool isCloudFull(int nRequestedCores);

    /**
     * Insert a new node into the datacentre
     * @param nDatacentre
     * @param pNode
     */
    virtual void insertNode(int nDataCentre, NodeResourceInfo* pNode);

    /**
     * This method handles a VM request
     * @param pVmRequest VM request
     * @return TRUE if this VM is accepted. FALSE i.o.c.
     */
    virtual bool handleVmRequest(NodeResourceRequest*& pVmRequest);

    /**
     * Free a VM request
     * @param strVmId Identifier of the VM
     * @return TRUE if this VM has been released successfully. FALSE i.o.c.
     */
    virtual bool freeVmRequest(std::string strVmId);

    /**
     * Get the number of available cores of the whole system (all the datacentres).
     * @return
     */
    virtual int getTotalAvailableCores();

    /**
     * Get the total number of cores of the whole system (all the datacentres).
     * @return
     */
    virtual int getTotalCores();

    /**
     * Print a summary about the datacentre sizes.
     */
    virtual void printDCSizes();
};

#endif /* DATACENTREINFOCOLLECTION_H_ */
