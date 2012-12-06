/*
 *  AutoDiscovery.cpp
 *  cefix_alterable
 *
 *  Created by Stephan Huber on 10.09.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#include "AutoDiscovery.h"

#ifdef __APPLE__
#include "AutoDiscoveryBonjourImpl.h"
#else 
#include "AutoDiscoveryWinImpl.h"
#endif






DiscoveredServicesCallback::DiscoveredServicesCallback()
:    osg::Referenced() 
{
}


AutoDiscovery::~AutoDiscovery() 
{
    if (_clientImpl) {
        delete _clientImpl;
        _clientImpl = NULL;
    }
    deregisterServices();
}


void AutoDiscovery::registerService(const std::string& type, unsigned int port)
{
    deregisterServices();
    _serverImpl = new AutoDiscoveryServerImpl(type, port);
}

void AutoDiscovery::deregisterServices() 
{
    if (_serverImpl) {
        delete _serverImpl;
        _serverImpl = NULL; 
    }
}



void AutoDiscovery::update() 
{
    if (_serverImpl)
        _serverImpl->update();
    if (_clientImpl)
        _clientImpl->update();
}

void AutoDiscovery::discoverServices(const std::string& type, DiscoveredServicesCallback* cb)
{
    _clientImpl = new AutoDiscoveryClientImpl(type, cb);
}

bool AutoDiscovery::needsContinuousUpdate() const
{
    return (((_clientImpl) && _clientImpl->needsTimer()) || ((_serverImpl) && _serverImpl->needsTimer()));
}
