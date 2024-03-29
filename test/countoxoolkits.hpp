/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <cppunit/extensions/HelperMacros.h>

#include <Poco/DirectoryIterator.h>
#include <Poco/FileStream.h>
#include <Poco/StreamCopier.h>

#include <Common.hpp>
#include "Util.hpp"
#include "test.hpp"
#include "helpers.hpp"

static int countOxoolKitProcesses(const int expected,
                                 std::chrono::milliseconds timeoutMs
                                 = std::chrono::milliseconds(COMMAND_TIMEOUT_MS * 8))
{
    const auto testname = "countOxoolKitProcesses ";
    TST_LOG_BEGIN("Waiting until oxoolkit processes are exactly " << expected << ". Oxoolkits: ");

    Util::Stopwatch stopwatch;

    // This does not need to depend on any constant from Common.hpp.
    // The shorter the better (the quicker the test runs).
    constexpr int sleepMs = 10;

    // This has to cause waiting for at least COMMAND_TIMEOUT_MS. Tolerate more for safety.
    const std::size_t repeat = (timeoutMs.count() / sleepMs);
    int count = getOxoolKitProcessCount();
    for (std::size_t i = 0; i < repeat; ++i)
    {
        TST_LOG_APPEND(count << ' ');
        if (count == expected)
        {
            break;
        }

        // Give polls in the oxool processes time to time out etc
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));

        const int newCount = getOxoolKitProcessCount();
        if (count != newCount)
        {
            // Allow more time until the number settles.
            i = 0;
            count = newCount;
        }
    }

    TST_LOG_END;
    if (expected != count)
    {
        TST_LOG_BEGIN("Found " << count << " LoKit processes but was expecting " << expected << ": [");
        for (pid_t i : getKitPids())
        {
            TST_LOG_APPEND(i << ' ');
        }
        TST_LOG_APPEND(']');
        TST_LOG_END;

    }

    std::set<pid_t> pids = getKitPids();
    std::ostringstream oss;
    oss << "Test kit pids are [";
    for (pid_t i : pids)
        oss << i << ' ';
    oss << "] after waiting for " << stopwatch.elapsed();
    TST_LOG(oss.str());

    return count;
}

// FIXME: we probably should make this extern
// and reuse it. As it stands now, it is per
// translation unit, which isn't desirable if
// (in the non-ideal event that) it's not 1,
// it will cause testNoExtraOxoolKitsLeft to
// wait unnecessarily and fail.
static int InitialOxoolKitCount = 1;
static std::chrono::steady_clock::time_point TestStartTime;

static void testCountHowManyOxoolkits()
{
    const char testname[] = "countHowManyOxoolkits ";
    TestStartTime = std::chrono::steady_clock::now();

    InitialOxoolKitCount = countOxoolKitProcesses(InitialOxoolKitCount);
    TST_LOG("Initial oxoolkit count is " << InitialOxoolKitCount);
    LOK_ASSERT(InitialOxoolKitCount > 0);

    TestStartTime = std::chrono::steady_clock::now();
}

static void testNoExtraOxoolKitsLeft()
{
    const char testname[] = "noExtraOxoolKitsLeft ";
    const int countNow = countOxoolKitProcesses(InitialOxoolKitCount);
    LOK_ASSERT_EQUAL(InitialOxoolKitCount, countNow);

    const auto duration = (std::chrono::steady_clock::now() - TestStartTime);
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    TST_LOG(" (" << durationMs << ')');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
