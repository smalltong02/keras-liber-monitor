#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include "StaticPEManager\StaticFileManager.h"

#ifdef _FUNCTION_TEST
class TraceStaticPETest : public testing::Test
{
protected:
    TraceStaticPETest() {}
    ~TraceStaticPETest() override {}

    virtual void SetUp() override {}

    void TearDown() override {
    }
    
    static const std::vector<std::string> m_file_list;
    static const std::string m_folder;
private:
};

const std::vector<std::string> TraceStaticPETest::m_file_list = {
    "..\\samples\\test_vb.exe",
    "..\\samples\\ILSpy.exe",
    "..\\samples\\procexp.exe",
    "..\\samples\\die.exe",
    "..\\samples\\dpinst.exe",
    "..\\samples\\fortitcs.exe",
    "..\\samples\\CyUcmClient.sys",
    "..\\samples\\CyUcmClient.inf",
    "..\\samples\\HfcDisableService.exe",
    "..\\samples\\HipsHookTest.exe",
    "..\\samples\\test_visualbasic.exe",
};

const std::string TraceStaticPETest::m_folder = "..\\samples";

// test class CJsonOptions
TEST_F(TraceStaticPETest, JsonOptions_Class_Test)
{
    CJsonOptions options("CFGRES", IDR_JSONPE_CFG);
    ASSERT_TRUE(options.Parse());
    EXPECT_TRUE(options.RequestBaseInfo());
    EXPECT_TRUE(options.RequestVersionInfo());
    EXPECT_TRUE(options.RequestInstallInfo());
    EXPECT_TRUE(options.RequestSignatureInfo());
    EXPECT_TRUE(options.RequestPeinsideInfo());
    EXPECT_TRUE(options.RequestPeinsnflowInfo());
    auto& config = options.GetConfigInfo();
    EXPECT_TRUE(config.MatchPeType("PE"));
    EXPECT_FALSE(config.MatchPeType("PE48"));
    EXPECT_TRUE(config.MatchSubsystem("win_gui"));
    EXPECT_TRUE(config.MatchSubsystem("win_cui"));
    EXPECT_FALSE(config.MatchSubsystem("os2"));
    EXPECT_TRUE(config.IsNormalMode());

    CJsonOptions::_base_info base_info{};
    ASSERT_TRUE(options.GetOptionsInfo(CJsonOptions::_info_type::info_type_base, std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&base_info), sizeof(base_info))));
    EXPECT_TRUE(base_info.bfilename);
    EXPECT_TRUE(base_info.blocation);
    EXPECT_TRUE(base_info.bfiletype);
    EXPECT_TRUE(base_info.bfilesize);
    EXPECT_TRUE(base_info.bfileattribute);
    EXPECT_FALSE(base_info.bsha256);
    EXPECT_FALSE(base_info.bsizeondisk);
    EXPECT_FALSE(base_info.bcreatetime);
    EXPECT_FALSE(base_info.bmodifytime);
}

//test class CStaticFileManager
TEST_F(TraceStaticPETest, CStaticFileManager_Class_Test)
{
    std::unique_ptr<CJsonOptions> options = std::make_unique<CJsonOptions>("CFGRES", IDR_JSONPE_CFG);
    ASSERT_TRUE(options != nullptr);
    ASSERT_TRUE(options->Parse());
    CStaticFileManager manager(std::move(options));
    
    for (auto& file : m_file_list) {
        manager.Scan(file, "..\\outputs");
    }
    ASSERT_EQ(manager.GetSuccessScanCount(), 10);
}
#endif

#ifdef _BENCHMARK_TEST

#endif
