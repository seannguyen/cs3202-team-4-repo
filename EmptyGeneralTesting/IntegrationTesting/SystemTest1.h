#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "../AutoTester/source/AbstractWrapper.h"
#include "Parser.h"
#include "QueryProcessor.h"

/* SYSTEM TESTING FOR ITERATION 0 AND 1 */
class SystemTest1 : public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE( SystemTest1 ); 
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
private:
	void test1();
};