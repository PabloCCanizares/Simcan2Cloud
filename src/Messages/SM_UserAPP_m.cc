//
// Generated file, do not edit! Created by nedtool 5.0 from Messages/SM_UserAPP.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "SM_UserAPP_m.h"

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

SM_UserAPP_Base::SM_UserAPP_Base(const char *name, int kind) : ::SIMCAN_Message(name,kind)
{
    app_arraysize = 0;
    this->app = 0;
    this->nFinishedApps = 0;
    this->finished = false;
}

SM_UserAPP_Base::SM_UserAPP_Base(const SM_UserAPP_Base& other) : ::SIMCAN_Message(other)
{
    app_arraysize = 0;
    this->app = 0;
    copy(other);
}

SM_UserAPP_Base::~SM_UserAPP_Base()
{
    delete [] this->app;
}

SM_UserAPP_Base& SM_UserAPP_Base::operator=(const SM_UserAPP_Base& other)
{
    if (this==&other) return *this;
    ::SIMCAN_Message::operator=(other);
    copy(other);
    return *this;
}

void SM_UserAPP_Base::copy(const SM_UserAPP_Base& other)
{
    this->vmId = other.vmId;
    this->userID = other.userID;
    delete [] this->app;
    this->app = (other.app_arraysize==0) ? nullptr : new APP_Request[other.app_arraysize];
    app_arraysize = other.app_arraysize;
    for (unsigned int i=0; i<app_arraysize; i++)
        this->app[i] = other.app[i];
    this->nFinishedApps = other.nFinishedApps;
    this->finished = other.finished;
}

void SM_UserAPP_Base::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::SIMCAN_Message::parsimPack(b);
    doParsimPacking(b,this->vmId);
    doParsimPacking(b,this->userID);
    b->pack(app_arraysize);
    doParsimArrayPacking(b,this->app,app_arraysize);
    doParsimPacking(b,this->nFinishedApps);
    doParsimPacking(b,this->finished);
}

void SM_UserAPP_Base::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::SIMCAN_Message::parsimUnpack(b);
    doParsimUnpacking(b,this->vmId);
    doParsimUnpacking(b,this->userID);
    delete [] this->app;
    b->unpack(app_arraysize);
    if (app_arraysize==0) {
        this->app = 0;
    } else {
        this->app = new APP_Request[app_arraysize];
        doParsimArrayUnpacking(b,this->app,app_arraysize);
    }
    doParsimUnpacking(b,this->nFinishedApps);
    doParsimUnpacking(b,this->finished);
}

const char * SM_UserAPP_Base::getVmId() const
{
    return this->vmId.c_str();
}

void SM_UserAPP_Base::setVmId(const char * vmId)
{
    this->vmId = vmId;
}

const char * SM_UserAPP_Base::getUserID() const
{
    return this->userID.c_str();
}

void SM_UserAPP_Base::setUserID(const char * userID)
{
    this->userID = userID;
}

void SM_UserAPP_Base::setAppArraySize(unsigned int size)
{
    APP_Request *app2 = (size==0) ? nullptr : new APP_Request[size];
    unsigned int sz = app_arraysize < size ? app_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        app2[i] = this->app[i];
    app_arraysize = size;
    delete [] this->app;
    this->app = app2;
}

unsigned int SM_UserAPP_Base::getAppArraySize() const
{
    return app_arraysize;
}

APP_Request& SM_UserAPP_Base::getApp(unsigned int k)
{
    if (k>=app_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", app_arraysize, k);
    return this->app[k];
}

void SM_UserAPP_Base::setApp(unsigned int k, const APP_Request& app)
{
    if (k>=app_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", app_arraysize, k);
    this->app[k] = app;
}

int SM_UserAPP_Base::getNFinishedApps() const
{
    return this->nFinishedApps;
}

void SM_UserAPP_Base::setNFinishedApps(int nFinishedApps)
{
    this->nFinishedApps = nFinishedApps;
}

bool SM_UserAPP_Base::getFinished() const
{
    return this->finished;
}

void SM_UserAPP_Base::setFinished(bool finished)
{
    this->finished = finished;
}

class SM_UserAPPDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    SM_UserAPPDescriptor();
    virtual ~SM_UserAPPDescriptor();

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

Register_ClassDescriptor(SM_UserAPPDescriptor);

SM_UserAPPDescriptor::SM_UserAPPDescriptor() : omnetpp::cClassDescriptor("SM_UserAPP", "SIMCAN_Message")
{
    propertynames = nullptr;
}

SM_UserAPPDescriptor::~SM_UserAPPDescriptor()
{
    delete[] propertynames;
}

bool SM_UserAPPDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SM_UserAPP_Base *>(obj)!=nullptr;
}

const char **SM_UserAPPDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = { "customize",  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *SM_UserAPPDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"customize")) return "true";
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int SM_UserAPPDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int SM_UserAPPDescriptor::getFieldTypeFlags(int field) const
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
        FD_ISARRAY | FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *SM_UserAPPDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "vmId",
        "userID",
        "app",
        "nFinishedApps",
        "finished",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int SM_UserAPPDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='v' && strcmp(fieldName, "vmId")==0) return base+0;
    if (fieldName[0]=='u' && strcmp(fieldName, "userID")==0) return base+1;
    if (fieldName[0]=='a' && strcmp(fieldName, "app")==0) return base+2;
    if (fieldName[0]=='n' && strcmp(fieldName, "nFinishedApps")==0) return base+3;
    if (fieldName[0]=='f' && strcmp(fieldName, "finished")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *SM_UserAPPDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "string",
        "APP_Request",
        "int",
        "bool",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **SM_UserAPPDescriptor::getFieldPropertyNames(int field) const
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

const char *SM_UserAPPDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int SM_UserAPPDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Base *pp = (SM_UserAPP_Base *)object; (void)pp;
    switch (field) {
        case 2: return pp->getAppArraySize();
        default: return 0;
    }
}

std::string SM_UserAPPDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Base *pp = (SM_UserAPP_Base *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getVmId());
        case 1: return oppstring2string(pp->getUserID());
        case 2: {std::stringstream out; out << pp->getApp(i); return out.str();}
        case 3: return long2string(pp->getNFinishedApps());
        case 4: return bool2string(pp->getFinished());
        default: return "";
    }
}

bool SM_UserAPPDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Base *pp = (SM_UserAPP_Base *)object; (void)pp;
    switch (field) {
        case 0: pp->setVmId((value)); return true;
        case 1: pp->setUserID((value)); return true;
        case 3: pp->setNFinishedApps(string2long(value)); return true;
        case 4: pp->setFinished(string2bool(value)); return true;
        default: return false;
    }
}

const char *SM_UserAPPDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 2: return omnetpp::opp_typename(typeid(APP_Request));
        default: return nullptr;
    };
}

void *SM_UserAPPDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Base *pp = (SM_UserAPP_Base *)object; (void)pp;
    switch (field) {
        case 2: return (void *)(&pp->getApp(i)); break;
        default: return nullptr;
    }
}


