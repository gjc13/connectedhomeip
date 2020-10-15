#include <condition_variable>
#include <mutex>
#include <thread>

#include <nlunit-test.h>

#include "platform/CHIPDeviceLayer.h"
#include "platform/Mdns.h"

using chip::DeviceLayer::MdnsResolveResult;
using chip::DeviceLayer::MdnsService;
using chip::DeviceLayer::MdnsServiceProtocol;
using chip::DeviceLayer::TextEntry;

static void HandleResolve(void * context, MdnsResolveResult * result, CHIP_ERROR error)
{
    char addrBuf[100];
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, result != nullptr);
    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);
    result->mAddress.ToString(addrBuf, sizeof(addrBuf));
    printf("Service at [%s]:%u\n", addrBuf, result->mService.mPort);
    exit(0);
}

static void HandleBrowse(void * context, MdnsService * services, size_t servicesSize, CHIP_ERROR error)
{
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);
    if (services)
    {
        printf("Mdns service size %zu\n", servicesSize);
        printf("Service name %s\n", services->mName);
        printf("Service type %s\n", services->mType);
        ChipMdnsResolve(services->mName, services->mType, services->mProtocol, INET_NULL_INTERFACEID, HandleResolve, suite);
    }
}

static void InitCallback(void * context, CHIP_ERROR error)
{
    MdnsService service;
    TextEntry entry;
    char buf[]          = "key=val";
    nlTestSuite * suite = static_cast<nlTestSuite *>(context);

    NL_TEST_ASSERT(suite, error == CHIP_NO_ERROR);

    service.interface = INET_NULL_INTERFACEID;
    service.mPort     = 80;
    strcpy(service.mName, "test");
    strcpy(service.mType, "_mock");
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    entry.mData            = reinterpret_cast<const uint8_t *>(buf);
    entry.mSize            = strlen(reinterpret_cast<const char *>(entry.mData));
    service.mTextEntryies  = &entry;
    service.mTextEntrySize = 1;

    NL_TEST_ASSERT(suite, ChipMdnsPublishService(&service) == CHIP_NO_ERROR);
    ChipMdnsBrowse("_mock", MdnsServiceProtocol::kMdnsProtocolTcp, INET_NULL_INTERFACEID, HandleBrowse, suite);
}

static void ErrorCallback(void * context, CHIP_ERROR error)
{
    if (error != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Mdns error: %d\n", static_cast<int>(error));
        abort();
    }
}

void TestMdnsPubSub(nlTestSuite * inSuite, void * inContext)
{
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, chip::DeviceLayer::ChipMdnsInit(InitCallback, ErrorCallback, inSuite) == CHIP_NO_ERROR);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}

static const nlTest sTests[] = { NL_TEST_DEF("Test Mdns::PubSub", TestMdnsPubSub), NL_TEST_SENTINEL() };

int TestMdns()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable done;
    int retVal = -1;

    std::thread t([&done, &retVal]() {
        nlTestSuite theSuite = { "CHIP DeviceLayer mdns tests", &sTests[0], nullptr, nullptr };

        nlTestRunner(&theSuite, nullptr);
        retVal = nlTestRunnerStats(&theSuite);
        done.notify_all();
    });

    if (done.wait_for(lock, std::chrono::seconds(5)) == std::cv_status::timeout)
    {
        fprintf(stderr, "mDNS test timeout, is avahi daemon running?");
        retVal = -1;
    }
    return retVal;
}
