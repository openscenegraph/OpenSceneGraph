#pragma once


#include <string>
#include <list>
#include <map>
#include <vector>
#include "dns_sd.h"
#include "AutoDiscovery.h"
#include <osg/observer_ptr>

class DiscoveredServicesCallback;

class AutoDiscoveryServerImpl {

public:
    AutoDiscoveryServerImpl(const std::string& type, unsigned int port);
    ~AutoDiscoveryServerImpl();
    bool  needsTimer() const { return true; }
    void update();
    void errorOccured() {}
    void serviceAdded();
    void serviceRemoved();

private:
    DNSServiceRef client;
    
};

class AutoDiscoveryClientImpl {
public:
    typedef std::pair< std::string, unsigned int> Address;
    typedef std::vector<Address> AddressVector;
    typedef std::map<std::string, AddressVector> AddressMap;


    AutoDiscoveryClientImpl(const std::string& type, DiscoveredServicesCallback* cb);
    ~AutoDiscoveryClientImpl();
    void update();
    void updateRef(DNSServiceRef& ref);
    bool  needsTimer() const { return true; }

    void serviceRemoved(const std::string& replyName, const std::string& replyType, const std::string& replyDomain);
    void serviceAdded(const std::string& fullname, const std::string& host, unsigned int port);
    DNSServiceRef getClient() { return client; }

    void addRef(DNSServiceRef ref) { resolveRefs.push_back(ref); }
    void removeRef(DNSServiceRef ref) {resolveRefsToDelete.push_back(ref); }
private:
    DNSServiceRef client;
    osg::observer_ptr<DiscoveredServicesCallback> _cb;
    std::list<DNSServiceRef> resolveRefs, resolveRefsToDelete;
    AddressMap _addresses;
};