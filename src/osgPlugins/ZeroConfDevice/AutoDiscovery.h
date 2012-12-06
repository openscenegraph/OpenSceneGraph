/*
 *  AutoDiscovery.h
 *  cefix_alterable
 *
 *  Created by Stephan Huber on 10.09.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#pragma once

#include <string>
#include <osg/Referenced>


class AutoDiscoveryServerImpl;
class AutoDiscoveryClientImpl;


class DiscoveredServicesCallback : public osg::Referenced {
public:
    DiscoveredServicesCallback();
    virtual bool ignoreIP6Addresses() { return false; }
    virtual void serviceAdded(const std::string& host, unsigned int port) = 0;
    virtual void serviceRemoved(const std::string& host, unsigned int port) = 0;
};

class AutoDiscovery : public osg::Referenced {
public:
    AutoDiscovery() : _serverImpl(NULL), _clientImpl(NULL) {}; 
    
    void registerService(const std::string& type, unsigned int port);
    void deregisterServices();
    void discoverServices(const std::string& type, DiscoveredServicesCallback* cb);
    
    ~AutoDiscovery();
    
    void update();
    
    bool needsContinuousUpdate() const;
    
private:
    
    AutoDiscoveryServerImpl* _serverImpl;
    AutoDiscoveryClientImpl* _clientImpl;

};
