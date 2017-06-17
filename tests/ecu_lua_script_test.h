/**
 * @file ecu_lua_script_test.h
 *
 */

#ifndef ECU_LUA_SCRIPT_TEST_H
#define ECU_LUA_SCRIPT_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class EcuLuaScriptTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(EcuLuaScriptTest);

    CPPUNIT_TEST(testEcuLuaScript);
    CPPUNIT_TEST(testGetRequestId);
    CPPUNIT_TEST(testGetResponseId);
    CPPUNIT_TEST(testGetDataByIdentifier);
    CPPUNIT_TEST(testGetSeed);
    CPPUNIT_TEST(testLiteralHexStrToBytes);

    CPPUNIT_TEST_SUITE_END();

public:
    EcuLuaScriptTest();
    virtual ~EcuLuaScriptTest();
    void setUp();
    void tearDown();

private:
    void testEcuLuaScript();
    void testGetRequestId();
    void testGetResponseId();
    void testGetDataByIdentifier();
    void testGetSeed();
    void testLiteralHexStrToBytes();

};

#endif /* ECU_LUA_SCRIPT_TEST_H */
