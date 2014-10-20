#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "../AutoTester/source/AbstractWrapper.h"
#include "Parser.h"
#include "DesignExtractor.h"
#include "QueryProcessor.h"
#include "PKB.h"

/* SYSTEM TESTING FOR ITERATION 0 AND 1 */
class SystemTest2 : public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE( SystemTest2 ); 
	CPPUNIT_TEST( processUses );
	CPPUNIT_TEST( processModify );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
private:
	void processUses();
	void processModify();
};