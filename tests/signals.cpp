#include <boost/test/unit_test.hpp>
#include <utils/signals.hpp>

BOOST_AUTO_TEST_SUITE(signals)

BOOST_AUTO_TEST_CASE(doc_test)
{
    class Controller
    {
    public:
        utils::signal<void(int x, int y)> on_mouse_move;
        utils::signal<void(int x, int y)> on_touch_move;

        void move_touch_to(int x, int y) const
        {
            on_touch_move.emit(x, y);
        }
    };

    int a = 0, b = 0, c = 0, d = 0;

    Controller controller;

    BOOST_CHECK(controller.on_touch_move.empty());
    BOOST_CHECK(controller.on_mouse_move.empty());

    controller.on_touch_move += [&](int x, int y)
    {
        a = x;
        b = y;
    };

    BOOST_CHECK(!controller.on_touch_move.empty());
    BOOST_CHECK(controller.on_mouse_move.empty());

    controller.on_mouse_move += [&](int x, int y)
    {
        c = x*3;
        d = y*3;
    };

    BOOST_CHECK(!controller.on_touch_move.empty());
    BOOST_CHECK(!controller.on_mouse_move.empty());

    controller.on_touch_move += controller.on_mouse_move;
    controller.move_touch_to(1, 2);

    BOOST_CHECK_EQUAL(a, 1);
    BOOST_CHECK_EQUAL(b, 2);
    BOOST_CHECK_EQUAL(c, 3);
    BOOST_CHECK_EQUAL(d, 6);
}

BOOST_AUTO_TEST_CASE(slot_connection_ctor_and_const_test)
{
    utils::slot_connection conn;
    utils::slot_connection conn2 (std::move(conn));
    utils::slot_connection conn3;
    conn3 = std::move(conn2);
    const utils::slot_connection conn4;
    BOOST_CHECK(!conn4.is_connected());
    BOOST_CHECK(conn4.is_suspended());
}

BOOST_AUTO_TEST_SUITE_END()

