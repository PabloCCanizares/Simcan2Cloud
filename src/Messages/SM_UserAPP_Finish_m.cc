//
// Generated file, do not edit! Created by nedtool 5.0 from Messages/SM_UserAPP_Finish.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "SM_UserAPP_Finish_m.h"

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

Register_Class(SM_UserAPP_Finish);

SM_UserAPP_Finish::SM_UserAPP_Finish(const char *name, int kind) : ::SIMCAN_Message(name,kind)
{
    this->nTotalTime = 0;
}

SM_UserAPP_Finish::SM_UserAPP_Finish(const SM_UserAPP_Finish& other) : ::SIMCAN_Message(other)
{
    copy(other);
}

SM_UserAPP_Finish::~SM_UserAPP_Finish()
{
}

SM_UserAPP_Finish& SM_UserAPP_Finish::operator=(const SM_UserAPP_Finish& other)
{
    if (this==&other) return *this;
    ::SIMCAN_Message::operator=(other);
    copy(other);
    return *this;
}

void SM_UserAPP_Finish::copy(const SM_UserAPP_Finish& other)
{
    this->userID = other.userID;
    this->strVmId = other.strVmId;
    this->strApp = other.strApp;
    this->nTotalTime = other.nTotalTime;
}

void SM_UserAPP_Finish::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::SIMCAN_Message::parsimPack(b);
    doParsimPacking(b,this->userID);
    doParsimPacking(b,this->strVmId);
    doParsimPacking(b,this->strApp);
    doParsimPacking(b,this->nTotalTime);
}

void SM_UserAPP_Finish::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::SIMCAN_Message::parsimUnpack(b);
    doParsimUnpacking(b,this->userID);
    doParsimUnpacking(b,this->strVmId);
    doParsimUnpacking(b,this->strApp);
    doParsimUnpacking(b,this->nTotalTime);
}

const char * SM_UserAPP_Finish::getUserID() const
{
    return this->userID.c_str();
}

void SM_UserAPP_Finish::setUserID(const char * userID)
{
    this->userID = userID;
}

const char * SM_UserAPP_Finish::getStrVmId() const
{
    return this->strVmId.c_str();
}

void SM_UserAPP_Finish::setStrVmId(const char * strVmId)
{
    this->strVmId = strVmId;
}

const char * SM_UserAPP_Finish::getStrApp() const
{
    return this->strApp.c_str();
}

void SM_UserAPP_Finish::setStrApp(const char * strApp)
{
    this->strApp = strApp;
}

int SM_UserAPP_Finish::getNTotalTime() const
{
    return this->nTotalTime;
}

void SM_UserAPP_Finish::setNTotalTime(int nTotalTime)
{
    this->nTotalTime = nTotalTime;
}

class SM_UserAPP_FinishDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    SM_UserAPP_FinishDescriptor();
    virtual ~SM_UserAPP_FinishDescriptor();

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

Register_ClassDescriptor(SM_UserAPP_FinishDescriptor);

SM_UserAPP_FinishDescriptor::SM_UserAPP_FinishDescriptor() : omnetpp::cClassDescriptor("SM_UserAPP_Finish", "SIMCAN_Message")
{
    propertynames = nullptr;
}

SM_UserAPP_FinishDescriptor::~SM_UserAPP_FinishDescriptor()
{
    delete[] propertynames;
}

bool SM_UserAPP_FinishDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SM_UserAPP_Finish *>(obj)!=nullptr;
}

const char **SM_UserAPP_FinishDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = { "customize",  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *SM_UserAPP_FinishDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"customize")) return "false";
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int SM_UserAPP_FinishDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int SM_UserAPP_FinishDescriptor::getFieldTypeFlags(int field) const
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
    };
    return (field>=0 && field<4) ? fieldTypeFlags[field] : 0;
}

const char *SM_UserAPP_FinishDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "userID",
        "strVmId",
        "strApp",
        "nTotalTime",
    };
    return (field>=0 && field<4) ? fieldNames[field] : nullptr;
}

int SM_UserAPP_FinishDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='u' && strcmp(fieldName, "userID")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "strVmId")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "strApp")==0) return base+2;
    if (fieldName[0]=='n' && strcmp(fieldName, "nTotalTime")==0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *SM_UserAPP_FinishDescriptor::getFieldTypeString(int field) const
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
        "string",
        "int",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : nullptr;
}

const char **SM_UserAPP_FinishDescriptor::getFieldPropertyNames(int field) const
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

const char *SM_UserAPP_FinishDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int SM_UserAPP_FinishDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Finish *pp = (SM_UserAPP_Finish *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string SM_UserAPP_FinishDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Finish *pp = (SM_UserAPP_Finish *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getUserID());
        case 1: return oppstring2string(pp->getStrVmId());
        case 2: return oppstring2string(pp->getStrApp());
        case 3: return long2string(pp->getNTotalTime());
        default: return "";
    }
}

bool SM_UserAPP_FinishDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Finish *pp = (SM_UserAPP_Finish *)object; (void)pp;
    switch (field) {
        case 0: pp->setUserID((value)); return true;
        case 1: pp->setStrVmId((value)); return true;
        case 2: pp->setStrApp((value)); return true;
        case 3: pp->setNTotalTime(string2long(value)); return true;
        default: return false;
    }
}

const char *SM_UserAPP_FinishDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *SM_UserAPP_FinishDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    SM_UserAPP_Finish *pp = (SM_UserAPP_Finish *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


