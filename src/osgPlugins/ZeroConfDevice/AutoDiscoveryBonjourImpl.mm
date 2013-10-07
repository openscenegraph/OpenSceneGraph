/*
 *  AutoDiscoveryBonjourImpl.cpp
 *  cefix_alterable
 *
 *  Created by Stephan Huber on 10.09.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#include "AutoDiscoveryBonjourImpl.h"
#import "TargetConditionals.h" 
#if (TARGET_OS_IPHONE)
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#include "AutoDiscovery.h"
#include <arpa/inet.h>

@interface ServerController : NSObject<NSNetServiceDelegate> {
    NSNetService *netService;
}

-(void)startServiceWithType: (NSString*) type withPort: (unsigned int) port;
-(void)stopService;

@end


@implementation ServerController


-(void)startServiceWithType:(NSString*) type withPort: (unsigned int) port;
{
    netService = [[NSNetService alloc] initWithDomain:@"" type: type 
        name:@"" port:port];
    netService.delegate = self;
    [netService publish];
}

-(void)stopService {
    [netService stop];
    [netService release]; 
    netService = nil;
}

-(void)dealloc {
    [self stopService];
    [super dealloc];
}

#pragma mark Net Service Delegate Methods
-(void)netService:(NSNetService *)aNetService didNotPublish:(NSDictionary *)dict {
    NSLog(@"Failed to publish: %@", dict);
}

@end

@interface ClientController : NSObject<NSNetServiceBrowserDelegate, NSNetServiceDelegate> {
    BOOL isConnected;
    NSNetServiceBrowser *browser;
    NSNetService *connectedService;
    NSMutableArray *services;
    NSString* type;
    AutoDiscoveryClientImpl* impl;
}

@property (readonly, retain) NSMutableArray *services;
@property (readwrite, retain) NSString *type;

@property (readwrite, retain) NSNetServiceBrowser *browser;

@end

@implementation ClientController

@synthesize browser;
@synthesize type;
@synthesize services;

-(id)initWithType: (NSString*) in_type  withImpl:(AutoDiscoveryClientImpl*) in_impl {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    services = [NSMutableArray new];
    self.browser = [[NSNetServiceBrowser new] init];
    self.browser.delegate = self;
    self.type = in_type;
    impl = in_impl;
    [self.browser searchForServicesOfType:in_type inDomain:@""];
    [pool release];
    return [super init];
}

-(void)dealloc {
    
    [self.browser stop];
    self.browser.delegate = nil;
    
    self.browser = nil;
    [services makeObjectsPerformSelector:@selector(setDelegate:) withObject:nil];
    [services makeObjectsPerformSelector:@selector(stop)];
    [services release];
    
    [super dealloc];
}



#pragma mark Net Service Browser Delegate Methods

-(void)netServiceBrowser:(NSNetServiceBrowser *)aBrowser didFindService:(NSNetService *)aService moreComing:(BOOL)more 
{
    [services addObject:aService];
    aService.delegate = self;
    [aService resolveWithTimeout:0];
}

-(void)netServiceBrowser:(NSNetServiceBrowser *)aBrowser didRemoveService:(NSNetService *)aService moreComing:(BOOL)more 
{
    int ndx = [services indexOfObject: aService];
    impl->servicesRemoved([services objectAtIndex: ndx]);
    aService.delegate = nil;
    [services removeObject:aService];
}

-(void)netServiceDidResolveAddress:(NSNetService *)service {
    
    //NSLog(@"hostname: %@", [service hostName]);
    
    for (NSData* data in [service addresses]) {

        char addressBuffer[100];
        struct sockaddr_in* socketAddress = (struct sockaddr_in*) [data bytes];

        int sockFamily = socketAddress->sin_family;
        if (sockFamily == AF_INET || sockFamily == AF_INET6) {

            const char* addressStr = inet_ntop(sockFamily,
                                &(socketAddress->sin_addr), addressBuffer,
                                sizeof(addressBuffer));

            int port = ntohs(socketAddress->sin_port);

            if (addressStr && port) 
            {
                impl->serviceAdded(service, addressStr, port, sockFamily == AF_INET6);
            }
        }

    }
}


-(void)netService:(NSNetService *)service didNotResolve:(NSDictionary *)errorDict {
    NSLog(@"Could not resolve: %@", errorDict);
}

@end



AutoDiscoveryServerImpl::AutoDiscoveryServerImpl(const std::string& type, unsigned int port)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    OSG_INFO <<"AutoDiscoveryServerImpl :: registering service " << type << " port: " << port << std::endl;
    
    _controller = [[ServerController alloc] init];
    [_controller startServiceWithType: [NSString stringWithUTF8String: type.c_str()] withPort: port];
    [pool release];
}

AutoDiscoveryServerImpl::~AutoDiscoveryServerImpl()
{
    [_controller release];
}


AutoDiscoveryClientImpl::AutoDiscoveryClientImpl(const std::string& type, DiscoveredServicesCallback* cb)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    _cb = cb;
    _controller = [[ClientController alloc] initWithType: [NSString stringWithUTF8String: type.c_str()] withImpl: this];
    
    [pool release];
}


void AutoDiscoveryClientImpl::serviceAdded(void* key, const std::string& address, unsigned int port, bool is_ip6)
{    
    if (getCallback()) 
    {
        if ((!is_ip6) || (is_ip6 && !getCallback()->ignoreIP6Addresses())) 
        {
            _addresses[key].push_back(std::make_pair(address, port));
            getCallback()->serviceAdded(address, port);
        }
    }
}    


void AutoDiscoveryClientImpl::servicesRemoved(void* key)
{
    if (!getCallback()) return;
    
    AddressMap::iterator itr = _addresses.find(key);
    if (itr != _addresses.end()) {
        AddressVector& addresses = itr->second;
        for(AddressVector::iterator i = addresses.begin(); i != addresses.end(); ++i) 
        {
            getCallback()->serviceRemoved(i->first, i->second);
        }
    }
}

AutoDiscoveryClientImpl::~AutoDiscoveryClientImpl()
{
    [_controller release];
}
