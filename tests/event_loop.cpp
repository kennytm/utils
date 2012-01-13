#include <chrono>
#include <numeric>
#include <csignal>
#include <boost/test/unit_test.hpp>
#include <utils/event_loop.hpp>

// Ref: http://stackoverflow.com/a/335972/224671
struct null_output_iterator : std::iterator<std::output_iterator_tag, null_output_iterator>
{
    template <typename T>
    void operator=(const T&) noexcept {}
    null_output_iterator& operator++() noexcept { return *this; }
    null_output_iterator operator++(int) noexcept { return *this; }
    null_output_iterator& operator*() noexcept { return *this; }
};

BOOST_AUTO_TEST_SUITE(signals)

BOOST_AUTO_TEST_CASE(imm_delay)
{
    // This test is to check if the 'delay' function is properly called and
    // sequenced.

    utils::event_loop loop;

    int p, q, r;
    p = q = r = 0;

    loop.delay([&]
    {
        ++ p;
        q = p;
    });

    ++ p;
    r = p;

    loop.run();

    BOOST_CHECK_EQUAL(p, 2);
    BOOST_CHECK_EQUAL(r, 1);
    BOOST_CHECK_EQUAL(q, 2);
}

BOOST_AUTO_TEST_CASE(imm_delay_keep)
{
    // This test is to check if the 'keep' argument works.

    utils::event_loop loop;

    int p, q, r;
    p = q = r = 0;

    loop.delay([&](bool& keep, utils::event_loop& loop, utils::event_handle)
    {
        ++ p;
        q = p;
        keep = (p < 10);
    });

    ++ p;
    r = p;

    loop.run();

    BOOST_CHECK_EQUAL(p, 10);
    BOOST_CHECK_EQUAL(r, 1);
    BOOST_CHECK_EQUAL(q, 10);
}

BOOST_AUTO_TEST_CASE(imm_repeat)
{
    // This test is to check if the 'repeat' function works and can be properly
    // cancelled.

    utils::event_loop loop;

    int p, q, r;
    p = q = r = 0;

    loop.repeat([&](utils::event_loop& loop, utils::event_handle)
    {
        static int counter = 0;
        ++ counter;
        if (counter >= 11)
            loop.stop();
    });

    loop.repeat([&](utils::event_loop& loop, utils::event_handle handle)
    {
        ++ p;
        q = p;
        if (p >= 10)
            loop.cancel(handle);
    });

    ++ p;
    r = p;

    loop.run();

    BOOST_CHECK_EQUAL(p, 10);
    BOOST_CHECK_EQUAL(r, 1);
    BOOST_CHECK_EQUAL(q, 10);
}

BOOST_AUTO_TEST_CASE(cancel_before_start)
{
    utils::event_loop loop;

    auto handle = loop.repeat([]
    {
        BOOST_FAIL("Event should have been cancelled.");
    });

    loop.cancel(handle);
    loop.run();
}

BOOST_AUTO_TEST_CASE(timed_delay)
{
    utils::event_loop loop;

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time;

    loop.delay(std::chrono::milliseconds(400), [&]
    {
        end_time = std::chrono::steady_clock::now();
    });

    loop.run();

    auto delta_time = end_time - start_time;
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
    BOOST_CHECK_GE(millisecs, 395);
    BOOST_CHECK_LE(millisecs, 405);
}

BOOST_AUTO_TEST_CASE(timed_delay_with_keep)
{
    utils::event_loop loop;

    typedef std::chrono::steady_clock::time_point time;

    std::vector<time> times;
    times.push_back(std::chrono::steady_clock::now());

    loop.delay(std::chrono::milliseconds(200), [&](bool& keep, utils::event_loop& loop, utils::event_handle)
    {
        times.push_back(std::chrono::steady_clock::now());
        keep = times.size() <= 4;
    });

    loop.run();

    BOOST_CHECK_EQUAL(times.size(), 5);

    std::adjacent_difference(
        times.begin(), times.end(), null_output_iterator(),
        [](time after, time before)
        {
            auto diff = after - before;
            auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
            BOOST_CHECK_GE(millisecs, 195);
            BOOST_CHECK_LE(millisecs, 205);
            return 0;
        }
    );
}

BOOST_AUTO_TEST_CASE(timed_repeat)
{
    utils::event_loop loop;

    typedef std::chrono::steady_clock::time_point time;

    std::vector<time> times;
    times.push_back(std::chrono::steady_clock::now());

    loop.repeat(std::chrono::milliseconds(200), [&](utils::event_loop& loop, utils::event_handle handle)
    {
        times.push_back(std::chrono::steady_clock::now());
        if (times.size() > 4)
            loop.cancel(handle);
    });

    loop.run();

    BOOST_CHECK_EQUAL(times.size(), 5);

    std::adjacent_difference(
        times.begin(), times.end(), null_output_iterator(),
        [](time after, time before)
        {
            auto diff = after - before;
            auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
            BOOST_CHECK_GE(millisecs, 195);
            BOOST_CHECK_LE(millisecs, 205);
            return 0;
        }
    );
}

BOOST_AUTO_TEST_CASE(pipe_io)
{
    utils::event_loop loop;

    int fds[2];
    pipe(fds);

    auto start_time = std::chrono::steady_clock::now();

    loop.repeat(std::chrono::milliseconds(200), [=](utils::event_loop& loop, utils::event_handle handle)
    {
        static int counter = 0;
        ++ counter;
        write(fds[1], &counter, sizeof(counter));
        if (counter >= 4)
        {
            close(fds[1]);
            loop.cancel(handle);
        }
    });

    int counters_received = 0;

    loop.listen(fds[0], [=, &counters_received](int fd, utils::event_loop& loop, utils::event_handle handle)
    {
        int counter;
        ssize_t res = read(fd, &counter, sizeof(counter));
        if (res <= 0)
        {
            close(fd);
            loop.cancel(handle);
        }
        else
        {
            auto time_diff = std::chrono::steady_clock::now() - start_time;
            auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();
            BOOST_TEST_CHECKPOINT("counter = " << counter);
            BOOST_CHECK_GE(millisecs, 200 * counter - 8);
            BOOST_CHECK_LE(millisecs, 200 * counter + 8);

            counters_received += counter;
        }
    });

    loop.run();
    BOOST_CHECK_EQUAL(counters_received, 1+2+3+4);
}

BOOST_AUTO_TEST_CASE(catch_signal)
{
    utils::event_loop loop;
    int raised_signal = 0;

    loop.signal(SIGINT, [&](int sig, utils::event_loop& loop, utils::event_handle handle)
    {
        raised_signal = sig;
        loop.cancel(handle);
    });

    loop.delay([]{ raise(SIGINT); });

    loop.run();

    BOOST_CHECK_EQUAL(raised_signal, SIGINT);
}

BOOST_AUTO_TEST_CASE(unique_event_owner)
{
    utils::event_loop loop;

    std::vector<utils::unique_event> events;
    int i = 0;

    auto handle = loop.repeat([&]
    {
        ++ i;
        events.pop_back();
    });
    events.emplace_back(loop, std::move(handle));

    loop.run();

    BOOST_CHECK_EQUAL(i, 1);
}

BOOST_AUTO_TEST_SUITE_END()

