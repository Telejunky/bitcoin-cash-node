// Copyright (c) 2014 BitPay Inc.
// Copyright (c) 2014-2016 The Bitcoin Core developers
// Copyright (c) 2020 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/mit-license.php.

#include <cassert>
#include <clocale>
#include <cstdint>
#include <limits>
#include <locale>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <univalue.h>

#define BOOST_FIXTURE_TEST_SUITE(a, b)
#define BOOST_AUTO_TEST_CASE(funcName) void funcName()
#define BOOST_AUTO_TEST_SUITE_END()
#define BOOST_CHECK(expr) assert(expr)
#define BOOST_CHECK_EQUAL(v1, v2) assert((v1) == (v2))
#define BOOST_CHECK_THROW(stmt, excMatch) { \
        try { \
            (stmt); \
            assert(0 && "No exception caught"); \
        } catch (excMatch & e) { \
	} catch (...) { \
	    assert(0 && "Wrong exception caught"); \
	} \
    }
#define BOOST_CHECK_NO_THROW(stmt) { \
        try { \
            (stmt); \
	} catch (...) { \
	    assert(0); \
	} \
    }

BOOST_FIXTURE_TEST_SUITE(univalue_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(univalue_constructor)
{
    UniValue v1;
    BOOST_CHECK(v1.isNull());

    UniValue v2(UniValue::VSTR);
    BOOST_CHECK(v2.isStr());

    UniValue v3(UniValue::VSTR, "foo");
    BOOST_CHECK(v3.isStr());
    BOOST_CHECK_EQUAL(v3.getValStr(), "foo");

    UniValue numTest;
    numTest.setNumStr("82");
    BOOST_CHECK(numTest.isNum());
    BOOST_CHECK_EQUAL(numTest.getValStr(), "82");

    uint64_t vu64 = 82;
    UniValue v4(vu64);
    BOOST_CHECK(v4.isNum());
    BOOST_CHECK_EQUAL(v4.getValStr(), "82");

    int64_t vi64 = -82;
    UniValue v5(vi64);
    BOOST_CHECK(v5.isNum());
    BOOST_CHECK_EQUAL(v5.getValStr(), "-82");

    int vi = -688;
    UniValue v6(vi);
    BOOST_CHECK(v6.isNum());
    BOOST_CHECK_EQUAL(v6.getValStr(), "-688");

    double vd = -7.21;
    UniValue v7(vd);
    BOOST_CHECK(v7.isNum());
    BOOST_CHECK_EQUAL(v7.getValStr(), "-7.21");

    std::string vs("yawn");
    UniValue v8(vs);
    BOOST_CHECK(v8.isStr());
    BOOST_CHECK_EQUAL(v8.getValStr(), "yawn");

    const char *vcs = "zappa";
    UniValue v9(vcs);
    BOOST_CHECK(v9.isStr());
    BOOST_CHECK_EQUAL(v9.getValStr(), "zappa");
}

BOOST_AUTO_TEST_CASE(univalue_typecheck)
{
    UniValue v1;
    v1.setNumStr("1");
    BOOST_CHECK(v1.isNum());
    BOOST_CHECK_THROW(v1.get_bool(), std::runtime_error);

    UniValue v2;
    v2.setBool(true);
    BOOST_CHECK_EQUAL(v2.get_bool(), true);
    BOOST_CHECK_THROW(v2.get_int(), std::runtime_error);

    UniValue v3;
    v3.setNumStr("32482348723847471234");
    BOOST_CHECK_THROW(v3.get_int64(), std::runtime_error);
    v3.setNumStr("1000");
    BOOST_CHECK_EQUAL(v3.get_int64(), 1000);

    UniValue v4;
    v4.setNumStr("2147483648");
    BOOST_CHECK_EQUAL(v4.get_int64(), 2147483648);
    BOOST_CHECK_THROW(v4.get_int(), std::runtime_error);
    v4.setNumStr("1000");
    BOOST_CHECK_EQUAL(v4.get_int(), 1000);
    BOOST_CHECK_THROW(v4.get_str(), std::runtime_error);
    BOOST_CHECK_EQUAL(v4.get_real(), 1000);
    BOOST_CHECK_THROW(v4.get_array(), std::runtime_error);
    BOOST_CHECK_THROW(v4.getObjectEntries(), std::runtime_error);
    BOOST_CHECK_THROW(v4.get_obj(), std::runtime_error);

    UniValue v5;
    BOOST_CHECK(v5.read("[true, 10]"));
    BOOST_CHECK_NO_THROW(v5.get_array());
    std::vector<UniValue> vals = v5.getArrayValues();
    BOOST_CHECK_THROW(vals[0].get_int(), std::runtime_error);
    BOOST_CHECK_EQUAL(vals[0].get_bool(), true);

    BOOST_CHECK_EQUAL(vals[1].get_int(), 10);
    BOOST_CHECK_THROW(vals[1].get_bool(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(univalue_set)
{
    UniValue v(UniValue::VSTR, "foo");
    v.setNull();
    BOOST_CHECK(v.isNull());
    BOOST_CHECK_EQUAL(v.getValStr(), "");

    v.setObject();
    BOOST_CHECK(v.isObject());
    BOOST_CHECK_EQUAL(v.size(), 0);
    BOOST_CHECK_EQUAL(v.getType(), UniValue::VOBJ);
    BOOST_CHECK(v.empty());

    v.setArray();
    BOOST_CHECK(v.isArray());
    BOOST_CHECK_EQUAL(v.size(), 0);

    v.setStr("zum");
    BOOST_CHECK(v.isStr());
    BOOST_CHECK_EQUAL(v.getValStr(), "zum");

    v.setFloat(std::numeric_limits<double>::quiet_NaN());
    BOOST_CHECK(v.isStr());
    BOOST_CHECK_EQUAL(v.getValStr(), "zum");

    v.setFloat(std::numeric_limits<double>::signaling_NaN());
    BOOST_CHECK(v.isStr());
    BOOST_CHECK_EQUAL(v.getValStr(), "zum");

    v.setFloat(std::numeric_limits<double>::infinity());
    BOOST_CHECK(v.isStr());
    BOOST_CHECK_EQUAL(v.getValStr(), "zum");

    v.setFloat(-std::numeric_limits<double>::infinity());
    BOOST_CHECK(v.isStr());
    BOOST_CHECK_EQUAL(v.getValStr(), "zum");

    v.setFloat(-1.01);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-1.01");

    v.setNull();
    BOOST_CHECK(v.isNull());
    v.setFloat(-std::numeric_limits<double>::max());
    BOOST_CHECK(v.isNum());

    v.setFloat(-1.79769313486231570e+308);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-1.797693134862316e+308");

    v.setFloat(-100000000000000000. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-3.333333333333333e+16");

    v.setFloat(-10000000000000000. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-3333333333333334");

    v.setFloat(-1000000000000000. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-333333333333333.3");

    v.setFloat(-10. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-3.333333333333333");

    v.setFloat(-1.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-1");

    v.setFloat(-1. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-0.3333333333333333");

    v.setFloat(-1. / 3000.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-0.0003333333333333333");

    v.setFloat(-1. / 30000.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-3.333333333333333e-05");

    v.setFloat(-4.94065645841246544e-324);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-4.940656458412465e-324");

    v.setNull();
    BOOST_CHECK(v.isNull());
    v.setFloat(-std::numeric_limits<double>::min());
    BOOST_CHECK(v.isNum());

    v.setFloat(-1. / std::numeric_limits<double>::infinity());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-0");

    v.setFloat(1. / std::numeric_limits<double>::infinity());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "0");

    v.setNull();
    BOOST_CHECK(v.isNull());
    v.setFloat(std::numeric_limits<double>::min());
    BOOST_CHECK(v.isNum());

    v.setFloat(4.94065645841246544e-324);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "4.940656458412465e-324");

    v.setFloat(1. / 30000.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "3.333333333333333e-05");

    v.setFloat(1. / 3000.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "0.0003333333333333333");

    v.setFloat(1. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "0.3333333333333333");

    v.setFloat(1.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "1");

    v.setFloat(10. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "3.333333333333333");

    v.setFloat(1000000000000000. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "333333333333333.3");

    v.setFloat(10000000000000000. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "3333333333333334");

    v.setFloat(100000000000000000. / 3.);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "3.333333333333333e+16");

    v.setFloat(1.79769313486231570e+308);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "1.797693134862316e+308");

    v.setNull();
    BOOST_CHECK(v.isNull());
    v.setFloat(std::numeric_limits<double>::max());
    BOOST_CHECK(v.isNum());

    v.setInt(1023);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "1023");

    v.setInt(0);
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "0");

    v.setInt(std::numeric_limits<int>::min());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), std::to_string(std::numeric_limits<int>::min()));

    v.setInt(std::numeric_limits<int>::max());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), std::to_string(std::numeric_limits<int>::max()));

    v.setInt(int64_t(-1023));
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-1023");

    v.setInt(int64_t(0));
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "0");

    v.setInt(std::numeric_limits<int64_t>::min());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-9223372036854775808");

    v.setInt(std::numeric_limits<int64_t>::max());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "9223372036854775807");

    v.setInt(uint64_t(1023));
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "1023");

    v.setInt(uint64_t(0));
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "0");

    v.setInt(std::numeric_limits<uint64_t>::max());
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "18446744073709551615");

    v.setNumStr("-688");
    BOOST_CHECK(v.isNum());
    BOOST_CHECK_EQUAL(v.getValStr(), "-688");

    v.setBool(false);
    BOOST_CHECK_EQUAL(v.isBool(), true);
    BOOST_CHECK_EQUAL(v.isTrue(), false);
    BOOST_CHECK_EQUAL(v.isFalse(), true);
    BOOST_CHECK_EQUAL(v.getBool(), false);

    v.setBool(true);
    BOOST_CHECK_EQUAL(v.isBool(), true);
    BOOST_CHECK_EQUAL(v.isTrue(), true);
    BOOST_CHECK_EQUAL(v.isFalse(), false);
    BOOST_CHECK_EQUAL(v.getBool(), true);

    v.setNumStr("zombocom");
    BOOST_CHECK_EQUAL(v.isBool(), true);
    BOOST_CHECK_EQUAL(v.isTrue(), true);
    BOOST_CHECK_EQUAL(v.isFalse(), false);
    BOOST_CHECK_EQUAL(v.getBool(), true);

    v.setNull();
    BOOST_CHECK(v.isNull());
}

BOOST_AUTO_TEST_CASE(univalue_array)
{
    UniValue arr(UniValue::VARR);

    UniValue v((int64_t)1023LL);
    arr.push_back(v);

    std::string vStr("zippy");
    arr.push_back(vStr);

    const char *s = "pippy";
    arr.push_back(s);

    std::vector<UniValue> vec;
    v.setStr("boing");
    vec.push_back(v);

    v.setStr("going");
    vec.push_back(v);

    for (UniValue& thing : vec) {
        arr.push_back(std::move(thing));
    }

    arr.push_back((uint64_t) 400ULL);
    arr.push_back((int64_t) -400LL);
    arr.push_back((int) -401);
    arr.push_back(-40.1);

    BOOST_CHECK_EQUAL(arr.empty(), false);
    BOOST_CHECK_EQUAL(arr.size(), 9);

    BOOST_CHECK_EQUAL(arr[0].getValStr(), "1023");
    BOOST_CHECK_EQUAL(arr[1].getValStr(), "zippy");
    BOOST_CHECK_EQUAL(arr[2].getValStr(), "pippy");
    BOOST_CHECK_EQUAL(arr[3].getValStr(), "boing");
    BOOST_CHECK_EQUAL(arr[4].getValStr(), "going");
    BOOST_CHECK_EQUAL(arr[5].getValStr(), "400");
    BOOST_CHECK_EQUAL(arr[6].getValStr(), "-400");
    BOOST_CHECK_EQUAL(arr[7].getValStr(), "-401");
    BOOST_CHECK_EQUAL(arr[8].getValStr(), "-40.1");

    BOOST_CHECK_EQUAL(arr[999].getValStr(), "");

    arr.setNull();
    BOOST_CHECK(arr.empty());
    BOOST_CHECK_EQUAL(arr.size(), 0);
}

BOOST_AUTO_TEST_CASE(univalue_object)
{
    UniValue obj(UniValue::VOBJ);
    std::string strKey, strVal;
    UniValue v;

    strKey = "age";
    v.setInt(100);
    obj.pushKV(strKey, v);

    strKey = "first";
    strVal = "John";
    obj.pushKV(strKey, strVal);

    strKey = "last";
    const char *cVal = "Smith";
    obj.pushKV(strKey, cVal);

    strKey = "distance";
    obj.pushKV(strKey, (int64_t) 25);

    strKey = "time";
    obj.pushKV(strKey, (uint64_t) 3600);

    strKey = "calories";
    obj.pushKV(strKey, (int) 12);

    strKey = "temperature";
    obj.pushKV(strKey, (double) 90.012);

    strKey = "moon";
    obj.pushKV(strKey, true);

    strKey = "spoon";
    obj.pushKV(strKey, false);

    UniValue obj2(UniValue::VOBJ);
    obj2.pushKV("cat1", 9000);
    obj2.pushKV("cat2", 12345);

    for (auto& pair : obj2.getObjectEntries()) {
        obj.pushKV(std::move(pair.first), std::move(pair.second));
    }

    BOOST_CHECK_EQUAL(obj.empty(), false);
    BOOST_CHECK_EQUAL(obj.size(), 11);

    BOOST_CHECK_EQUAL(obj["age"].getValStr(), "100");
    BOOST_CHECK_EQUAL(obj["first"].getValStr(), "John");
    BOOST_CHECK_EQUAL(obj["last"].getValStr(), "Smith");
    BOOST_CHECK_EQUAL(obj["distance"].getValStr(), "25");
    BOOST_CHECK_EQUAL(obj["time"].getValStr(), "3600");
    BOOST_CHECK_EQUAL(obj["calories"].getValStr(), "12");
    BOOST_CHECK_EQUAL(obj["temperature"].getValStr(), "90.012");
    BOOST_CHECK_EQUAL(obj["moon"].getValStr(), "1");
    BOOST_CHECK_EQUAL(obj["spoon"].getValStr(), "");
    BOOST_CHECK_EQUAL(obj["cat1"].getValStr(), "9000");
    BOOST_CHECK_EQUAL(obj["cat2"].getValStr(), "12345");

    BOOST_CHECK_EQUAL(obj["nyuknyuknyuk"].getValStr(), "");

    BOOST_CHECK_EQUAL(obj.find("age"), &obj["age"]);
    BOOST_CHECK_EQUAL(obj.find("first"), &obj["first"]);
    BOOST_CHECK_EQUAL(obj.find("last"), &obj["last"]);
    BOOST_CHECK_EQUAL(obj.find("distance"), &obj["distance"]);
    BOOST_CHECK_EQUAL(obj.find("time"), &obj["time"]);
    BOOST_CHECK_EQUAL(obj.find("calories"), &obj["calories"]);
    BOOST_CHECK_EQUAL(obj.find("temperature"), &obj["temperature"]);
    BOOST_CHECK_EQUAL(obj.find("moon"), &obj["moon"]);
    BOOST_CHECK_EQUAL(obj.find("spoon"), &obj["spoon"]);
    BOOST_CHECK_EQUAL(obj.find("cat1"), &obj["cat1"]);
    BOOST_CHECK_EQUAL(obj.find("cat2"), &obj["cat2"]);

    BOOST_CHECK_EQUAL(obj.find("nyuknyuknyuk"), nullptr);

    BOOST_CHECK_EQUAL(obj["age"].getType(), UniValue::VNUM);
    BOOST_CHECK_EQUAL(obj["first"].getType(), UniValue::VSTR);
    BOOST_CHECK_EQUAL(obj["last"].getType(), UniValue::VSTR);
    BOOST_CHECK_EQUAL(obj["distance"].getType(), UniValue::VNUM);
    BOOST_CHECK_EQUAL(obj["time"].getType(), UniValue::VNUM);
    BOOST_CHECK_EQUAL(obj["calories"].getType(), UniValue::VNUM);
    BOOST_CHECK_EQUAL(obj["temperature"].getType(), UniValue::VNUM);
    BOOST_CHECK_EQUAL(obj["moon"].getType(), UniValue::VBOOL);
    BOOST_CHECK_EQUAL(obj["spoon"].getType(), UniValue::VBOOL);
    BOOST_CHECK_EQUAL(obj["cat1"].getType(), UniValue::VNUM);
    BOOST_CHECK_EQUAL(obj["cat2"].getType(), UniValue::VNUM);

    BOOST_CHECK_EQUAL(obj["nyuknyuknyuk"].getType(), UniValue::VNULL);

    obj.setNull();
    BOOST_CHECK(obj.empty());
    BOOST_CHECK_EQUAL(obj.size(), 0);
    BOOST_CHECK_EQUAL(obj.getType(), UniValue::VNULL);

    obj.setObject();
    UniValue uv;
    uv.setInt(42);
    obj.pushKV("age", uv);
    BOOST_CHECK_EQUAL(obj.size(), 1);
    BOOST_CHECK_EQUAL(obj["age"].getValStr(), "42");

    uv.setInt(43);
    obj.pushKV("age", uv);
    BOOST_CHECK_EQUAL(obj.size(), 1);
    BOOST_CHECK_EQUAL(obj["age"].getValStr(), "43");

    obj.pushKV("name", "foo bar");
    BOOST_CHECK_EQUAL(obj["name"].getValStr(), "foo bar");

    // test takeArrayValues(), front() / back() as well as operator==
    UniValue arr{UniValue::VNUM}; // this is intentional.
    BOOST_CHECK_THROW(arr.takeArrayValues(), std::runtime_error); // should throw if !array
    BOOST_CHECK_EQUAL(&arr.front(), &NullUniValue); // should return the NullUniValue if !array
    std::vector<UniValue> vals = {{
        "foo", "bar", UniValue::VOBJ, "baz", "bat", false, {}, 1.2, true, 10, -42, -12345678.11234678,
        UniValue{UniValue::VARR}
    }};
    vals[2].pushKV("akey", "this is a value");
    vals.back().setArray(vals); // vals recursively contains a partial copy of vals!
    const auto valsExpected = vals; // save a copy
    arr.setArray(std::move(vals)); // assign to array via move
    BOOST_CHECK(vals.empty()); // vector should be empty after move
    BOOST_CHECK(!arr.empty()); // but our array should not be
    BOOST_CHECK(arr != UniValue{UniValue::VARR}); // check that UniValue::operator== is not a yes-man
    BOOST_CHECK(arr != UniValue{1.234}); // check operator== for differing types
    BOOST_CHECK_EQUAL(arr.front(), valsExpected.front());
    BOOST_CHECK_EQUAL(arr.back(), valsExpected.back());
    BOOST_CHECK(arr.getArrayValues() == valsExpected);
    auto vals2 = arr.takeArrayValues(); // take the values back
    BOOST_CHECK(arr.empty());
    BOOST_CHECK(!vals2.empty());
    BOOST_CHECK_EQUAL(vals2, valsExpected);
}

static const char *json1 =
"[1.10000000,{\"key1\":\"str\\u0000\",\"key2\":800,\"key3\":{\"name\":\"martian http://test.com\"}}]";

BOOST_AUTO_TEST_CASE(univalue_readwrite)
{
    UniValue v;
    BOOST_CHECK(v.read(json1));
    const UniValue vjson1 = v; // save a copy for below

    std::string strJson1(json1);
    BOOST_CHECK(v.read(strJson1));

    BOOST_CHECK(v.isArray());
    BOOST_CHECK_EQUAL(v.size(), 2);

    BOOST_CHECK_EQUAL(v[0].getValStr(), "1.10000000");

    UniValue obj = v[1];
    BOOST_CHECK(obj.isObject());
    BOOST_CHECK_EQUAL(obj.size(), 3);

    BOOST_CHECK(obj["key1"].isStr());
    std::string correctValue("str");
    correctValue.push_back('\0');
    BOOST_CHECK_EQUAL(obj["key1"].getValStr(), correctValue);
    BOOST_CHECK(obj["key2"].isNum());
    BOOST_CHECK_EQUAL(obj["key2"].getValStr(), "800");
    BOOST_CHECK(obj["key3"].isObject());

    BOOST_CHECK_EQUAL(strJson1, v.write());

    /* Check for (correctly reporting) a parsing error if the initial
       JSON construct is followed by more stuff.  Note that whitespace
       is, of course, exempt.  */

    BOOST_CHECK(v.read("  {}\n  "));
    BOOST_CHECK(v.isObject());
    BOOST_CHECK(v.read("  []\n  "));
    BOOST_CHECK(v.isArray());

    BOOST_CHECK(!v.read("@{}"));
    BOOST_CHECK(!v.read("{} garbage"));
    BOOST_CHECK(!v.read("[]{}"));
    BOOST_CHECK(!v.read("{}[]"));
    BOOST_CHECK(!v.read("{} 42"));

    // check that json escapes work correctly by putting a json string INTO a UniValue
    // and doing a round of ser/deser on it.
    v.setArray();
    v.push_back(json1);
    const auto vcopy = v;
    BOOST_CHECK(!vcopy.empty());
    v.setNull();
    BOOST_CHECK(v.empty());
    BOOST_CHECK(v.read(vcopy.write(2)));
    BOOST_CHECK(!v.empty());
    BOOST_CHECK_EQUAL(v, vcopy);
    BOOST_CHECK_EQUAL(v[0], json1);
    v.setNull();
    BOOST_CHECK(v.empty());
    BOOST_CHECK(v.read(vcopy[0].get_str())); // now deserialize the embedded json string
    BOOST_CHECK(!v.empty());
    BOOST_CHECK_EQUAL(v, vjson1); // ensure it deserializes to equal
}

BOOST_AUTO_TEST_SUITE_END()

int main()
{
    try {
        // First, we try to set the locale from the "user preferences" (typical env vars LC_ALL and/or LANG).
        // We do this for CI/testing setups that want to run this test on one of the breaking locales
        // such as "de_DE.UTF-8".
        std::setlocale(LC_ALL, "");
        std::locale loc("");
        std::locale::global(loc);
    } catch (...) {
        // If the env var specified a locale that does not exist or was installed, we can end up here.
        // Just fallback to the standard "C" locale in that case.
        std::setlocale(LC_ALL, "C");
        std::locale loc("C");
        std::locale::global(loc);
    }

    univalue_constructor();
    univalue_typecheck();
    univalue_set();
    univalue_array();
    univalue_object();
    univalue_readwrite();
    return 0;
}
