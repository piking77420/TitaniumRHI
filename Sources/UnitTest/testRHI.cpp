#include <iostream>

#include <gtest/gtest.h>
#include <Titanium/RHI.hpp>
#include <iostream>

static constexpr std::string_view AnsiReset = "\x1b[0m";
static constexpr std::string_view AnsiBlack = "\x1b[30m";
static constexpr std::string_view AnsiRed = "\x1b[31m";
static constexpr std::string_view AnsiGreen = "\x1b[32m";
static constexpr std::string_view AnsiYellow = "\x1b[33m";
static constexpr std::string_view AnsiOrange = "\x1b[38;2;255;165;0m";
static constexpr std::string_view AnsiBlue = "\x1b[34m";
static constexpr std::string_view AnsiMagenta = "\x1b[35m";
static constexpr std::string_view AnsiCyan = "\x1b[36m";
static constexpr std::string_view AnsiWhite = "\x1b[37m";
static constexpr std::string_view AnsiGrey = "\x1b[90m";
static constexpr std::string_view AnsiBrightRed = "\x1b[91m";
static constexpr std::string_view AnsiBrightGreen = "\x1b[92m";
static constexpr std::string_view AnsiBrightYellow = "\x1b[93m";
static constexpr std::string_view AnsiBrightBlue = "\x1b[94m";
static constexpr std::string_view AnsiBrightMagenta = "\x1b[95m";
static constexpr std::string_view AnsiBrightCyan = "\x1b[96m";
static constexpr std::string_view AnsiBrightWhite = "\x1b[97m";
bool error = false;

void debugCallBack(const std::wstring& message, TiRHI::RhiMessageLocation location, TiRHI::RhiApi api,
                   TiRHI::RhiMessageSeverity severity)
{
    auto getColor = [&severity]() -> std::string_view
    {
        switch (severity)
        {
        case TiRHI::RhiMessageSeverity::Verbose:
            return AnsiReset;
        case TiRHI::RhiMessageSeverity::Info:
            return AnsiGrey;
        case TiRHI::RhiMessageSeverity::Warning:
            return AnsiOrange;
        case TiRHI::RhiMessageSeverity::Error:
            return AnsiRed;
        case TiRHI::RhiMessageSeverity::Fatal:
            return AnsiMagenta;
        }
    };

    std::cout << "[RHI]" << '[' << TiRHI::toString(api) << ']' << getColor() << '[' << TiRHI::toString(severity) << ']'
              << AnsiReset << '[' << TiRHI::toString(location) << "] ";

    if (severity == TiRHI::RhiMessageSeverity::Fatal || severity == TiRHI::RhiMessageSeverity::Error)
        error = true;
}

TEST(DummyTest, Test)
{
    TiRHI::RhiCreate create{.logCallback = debugCallBack};

    TiRHI::RHI device(create);
    EXPECT_TRUE(error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
