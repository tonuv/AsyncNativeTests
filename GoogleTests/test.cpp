#include "pch.h"

#include <winrt/Windows.Foundation.h>
#include <chrono>
#include <roerrorapi.h>
#include <comdef.h>

using namespace std::chrono;
using namespace std::chrono_literals;


void FailWithError(winrt::hresult error)
{
	winrt::com_ptr<IRestrictedErrorInfo> errorInfo;
	char szErrorMessage[256];
	HRESULT hr = GetRestrictedErrorInfo(errorInfo.put());
	if (hr == S_OK) {
		_bstr_t errorDescription;
		_bstr_t restrictedDescription;
		_bstr_t capabilitySid;
		HRESULT hrError{ 0 };

		errorInfo->GetErrorDetails(errorDescription.GetAddress(), &hrError, restrictedDescription.GetAddress(), capabilitySid.GetAddress());
		sprintf_s(szErrorMessage, "%s%s\nHRESULT:0x%08X", (char*)errorDescription, (char*)restrictedDescription, (unsigned)hrError);
	}
	else
	{
		sprintf_s(szErrorMessage, "Test throw and exception %08X", (unsigned)error);
	}
	GTEST_FATAL_FAILURE_(szErrorMessage);
}

template<typename Async>
void RunTestAsync(Async const& async, winrt::Windows::Foundation::TimeSpan timeout = {30s})
{
	using namespace winrt;
	using namespace std::chrono;

	auto milliseconds = duration_cast<duration<uint32_t, std::milli>, winrt::Windows::Foundation::TimeSpan::rep, winrt::Windows::Foundation::TimeSpan::period>(timeout);

	auto status = winrt::impl::wait_for_completed(async, (uint32_t) (timeout.count() / 10000L));

	if (status == winrt::Windows::Foundation::AsyncStatus::Error)
	{
		FailWithError(async.ErrorCode());
	}
	else if (status == winrt::Windows::Foundation::AsyncStatus::Started)
	{
		GTEST_FATAL_FAILURE_("Test timed out");
	}
	else
	{
		GTEST_SUCCEED();
	}

}

winrt::Windows::Foundation::IAsyncAction WasteShortTimeAsync()
{
	co_await 500ms;
}
winrt::Windows::Foundation::IAsyncAction WasteLongTimeAsync()
{
	co_await 5s;
}
winrt::Windows::Foundation::IAsyncAction WasteAndThrowAsync()
{
	co_await 1s;
	throw winrt::hresult_not_implemented(L"Exception more detailed description");
}


TEST(NativeAsync, TestThatWillPass) 
{
	RunTestAsync(WasteShortTimeAsync(),1s);
}

TEST(NativeAsync, TestThatWillTimeOut)
{
	RunTestAsync(WasteLongTimeAsync(), 1s);
}

TEST(NativeAsync, TestThatWillFail)
{
	RunTestAsync(WasteAndThrowAsync());
}