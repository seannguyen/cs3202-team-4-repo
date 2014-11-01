#pragma once

#include "ResultManager.h"
#include <cppunit/extensions/HelperMacros.h>

class ResultManagerTest : public  CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE( ResultManagerTest );
	CPPUNIT_TEST( testInsertTable );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void testInsertTable();
private:
	//methods to populate data
	enum Case {Case1, Case2, Case3};
	ResultTable * populateData(Case caseNum);
};