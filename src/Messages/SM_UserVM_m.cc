//
// Generated file, do not edit! Created by nedtool 5.0 from Messages/SM_UserVM.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "SM_UserVM_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: no doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: no doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

SM_UserVM_Base::SM_UserVM_Base(const char *name, int kind) : ::SIMCAN_Message(name,kind)
{
    this->dStartSubscriptionTime = 0;
    this->dEndSubscriptionTime = 0;
    vms_arraysize = 0;
    this->vms = 0;
}

SM_UserVM_Base::SM_UserVM_Base(const SM_UserVM_Base& other) : ::SIMCAN_Message(other)
{
    vms_arraysize = 0;
    this->vms = 0;
    copy(other);
}

SM_UserVM_Base::~SM_UserVM_Base()
{
    delete [] this->vms;
}

SM_UserVM_Base& SM_UserVM_Base::operator=(const SM_UserVM_Base& other)
{
    if (this==&other) return *this;
    ::SIMCAN_Message::operator=(other);
    copy(other);
    return *this;
}

void SM_UserVM_Base::copy(const SM_UserVM_Base& other)
{
    this->dStartSubscriptionTime = other.dStartSubscriptionTime;
    this->dEndSubscriptionTime = other.dEndSubscriptionTime;
    this->strVmId = other.strVmId;
    this->userID = other.userID;
    delete [] this->vms;
    this->vms = (other.vms_arraysize==0) ? nullptr : new VM_Request[other.vms_arraysize];
    vms_arraysize = other.vms_arraysize;
    for (unsigned int i=0; i<vms_arraysize; i++)
        this->vms[i] = other.vms[i];
}

void SM_UserVM_Base::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::SIMCAN_Message::parsimPack(b);
    doParsimPacking(b,this->dStartSubscriptionTime);
    doParsimPacking(b,this->dEndSubscriptionTime);
    doParsimPacking(b,this->strVmId);
    doParsimPacking(b,this->userID);
    b->pack(vms_arraysize);
    doParsimArrayPacking(b,this->vms,vms_arraysize);
}

void SM_UserVM_Base::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::SIMCAN_Message::parsimUnpack(b);
    doParsimUnpacking(b,this->dStartSubscriptionTime);
    doParsimUnpacking(b,this->dEndSubscriptionTime);
    doParsimUnpacking(b,this->strVmId);
    doParsimUnpacking(b,this->userID);
    delete [] this->vms;
    b->unpack(vms_arraysize);
    if (vms_arraysize==0) {
        this->vms = 0;
    } else {
        this->vms = new VM_Request[vms_arraysize];
        doParsimArrayUnpacking(b,this->vms,vms_arraysize);
    }
}

double SM_UserVM_Base::getDStartSubscriptionTime() const
{
    return this->dStartSubscriptionTime;
}

void SM_UserVM_Base::setDStartSubscriptionTime(double dStartSubscriptionTime)
{
    this->dStartSubscriptionTime = dStartSubscriptionTime;
}

double SM_UserVM_Base::getDEndSubscriptionTime() const
{
    return this->dEndSubscriptionTime;
}

void SM_UserVM_Base::setDEndSubscriptionTime(double dEndSubscriptionTime)
{
    this->dEndSubscriptionTime = dEndSubscriptionTime;
}

const char * SM_UserVM_Base::getStrVmId() const
{
    return this->strVmId.c_str();
}

void SM_UserVM_Base::setStrVmId(const char * strVmId)
{
    this->strVmId = strVmId;
}

const char * SM_UserVM_Base::getUserID() const
{
    return this->userID.c_str();
}

void SM_UserVM_Base::setUserID(const char * userID)
{
    this->userID = userID;
}

void SM_UserVM_Base::setVmsArraySize(unsigned int size)
{
    VM_Request *vms2 = (size==0) ? nullptr : new VM_Request[size];
    unsigned int sz = vms_arraysize < size ? vms_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        vms2[i] = this->vms[i];
    vms_arraysize = size;
    delete [] this->vms;
    this->vms = vms2;
}

unsigned int SM_UserVM_Base::getVmsArraySize() const
{
    return vms_arraysize;
}

VM_Request& SM_UserVM_Base::getVms(unsigned int k)
{
    if (k>=vms_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", vms_arraysize, k);
    return this->vms[k];
}

void SM_UserVM_Base::setVms(unsigned int k, const VM_Request& vms)
{
    if (k>=vms_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", vms_arraysize, k);
    this->vms[k] = vms;
}

class SM_UserVMDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    SM_UserVMDescriptor();
    virtual ~SM_UserVMDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(SM_UserVMDescriptor);

SM_UserVMDescriptor::SM_UserVMDescriptor() : omnetpp::cClassDescriptor("SM_UserVM", "SIMCAN_Message")
{
    propertynames = nullptr;
}

SM_UserVMDescriptor::~SM_UserVMDescriptor()
{
    delete[] propertynames;
}

bool SM_UserVMDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SM_UserVM_Base *>(obj)!=nullptr;
}

const char **SM_UserVMDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = { "customize",  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *SM_UserVMDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"customize")) return "true";
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int SM_UserVMDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int SM_UserVMDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISARRAY | FD_ISCOMPOUND,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *SM_UserVMDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dStartSubscriptionTime",
        "dEndSubscriptionTime",
        "strVmId",
        "userID",
        "vms",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int SM_UserVMDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dStartSubscriptionTime")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dEndSubscriptionTime")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "strVmId")==0) return base+2;
    if (fieldName[0]=='u' && strcmp(fieldName, "userID")==0) return base+3;
    if (fieldName[0]=='v' && strcmp(fieldName, "vms")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *SM_UserVMDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "double",
        "double",
        "string",
        "string",
        "VM_Request",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **SM_UserVMDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SM_UserVMDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SM_UserVMDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    SM_UserVM_Base *pp = (SM_UserVM_Base *)object; (void)pp;
    switch (field) {
        case 4: return pp->getVmsArraySize();
        default: return 0;
    }
}

std::string SM_UserVMDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SM_UserVM_Base *pp = (SM_UserVM_Base *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->getDStartSubscriptionTime());
        case 1: return double2string(pp->getDEndSubscriptionTime());
        case 2: return oppstring2string(pp->getStrVmId());
        case 3: return oppstring2string(pp->getUserID());
        case 4: {std::stringstream out; out << pp->getVms(i); return out.str();}
        default: return "";
    }
}

bool SM_UserVMDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    SM_UserVM_Base *pp = (SM_UserVM_Base *)object; (void)pp;
    switch (field) {
        case 0: pp->setDStartSubscriptionTime(string2double(value)); return true;
        case 1: pp->setDEndSubscriptionTime(string2double(value)); return true;
        case 2: pp->setStrVmId((value)); return true;
        case 3: pp->setUserID((value)); return true;
        default: return false;
    }
}

const char *SM_UserVMDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 4: return omnetpp::opp_typename(typeid(VM_Request));
        default: return nullptr;
    };
}

void *SM_UserVMDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    SM_UserVM_Base *pp = (SM_UserVM_Base *)object; (void)pp;
    switch (field) {
        case 4: return (void *)(&pp->getVms(i)); break;
        default: return nullptr;
    }
}


