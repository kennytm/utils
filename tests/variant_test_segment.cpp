#include <vector>
#include <functional>
#include <boost/test/unit_test.hpp>
#include <utils/variant.hpp>

struct Pt { double x, y; };

struct MSeg { Pt e; };
struct LSeg { Pt s, e; };
struct QSeg { Pt s, c, e; };
struct CSeg { Pt s, c, d, e; };
struct ASeg { Pt c, r, a; double t; };

typedef utils::variant<MSeg, LSeg, QSeg, CSeg, ASeg, std::function<void()>> Seg;

BOOST_AUTO_TEST_SUITE(variant_test_segment)

BOOST_AUTO_TEST_CASE(push_back_test)
{
    Seg s = LSeg{{1, 2}, {3, 4}};
    const Seg& sclref = s;
    Seg& slref = s;
    const Seg&& scrref = std::move(s);
    Seg&& srref = std::move(s);

    std::vector<Seg> segs;
    segs.push_back(sclref);
    segs.push_back(slref);
    segs.push_back(scrref);
    segs.push_back(srref);

    BOOST_CHECK_EQUAL(segs.size(), 4);
    for (const auto& sf : segs)
    {
        LSeg lseg = utils::get<LSeg>(sf);
        BOOST_CHECK_EQUAL(lseg.s.x, 1.0);
        BOOST_CHECK_EQUAL(lseg.s.y, 2.0);
        BOOST_CHECK_EQUAL(lseg.e.x, 3.0);
        BOOST_CHECK_EQUAL(lseg.e.y, 4.0);
    }

    s = sclref;
    s = slref;
    s = scrref;
    s = srref;
}

BOOST_AUTO_TEST_CASE(lvalue_test)
{
    MSeg m {{1, 2}};

    const MSeg& mclref = m;
    MSeg& mlref = m;
    const MSeg&& mcrref = std::move(m);
    MSeg&& mrref = std::move(m);

    Seg s0 = m;
    Seg s1 = mclref;
    Seg s2 = mlref;
    Seg s3 = mcrref;
    Seg s4 = mrref;

    s4 = m;
    s3 = mclref;
    s2 = mlref;
    s1 = mcrref;
    s0 = mrref;
}

BOOST_AUTO_TEST_SUITE_END()

