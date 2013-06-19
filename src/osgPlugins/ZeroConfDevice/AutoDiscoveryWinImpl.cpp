#include "AutoDiscoveryWinImpl.h"
#include "Autodiscovery.h"
#include "dns_sd.h"
#include <sstream>

typedef union { unsigned char b[2]; unsigned short NotAnInteger; } Opaque16;

#ifndef HeapEnableTerminationOnCorruption
#     define HeapEnableTerminationOnCorruption (HEAP_INFORMATION_CLASS)1
#endif

static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
    const char *name, const char *regtype, const char *domain, void *context)
{
    if (context) {
        AutoDiscoveryServerImpl* impl = static_cast<AutoDiscoveryServerImpl*>(context);
        
        if (errorCode == kDNSServiceErr_NoError)
        {
            if (flags & kDNSServiceFlagsAdd) impl->serviceAdded(); 
            else impl->serviceRemoved(); 
        }
    
        else if (errorCode == kDNSServiceErr_NameConflict)
        {
            OSG_WARN << "AutoDiscoveryServerImpl :: Name conflict" << std::endl;
            impl->errorOccured();
        }
        else {
            OSG_WARN << "AutoDiscoveryServerImpl :: error " << errorCode << std::endl;
            impl->errorOccured();
        }
    }
}

static DNSServiceErrorType RegisterService(DNSServiceRef *sdref,
    const char *nam, const char *typ, const char *dom, const char *host, const char *port, AutoDiscoveryServerImpl* context)
    {
    DNSServiceFlags flags = 0;
    uint16_t PortAsNumber = atoi(port);
    Opaque16 registerPort = { { PortAsNumber >> 8, PortAsNumber & 0xFF } };
    unsigned char txt[2048] = "";
    unsigned char *ptr = txt;
    
    if (nam[0] == '.' && nam[1] == 0) nam = "";   // We allow '.' on the command line as a synonym for empty string
    if (dom[0] == '.' && dom[1] == 0) dom = "";   // We allow '.' on the command line as a synonym for empty string
    
    OSG_INFO << "AutoDiscoveryImpl :: Registering Service " << (nam[0] ? nam : "<<Default>>") << " " << typ << " " << (dom[0] ? "." : "", dom) << std::endl;
    if (host && *host) {
        OSG_INFO << "AutoDiscoveryImpl :: host " << host << " port " << port << std::endl;
    }

    // printf("\n");
    
    //flags |= kDNSServiceFlagsAllowRemoteQuery;
    //flags |= kDNSServiceFlagsNoAutoRename;
    
    return(DNSServiceRegister(sdref, flags, kDNSServiceInterfaceIndexAny, nam, typ, dom, host, registerPort.NotAnInteger, (uint16_t) (ptr-txt), txt, reg_reply, context));
}


AutoDiscoveryServerImpl::AutoDiscoveryServerImpl(const std::string& type, unsigned int port)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    std::ostringstream ss;
    ss << port;
    DNSServiceErrorType err = RegisterService(&client, "", type.c_str(), "", NULL, ss.str().c_str(), this);
    if (!client || err != kDNSServiceErr_NoError) { 
        OSG_WARN << "AutoDiscoveryImpl :: DNSService call failed " << (long int)err << std::endl; 
    }
}

void AutoDiscoveryServerImpl::update()
{
    int dns_sd_fd  = client    ? DNSServiceRefSockFD(client   ) : -1;
    int nfds = dns_sd_fd + 1;
    fd_set readfds;
    struct timeval tv;
    int result;
    
//     (dns_sd_fd2 > dns_sd_fd) nfds = dns_sd_fd2 + 1;

    // 1. Set up the fd_set as usual here.
    // This example client has no file descriptors of its own,
    // but a real application would call FD_SET to add them to the set here
    FD_ZERO(&readfds);

    // 2. Add the fd for our client(s) to the fd_set
    if (client   ) FD_SET(dns_sd_fd , &readfds);
    //if (client_pa) FD_SET(dns_sd_fd2, &readfds);

    // 3. Set up the timeout.
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
    if (result > 0)
    {
        DNSServiceErrorType err = kDNSServiceErr_NoError;
        if      (client    && FD_ISSET(dns_sd_fd , &readfds)) err = DNSServiceProcessResult(client   );
//        else if (client_pa && FD_ISSET(dns_sd_fd2, &readfds)) err = DNSServiceProcessResult(client_pa);
//        if (err) { fprintf(stderr, "DNSServiceProcessResult returned %d\n", err); stopNow = 1; }
    }
}

void AutoDiscoveryServerImpl::serviceAdded()
{
    OSG_INFO << "AutoDiscoveryImpl :: Service added" << std::endl;
}
void AutoDiscoveryServerImpl::serviceRemoved()
{
    OSG_INFO << "AutoDiscoveryImpl :: Service removed" << std::endl;
}

struct ContextDNSServiceRefPair {
    DNSServiceRef ref;
    AutoDiscoveryClientImpl* context;
};

static void DNSSD_API zonedata_resolve(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
    const char *fullname, const char *hosttarget, uint16_t opaqueport, uint16_t txtLen, const unsigned char *txt, void *context)
    {
    union { uint16_t s; u_char b[2]; } port = { opaqueport };
    uint16_t PortAsNumber = ((uint16_t)port.b[0]) << 8 | port.b[1];
    
    ContextDNSServiceRefPair* ref_pair = static_cast<ContextDNSServiceRefPair*>(context);
    if (!ref_pair) return;
    if (errorCode) { 
        OSG_WARN << "AutoDiscoveryImpl :: Error code " << errorCode << std::endl; 
        return; 
    }
    ref_pair->context->serviceAdded(fullname, hosttarget, PortAsNumber);
    /*
    const char *p = fullname;
    char n[kDNSServiceMaxDomainName];
    char t[kDNSServiceMaxDomainName];

    const unsigned char *max = txt + txtLen;

    (void)sdref;        // Unused
    (void)ifIndex;      // Unused
    (void)context;      // Unused

    //if (!(flags & kDNSServiceFlagsAdd)) return;
    if (errorCode) { printf("Error code %d\n", errorCode); return; }

    if (CopyLabels(n, n + kDNSServiceMaxDomainName, &p, 3)) return;        // Fetch name+type
    p = fullname;
    if (CopyLabels(t, t + kDNSServiceMaxDomainName, &p, 1)) return;        // Skip first label
    if (CopyLabels(t, t + kDNSServiceMaxDomainName, &p, 2)) return;        // Fetch next two labels (service type)

    if (num_printed++ == 0)
        {
        printf("\n");
        printf("; To direct clients to browse a different domain, substitute that domain in place of '@'\n");
        printf("%-47s PTR     %s\n", "lb._dns-sd._udp", "@");
        printf("\n");
        printf("; In the list of services below, the SRV records will typically reference dot-local Multicast DNS names.\n");
        printf("; When transferring this zone file data to your unicast DNS server, you'll need to replace those dot-local\n");
        printf("; names with the correct fully-qualified (unicast) domain name of the target host offering the service.\n");
        }

    printf("\n");
    printf("%-47s PTR     %s\n", t, n);
    printf("%-47s SRV     0 0 %d %s ; Replace with unicast FQDN of target host\n", n, PortAsNumber, hosttarget);
    printf("%-47s TXT    ", n);

    while (txt < max)
        {
        const unsigned char *const end = txt + 1 + txt[0];
        txt++;        // Skip over length byte
        printf(" \"");
        while (txt<end)
            {
            if (*txt == '\\' || *txt == '\"') printf("\\");
            printf("%c", *txt++);
            }
        printf("\"");
        }
    printf("\n");
    */
    ref_pair->context->removeRef(ref_pair->ref);
    DNSServiceRefDeallocate(ref_pair->ref);
    free(ref_pair);
}


static void DNSSD_API browse_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
    const char *replyName, const char *replyType, const char *replyDomain, void *context)
{
    if (errorCode) {
        OSG_WARN << "AutoDiscoveryImpl :: Error code " << errorCode << std::endl;
        return;
    }

    AutoDiscoveryClientImpl* impl = static_cast<AutoDiscoveryClientImpl*>(context);
    if(!impl) return;

    if  (!(flags & kDNSServiceFlagsAdd)) {
        impl->serviceRemoved(replyName, replyType, replyDomain);
        return;
    }
    ContextDNSServiceRefPair* ref_pair = new ContextDNSServiceRefPair();
    
    ref_pair->context = static_cast<AutoDiscoveryClientImpl*>(context);
    DNSServiceErrorType err = DNSServiceResolve(&ref_pair->ref, 0, ifIndex, replyName, replyType, replyDomain, zonedata_resolve, ref_pair);
    if (!ref_pair->ref || err != kDNSServiceErr_NoError) { 
        OSG_WARN << "AutoDiscoveryImpl :: DNSServiceResolve call failed " << (long int)err << std::endl; 
    } else
        impl->addRef(ref_pair->ref);
}


AutoDiscoveryServerImpl::~AutoDiscoveryServerImpl()
{
    if (client) DNSServiceRefDeallocate(client);
}


AutoDiscoveryClientImpl::AutoDiscoveryClientImpl(const std::string& type, DiscoveredServicesCallback* cb)
{
    _cb = cb;
    DNSServiceErrorType err = DNSServiceBrowse(&client, 0, kDNSServiceInterfaceIndexAny, type.c_str(), "", browse_reply, this);
    if (!client || err != kDNSServiceErr_NoError) { 
        OSG_WARN << "AutoDiscoveryImpl :: DNSServiceBrowse call failed " << (long int)err << std::endl; 
    }
}
void AutoDiscoveryClientImpl::update() 
{
    updateRef(client);
    for(std::list<DNSServiceRef>::iterator i = resolveRefs.begin(); i != resolveRefs.end(); ++i) {
        updateRef(*i);
    }
    for(std::list<DNSServiceRef>::iterator i = resolveRefsToDelete.begin(); i != resolveRefsToDelete.end(); ++i) {
        resolveRefs.remove(*i);
    }
    resolveRefsToDelete.clear();
}

void AutoDiscoveryClientImpl::updateRef(DNSServiceRef& ref)
{
    int dns_sd_fd  = ref    ? DNSServiceRefSockFD(ref   ) : -1;
    int nfds = dns_sd_fd + 1;
    fd_set readfds;
    struct timeval tv;
    int result;
    
//     (dns_sd_fd2 > dns_sd_fd) nfds = dns_sd_fd2 + 1;

    // 1. Set up the fd_set as usual here.
    // This example client has no file descriptors of its own,
    // but a real application would call FD_SET to add them to the set here
    FD_ZERO(&readfds);

    // 2. Add the fd for our client(s) to the fd_set
    if (ref   ) FD_SET(dns_sd_fd , &readfds);
    //if (client_pa) FD_SET(dns_sd_fd2, &readfds);

    // 3. Set up the timeout.
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
    if (result > 0)
    {
        DNSServiceErrorType err = kDNSServiceErr_NoError;
        if      (client    && FD_ISSET(dns_sd_fd , &readfds)) err = DNSServiceProcessResult(ref   );
//        else if (client_pa && FD_ISSET(dns_sd_fd2, &readfds)) err = DNSServiceProcessResult(client_pa);
//        if (err) { fprintf(stderr, "DNSServiceProcessResult returned %d\n", err); stopNow = 1; }
    }
}

AutoDiscoveryClientImpl::~AutoDiscoveryClientImpl()
{
    if (client) DNSServiceRefDeallocate(client);
}


void AutoDiscoveryClientImpl::serviceRemoved(const std::string& replyName, const std::string& replyType, const std::string& replyDomain)
{
    // TODO
    std::string full_name = replyName+"."+replyType+"."+replyDomain;
    // TODO full_name = cefix::strReplaceAll<std::string>(full_name, "..", ".");
    AddressMap::iterator itr = _addresses.find(full_name);
    if (itr == _addresses.end()) {
        OSG_INFO << "AutoDiscoveryImpl :: no services found to remove? " << full_name << std::endl;
        return;
    }
    if (!_cb.valid())
        return;

    for(AddressVector::iterator i = itr->second.begin(); i != itr->second.end(); ++i) {
        _cb->serviceRemoved(i->first, i->second);
    }
    _addresses.erase(itr);
}


void AutoDiscoveryClientImpl::serviceAdded(const std::string& fullname, const std::string& host, unsigned int port)
{
    if (_cb.valid()) {
        _addresses[fullname].push_back(std::make_pair(host, port));
        _cb->serviceAdded(host, port);
    }

}