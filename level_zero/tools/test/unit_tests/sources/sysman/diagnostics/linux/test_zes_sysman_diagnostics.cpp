/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/tools/test/unit_tests/sources/sysman/diagnostics/linux/mock_zes_sysman_diagnostics.h"

using ::testing::_;
namespace L0 {
namespace ult {

class ZesDiagnosticsFixture : public SysmanDeviceFixture {

  protected:
    zes_diag_handle_t hSysmanDiagnostics = {};
    std::unique_ptr<Mock<DiagnosticsInterface>> pMockDiagInterface;
    FirmwareUtil *pFwUtilInterfaceOld = nullptr;

    void SetUp() override {
        SysmanDeviceFixture::SetUp();

        pFwUtilInterfaceOld = pLinuxSysmanImp->pFwUtilInterface;
        pMockDiagInterface = std::make_unique<NiceMock<Mock<DiagnosticsInterface>>>();
        pLinuxSysmanImp->pFwUtilInterface = pMockDiagInterface.get();
        ON_CALL(*pMockDiagInterface.get(), fwDeviceInit())
            .WillByDefault(::testing::Invoke(pMockDiagInterface.get(), &Mock<DiagnosticsInterface>::mockFwDeviceInit));
        ON_CALL(*pMockDiagInterface.get(), getFirstDevice(_))
            .WillByDefault(::testing::Invoke(pMockDiagInterface.get(), &Mock<DiagnosticsInterface>::mockGetFirstDevice));
        ON_CALL(*pMockDiagInterface.get(), fwSupportedDiagTests(_))
            .WillByDefault(::testing::Invoke(pMockDiagInterface.get(), &Mock<DiagnosticsInterface>::mockFwSupportedDiagTests));
        for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
            delete handle;
        }
        pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
        pSysmanDeviceImp->pDiagnosticsHandleContext->init();
    }
    void TearDown() override {
        SysmanDeviceFixture::TearDown();
        pLinuxSysmanImp->pFwUtilInterface = pFwUtilInterfaceOld;
    }

    std::vector<zes_diag_handle_t> get_diagnostics_handles(uint32_t &count) {
        std::vector<zes_diag_handle_t> handles(count, nullptr);
        EXPECT_EQ(zesDeviceEnumDiagnosticTestSuites(device->toHandle(), &count, handles.data()), ZE_RESULT_SUCCESS);
        return handles;
    }
};

TEST_F(ZesDiagnosticsFixture, GivenComponentCountZeroWhenCallingzesDeviceEnumDiagnosticTestSuitesThenZeroCountIsReturnedAndVerifyzesDeviceEnumDiagnosticTestSuitesCallSucceeds) {
    std::vector<zes_diag_handle_t> diagnosticsHandle{};
    uint32_t count = 0;

    ze_result_t result = zesDeviceEnumDiagnosticTestSuites(device->toHandle(), &count, nullptr);

    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_EQ(count, 0u);

    uint32_t testCount = count + 1;

    result = zesDeviceEnumDiagnosticTestSuites(device->toHandle(), &testCount, nullptr);

    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_EQ(testCount, count);

    diagnosticsHandle.resize(count);
    result = zesDeviceEnumDiagnosticTestSuites(device->toHandle(), &count, diagnosticsHandle.data());

    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_EQ(count, 0u);

    DiagnosticsImp *ptestDiagnosticsImp = new DiagnosticsImp(pSysmanDeviceImp->pDiagnosticsHandleContext->pOsSysman, mockSupportedDiagTypes[0]);
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.push_back(ptestDiagnosticsImp);
    result = zesDeviceEnumDiagnosticTestSuites(device->toHandle(), &count, nullptr);

    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_EQ(count, 1u);

    testCount = count;

    diagnosticsHandle.resize(testCount);
    result = zesDeviceEnumDiagnosticTestSuites(device->toHandle(), &testCount, diagnosticsHandle.data());

    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_NE(nullptr, diagnosticsHandle.data());
    EXPECT_EQ(testCount, 1u);

    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.pop_back();
    delete ptestDiagnosticsImp;
}

TEST_F(ZesDiagnosticsFixture, GivenFailedFirmwareInitializationWhenInitializingDiagnosticsContextThenexpectNoHandles) {
    for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
        delete handle;
    }
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
    ON_CALL(*pMockDiagInterface.get(), fwDeviceInit())
        .WillByDefault(::testing::Invoke(pMockDiagInterface.get(), &Mock<DiagnosticsInterface>::mockFwDeviceInitFail));

    pSysmanDeviceImp->pDiagnosticsHandleContext->init();

    EXPECT_EQ(0u, pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.size());
}

TEST_F(ZesDiagnosticsFixture, GivenSupportedTestsWhenInitializingDiagnosticsContextThenexpectHandles) {
    for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
        delete handle;
    }
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
    pSysmanDeviceImp->pDiagnosticsHandleContext->supportedDiagTests.push_back(mockSupportedDiagTypes[0]);
    ON_CALL(*pMockDiagInterface.get(), fwDeviceInit())
        .WillByDefault(::testing::Invoke(pMockDiagInterface.get(), &Mock<DiagnosticsInterface>::mockFwDeviceInitFail));

    pSysmanDeviceImp->pDiagnosticsHandleContext->init();

    EXPECT_EQ(1u, pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.size());
}

TEST_F(ZesDiagnosticsFixture, GivenFirmwareInitializationFailureThenCreateHandleMustFail) {
    for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
        delete handle;
    }
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
    ON_CALL(*pMockDiagInterface.get(), fwDeviceInit())
        .WillByDefault(::testing::Invoke(pMockDiagInterface.get(), &Mock<DiagnosticsInterface>::mockFwDeviceInitFail));
    pSysmanDeviceImp->pDiagnosticsHandleContext->init();
    EXPECT_EQ(0u, pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.size());
}

TEST_F(ZesDiagnosticsFixture, GivenValidDiagnosticsHandleWhenGettingDiagnosticsPropertiesThenCallSucceeds) {
    for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
        delete handle;
    }
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
    DiagnosticsImp *ptestDiagnosticsImp = new DiagnosticsImp(pSysmanDeviceImp->pDiagnosticsHandleContext->pOsSysman, mockSupportedDiagTypes[0]);
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.push_back(ptestDiagnosticsImp);

    auto handle = pSysmanDeviceImp->pDiagnosticsHandleContext->handleList[0]->toHandle();
    zes_diag_properties_t properties = {};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDiagnosticsGetProperties(handle, &properties));
}

TEST_F(ZesDiagnosticsFixture, GivenValidDiagnosticsHandleWhenGettingDiagnosticsTestThenCallSucceeds) {
    for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
        delete handle;
    }
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
    DiagnosticsImp *ptestDiagnosticsImp = new DiagnosticsImp(pSysmanDeviceImp->pDiagnosticsHandleContext->pOsSysman, mockSupportedDiagTypes[0]);
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.push_back(ptestDiagnosticsImp);

    auto handle = pSysmanDeviceImp->pDiagnosticsHandleContext->handleList[0]->toHandle();
    zes_diag_test_t tests = {};
    uint32_t count = 0;
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, zesDiagnosticsGetTests(handle, &count, &tests));

    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.pop_back();
    delete ptestDiagnosticsImp;
}

TEST_F(ZesDiagnosticsFixture, GivenValidDiagnosticsHandleWhenRunningDiagnosticsTestThenCallSucceeds) {
    for (const auto &handle : pSysmanDeviceImp->pDiagnosticsHandleContext->handleList) {
        delete handle;
    }
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.clear();
    DiagnosticsImp *ptestDiagnosticsImp = new DiagnosticsImp(pSysmanDeviceImp->pDiagnosticsHandleContext->pOsSysman, mockSupportedDiagTypes[0]);
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.push_back(ptestDiagnosticsImp);

    auto handle = pSysmanDeviceImp->pDiagnosticsHandleContext->handleList[0]->toHandle();
    zes_diag_result_t results = ZES_DIAG_RESULT_FORCE_UINT32;
    uint32_t start = 0, end = 0;
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, zesDiagnosticsRunTests(handle, start, end, &results));
    pSysmanDeviceImp->pDiagnosticsHandleContext->handleList.pop_back();
    delete ptestDiagnosticsImp;
}

} // namespace ult
} // namespace L0
