#include "libdnf/hy-subject.h"
#include "libdnf/nevra.hpp"
#include "libdnf/transaction/RPMItem.hpp"
#include "libdnf/transaction/Transaction.hpp"
#include "libdnf/transaction/private/Transaction.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "../backports.hpp"

#include "TransactionTest.hpp"

using namespace libdnf;

CPPUNIT_TEST_SUITE_REGISTRATION(TransactionTest);

static RPMItemPtr
nevraToRPMItem(SQLite3Ptr conn, std::string nevra)
{
    libdnf::Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), HY_FORM_NEVRA)) {
        return nullptr;
    }
    if (nevraObject.getEpoch() < 0) {
        nevraObject.setEpoch(0);
    }

    auto rpm = std::make_shared< RPMItem >(conn);
    rpm->setName(nevraObject.getName());
    rpm->setEpoch(nevraObject.getEpoch());
    rpm->setVersion(nevraObject.getVersion());
    rpm->setRelease(nevraObject.getRelease());
    rpm->setArch(nevraObject.getArch());
    return rpm;
}

void
TransactionTest::setUp()
{
    conn = std::make_shared< SQLite3 >(":memory:");
    Transformer::createDatabase(conn);
}

void
TransactionTest::tearDown()
{
}

void
TransactionTest::testInsert()
{
    libdnf::swdb_private::Transaction trans(conn);
    trans.setDtBegin(1);
    trans.setDtEnd(2);
    trans.setRpmdbVersionBegin("begin - TransactionTest::testInsert");
    trans.setRpmdbVersionEnd("end - TransactionTest::testInsert");
    trans.setReleasever("26");
    trans.setUserId(1000);
    trans.setCmdline("dnf install foo");
    trans.setState(TransactionState::DONE);

    trans.addSoftwarePerformedWith(nevraToRPMItem(conn, "rpm-4.14.2-1.fc29.x86_64"));
    trans.addSoftwarePerformedWith(nevraToRPMItem(conn, "dnf-3.5.1-1.fc29.noarch"));
    // test adding a duplicate; only a single occurrence of the rpm is expected
    trans.addSoftwarePerformedWith(nevraToRPMItem(conn, "rpm-4.14.2-1.fc29.x86_64"));

    trans.begin();

    // getSoftwarePerformedWith returns results directly from the db
    // that's why it has to be called after begin(), where the records are saved
    CPPUNIT_ASSERT(trans.getSoftwarePerformedWith().size() == 2);

    // 2nd begin must throw an exception
    CPPUNIT_ASSERT_THROW(trans.begin(), std::runtime_error);

    // load the saved transaction from database and compare values
    libdnf::Transaction trans2(conn, trans.getId());
    CPPUNIT_ASSERT(trans2.getId() == trans.getId());
    CPPUNIT_ASSERT(trans2.getDtBegin() == trans.getDtBegin());
    CPPUNIT_ASSERT(trans2.getDtEnd() == trans.getDtEnd());
    CPPUNIT_ASSERT(trans2.getRpmdbVersionBegin() == trans.getRpmdbVersionBegin());
    CPPUNIT_ASSERT(trans2.getRpmdbVersionEnd() == trans.getRpmdbVersionEnd());
    CPPUNIT_ASSERT(trans2.getReleasever() == trans.getReleasever());
    CPPUNIT_ASSERT(trans2.getUserId() == trans.getUserId());
    CPPUNIT_ASSERT(trans2.getCmdline() == trans.getCmdline());
    CPPUNIT_ASSERT(trans2.getState() == trans.getState());
    CPPUNIT_ASSERT(trans2.getSoftwarePerformedWith().size() == 2);
}

void
TransactionTest::testInsertWithSpecifiedId()
{
    // it is not allowed to save a transaction with arbitrary ID
    libdnf::swdb_private::Transaction trans(conn);
    trans.setId(INT64_MAX);
    CPPUNIT_ASSERT_THROW(trans.begin(), std::runtime_error);
}

void
TransactionTest::testUpdate()
{
    libdnf::swdb_private::Transaction trans(conn);
    trans.setDtBegin(1);
    trans.setDtEnd(2);
    trans.setRpmdbVersionBegin("begin - TransactionTest::testInsert");
    trans.setRpmdbVersionEnd("end - TransactionTest::testInsert");
    trans.setReleasever("26");
    trans.setUserId(1000);
    trans.setCmdline("dnf install foo");
    trans.setState(TransactionState::ERROR);
    trans.begin();
    trans.finish(TransactionState::DONE);

    libdnf::Transaction trans2(conn, trans.getId());
    CPPUNIT_ASSERT(trans2.getId() == trans.getId());
    CPPUNIT_ASSERT(trans2.getDtBegin() == trans.getDtBegin());
    CPPUNIT_ASSERT(trans2.getDtEnd() == trans.getDtEnd());
    CPPUNIT_ASSERT(trans2.getRpmdbVersionBegin() == trans.getRpmdbVersionBegin());
    CPPUNIT_ASSERT(trans2.getRpmdbVersionEnd() == trans.getRpmdbVersionEnd());
    CPPUNIT_ASSERT(trans2.getReleasever() == trans.getReleasever());
    CPPUNIT_ASSERT(trans2.getUserId() == trans.getUserId());
    CPPUNIT_ASSERT(trans2.getCmdline() == trans.getCmdline());
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans2.getState());
}

void
TransactionTest::testComparison()
{
    // test operator ==, > and <
    libdnf::swdb_private::Transaction first(conn);
    libdnf::swdb_private::Transaction second(conn);

    // id comparison test
    first.setId(1);
    second.setId(2);
    CPPUNIT_ASSERT(first > second);
    CPPUNIT_ASSERT(second < first);

    // begin timestamp comparison test
    second.setId(1);
    first.setDtBegin(1);
    second.setDtBegin(2);
    CPPUNIT_ASSERT(first > second);
    CPPUNIT_ASSERT(second < first);

    // rpmdb comparison test
    second.setDtBegin(1);
    first.setRpmdbVersionBegin("0");
    second.setRpmdbVersionBegin("1");
    CPPUNIT_ASSERT(first > second);
    CPPUNIT_ASSERT(second < first);

    // equality
    second.setRpmdbVersionBegin("0");
    CPPUNIT_ASSERT(first == second);
}
