/*
 *  AutoDiscoveryBonjourImpl.h
 *  cefix_alterable
 *
 *  Created by Stephan Huber on 10.09.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */
 
#pragma once

#include <string>
#include <map>
#include <vector>
#include <osg/observer_ptr>

#ifdef __OBJC__
@class ServerController;
@class ClientController;
#else
class ServerController;
class ClientController;
#endif

class DiscoveredServicesCallback;


class AutoDiscoveryServerImpl {

public:
    AutoDiscoveryServerImpl(const std::string& type, unsigned int port);
    ~AutoDiscoveryServerImpl();
    
    bool needsTimer() const { return false; }
    void update() {}
private:
    ServerController* _controller;

};

class AutoDiscoveryClientImpl {
public:
    typedef std::pair< std::string, unsigned int> Address;
    typedef std::vector<Address> AddressVector;
    typedef std::map<void*, AddressVector> AddressMap;
    
    AutoDiscoveryClientImpl(const std::string& type, DiscoveredServicesCallback* cb);
    ~AutoDiscoveryClientImpl();
    DiscoveredServicesCallback* getCallback() { return _cb.get(); }
    
    void serviceAdded(void* key, const std::string& address, unsigned int port, bool is_ip6);
    void servicesRemoved(void* key);
    bool needsTimer() const { return false; }
    void update() {}
private:
    ClientController* _controller;
    osg::ref_ptr<DiscoveredServicesCallback> _cb;
    AddressMap _addresses;
    
};