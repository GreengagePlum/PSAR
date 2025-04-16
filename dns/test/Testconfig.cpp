#include "config.hpp"
#include "gtest/gtest.h"

namespace microdns
{

TEST(LoadingConfig, NoConfig)
{
    std::string path = "/dev/null";
    ConfigLoader cl(path);
    EXPECT_EQ((unsigned)0, cl.getCount());
    EXPECT_STREQ(path.c_str(), cl.getPath().c_str());
}

TEST(LoadingConfig, RealConfig)
{
    std::string path = "./microDNS.conf";
    ConfigLoader cl(path);
    EXPECT_EQ((unsigned)9, cl.getCount());
    EXPECT_STREQ(path.c_str(), cl.getPath().c_str());
}

TEST(ReadConfig, NoConfig)
{
    std::string path = "/dev/null";
    ConfigLoader cl(path);
    EXPECT_EQ((unsigned)0, cl.getCount());
    EXPECT_THROW(cl.getParam("LOCAL_DNS_IP"), std::out_of_range);
}

TEST(ReadConfig, RealConfig)
{
    std::string path = "./microDNS.conf";
    ConfigLoader cl(path);
    EXPECT_EQ((unsigned)9, cl.getCount());
    EXPECT_STREQ("127.0.0.1", cl.getParam("LOCAL_DNS_IP").c_str());
    EXPECT_STREQ("53", cl.getParam("LOCAL_DNS_PORT").c_str());
    EXPECT_STREQ("/etc/resolv.conf", cl.getParam("RESOLV_CONF").c_str());
}

} // namespace microdns
