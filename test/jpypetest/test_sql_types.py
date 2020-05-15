# -*- coding: iso-8859-1 -*-
# pysqlite2/test/types.py: tests for type conversion and detection
#
# Copyright (C) 2005 Gerhard H�ring <gh@ghaering.de>
#
# This file is part of pysqlite.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# This is a modification of the CPython SQLite testbench for JPype.
# The original can be found in cpython/Lib/sqlite3/test
#

import sys
import _jpype
import jpype
from jpype.types import *
from jpype import JPackage, java
import common
import pytest

import datetime
import unittest
import jpype.dbapi2 as dbapi2
try:
    import zlib
except ImportError:
    zlib = None


def getConnection():
    return "jdbc:sqlite::memory:"


class SqliteTypeTests(common.JPypeTestCase):

    def setUp(self):
        common.JPypeTestCase.setUp(self)
        self.con = dbapi2.connect(getConnection())
        self.cur = self.con.cursor()
        self.cur.execute(
            "create table test(i integer, s varchar, f number, b blob)")

    def tearDown(self):
        self.cur.close()
        self.con.close()

    def testString(self):
        self.cur.execute("insert into test(s) values (?)", ("�sterreich",))
        self.cur.execute("select s from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], "�sterreich")

    def testSmallInt(self):
        self.cur.execute("insert into test(i) values (?)", (42,))
        self.cur.execute("select i from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], 42)

    def testLargeInt(self):
        num = 2**40
        self.cur.execute("insert into test(i) values (?)", (num,))
        self.cur.execute("select i from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], num)

    def testFloat(self):
        val = 3.14
        self.cur.execute("insert into test(f) values (?)", (val,))
        self.cur.execute("select f from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], val)

    def testBlob(self):
        sample = b"Guglhupf"
        val = memoryview(sample)
        self.cur.execute("insert into test(b) values (?)", (val,))
        self.cur.execute("select b from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], sample)

    def testUnicodeExecute(self):
        self.cur.execute("select '�sterreich'")
        row = self.cur.fetchone()
        self.assertEqual(row[0], "�sterreich")


class DeclTypesTests(common.JPypeTestCase):

    def setUp(self):
        common.JPypeTestCase.setUp(self)

    class Foo:
        def __init__(self, _val):
            if isinstance(_val, bytes):
                # sqlite3 always calls __init__ with a bytes created from a
                # UTF-8 string when __conform__ was used to store the object.
                _val = _val.decode('utf-8')
            self.val = _val

        def __eq__(self, other):
            if not isinstance(other, DeclTypesTests.Foo):
                return NotImplemented
            return self.val == other.val

        def __conform__(self, protocol):
            if protocol is dbapi2.PrepareProtocol:
                return self.val
            else:
                return None

        def __str__(self):
            return "<%s>" % self.val

    class BadConform:
        def __init__(self, exc):
            self.exc = exc

        def __conform__(self, protocol):
            raise self.exc

    def setUp(self):
        self.con = dbapi2.connect(
            getConnection(), detect_types=dbapi2.PARSE_DECLTYPES)
        self.cur = self.con.cursor()
        self.cur.execute(
            "create table test(i int, s str, f float, b bool, u unicode, foo foo, bin blob, n1 number, n2 number(5), bad bad)")

        # override float, make them always return the same number
        dbapi2.converters["FLOAT"] = lambda x: 47.2

        # and implement two custom ones
        dbapi2.converters["BOOL"] = lambda x: bool(int(x))
        dbapi2.converters["FOO"] = DeclTypesTests.Foo
        dbapi2.converters["BAD"] = DeclTypesTests.BadConform
        dbapi2.converters["WRONG"] = lambda x: "WRONG"
        dbapi2.converters["NUMBER"] = float

    def tearDown(self):
        del dbapi2.converters["FLOAT"]
        del dbapi2.converters["BOOL"]
        del dbapi2.converters["FOO"]
        del dbapi2.converters["BAD"]
        del dbapi2.converters["WRONG"]
        del dbapi2.converters["NUMBER"]
        self.cur.close()
        self.con.close()

    def testString(self):
        # default
        self.cur.execute("insert into test(s) values (?)", ("foo",))
        self.cur.execute('select s as "s [WRONG]" from test')
        row = self.cur.fetchone()
        self.assertEqual(row[0], "foo")

    def testSmallInt(self):
        # default
        self.cur.execute("insert into test(i) values (?)", (42,))
        self.cur.execute("select i from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], 42)

    def testLargeInt(self):
        # default
        num = 2**40
        self.cur.execute("insert into test(i) values (?)", (num,))
        self.cur.execute("select i from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], num)

    def testFloat(self):
        # custom
        val = 3.14
        self.cur.execute("insert into test(f) values (?)", (val,))
        self.cur.execute("select f from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], 47.2)

    def testBool(self):
        # custom
        self.cur.execute("insert into test(b) values (?)", (False,))
        self.cur.execute("select b from test")
        row = self.cur.fetchone()
        self.assertIs(row[0], False)

        self.cur.execute("delete from test")
        self.cur.execute("insert into test(b) values (?)", (True,))
        self.cur.execute("select b from test")
        row = self.cur.fetchone()
        self.assertIs(row[0], True)

    def testUnicode(self):
        # default
        val = "\xd6sterreich"
        self.cur.execute("insert into test(u) values (?)", (val,))
        self.cur.execute("select u from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], val)

    def testFoo(self):
        val = DeclTypesTests.Foo("bla")
        self.cur.execute("insert into test(foo) values (?)", (val,))
        self.cur.execute("select foo from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], val)

    def testErrorInConform(self):
        val = DeclTypesTests.BadConform(TypeError)
        with self.assertRaises(dbapi2.InterfaceError):
            self.cur.execute("insert into test(bad) values (?)", (val,))
        with self.assertRaises(dbapi2.InterfaceError):
            self.cur.execute(
                "insert into test(bad) values (:val)", {"val": val})

        val = DeclTypesTests.BadConform(KeyboardInterrupt)
        with self.assertRaises(KeyboardInterrupt):
            self.cur.execute("insert into test(bad) values (?)", (val,))
        with self.assertRaises(KeyboardInterrupt):
            self.cur.execute(
                "insert into test(bad) values (:val)", {"val": val})

    def testUnsupportedSeq(self):
        class Bar:
            pass
        val = Bar()
        with self.assertRaises(dbapi2.InterfaceError):
            self.cur.execute("insert into test(f) values (?)", (val,))

#    def testUnsupportedDict(self):
#        class Bar:
#            pass
#        val = Bar()
#        with self.assertRaises(dbapi2.InterfaceError):
#            self.cur.execute("insert into test(f) values (:val)", {"val": val})

    def testBlob(self):
        # default
        sample = b"Guglhupf"
        val = memoryview(sample)
        self.cur.execute("insert into test(bin) values (?)", (val,))
        self.cur.execute("select bin from test")
        row = self.cur.fetchone()
        self.assertEqual(row[0], sample)

    def testNumber1(self):
        self.cur.execute("insert into test(n1) values (5)")
        value = self.cur.execute("select n1 from test").fetchone()[0]
        # if the converter is not used, it's an int instead of a float
        self.assertEqual(type(value), float)

    def testNumber2(self):
        """Checks whether converter names are cut off at '(' characters"""
        self.cur.execute("insert into test(n2) values (5)")
        value = self.cur.execute("select n2 from test").fetchone()[0]
        # if the converter is not used, it's an int instead of a float
        self.assertEqual(type(value), float)


class ColNamesTests(common.JPypeTestCase):

    def setUp(self):
        common.JPypeTestCase.setUp(self)
        self.con = dbapi2.connect(
            getConnection(), detect_types=dbapi2.PARSE_COLNAMES)
        self.cur = self.con.cursor()
        self.cur.execute("create table test(x foo)")

        dbapi2.converters["FOO"] = lambda x: "[%s]" % x.decode("ascii")
        dbapi2.converters["BAR"] = lambda x: "<%s>" % x.decode("ascii")
        dbapi2.converters["EXC"] = lambda x: 5 / 0
        dbapi2.converters["B1B1"] = lambda x: "MARKER"

    def tearDown(self):
        del dbapi2.converters["FOO"]
        del dbapi2.converters["BAR"]
        del dbapi2.converters["EXC"]
        del dbapi2.converters["B1B1"]
        self.cur.close()
        self.con.close()

    def testDeclTypeNotUsed(self):
        """
        Assures that the declared type is not used when PARSE_DECLTYPES
        is not set.
        """
        self.cur.execute("insert into test(x) values (?)", ("xxx",))
        self.cur.execute("select x from test")
        val = self.cur.fetchone()[0]
        self.assertEqual(val, "xxx")

    def testNone(self):
        self.cur.execute("insert into test(x) values (?)", (None,))
        self.cur.execute("select x from test")
        val = self.cur.fetchone()[0]
        self.assertEqual(val, None)

    def testColName(self):
        self.cur.execute("insert into test(x) values (?)", ("xxx",))
        self.cur.execute('select x as "x [bar]" from test')
        val = self.cur.fetchone()[0]
        self.assertEqual(val, "<xxx>")

        # Check if the stripping of colnames works. Everything after the first
        # whitespace should be stripped.
        self.assertEqual(self.cur.description[0][0], "x")

    def testCaseInConverterName(self):
        self.cur.execute("select 'other' as \"x [b1b1]\"")
        val = self.cur.fetchone()[0]
        self.assertEqual(val, "MARKER")

    def testCursorDescriptionNoRow(self):
        """
        cursor.description should at least provide the column name(s), even if
        no row returned.
        """
        self.cur.execute("select * from test where 0 = 1")
        self.assertEqual(self.cur.description[0][0], "x")

    def testCursorDescriptionInsert(self):
        self.cur.execute("insert into test values (1)")
        self.assertIsNone(self.cur.description)


class CommonTableExpressionTests(common.JPypeTestCase):

    def setUp(self):
        common.JPypeTestCase.setUp(self)
        self.con = dbapi2.connect(getConnection())
        self.cur = self.con.cursor()
        self.cur.execute("create table test(x foo)")

    def tearDown(self):
        self.cur.close()
        self.con.close()

    def testCursorDescriptionCTESimple(self):
        self.cur.execute("with one as (select 1) select * from one")
        self.assertIsNotNone(self.cur.description)
        self.assertEqual(self.cur.description[0][0], "1")

    def testCursorDescriptionCTESMultipleColumns(self):
        self.cur.execute("insert into test values(1)")
        self.cur.execute("insert into test values(2)")
        self.cur.execute(
            "with testCTE as (select * from test) select * from testCTE")
        self.assertIsNotNone(self.cur.description)
        self.assertEqual(self.cur.description[0][0], "x")

    def testCursorDescriptionCTE(self):
        self.cur.execute("insert into test values (1)")
        self.cur.execute(
            "with bar as (select * from test) select * from test where x = 1")
        self.assertIsNotNone(self.cur.description)
        self.assertEqual(self.cur.description[0][0], "x")
        self.cur.execute(
            "with bar as (select * from test) select * from test where x = 2")
        self.assertIsNotNone(self.cur.description)
        self.assertEqual(self.cur.description[0][0], "x")


class ObjectAdaptationTests(common.JPypeTestCase):

    def cast(obj):
        return float(obj)
    cast = staticmethod(cast)

    def setUp(self):
        common.JPypeTestCase.setUp(self)
        self.con = dbapi2.connect(getConnection())
        try:
            del dbapi2.adapters[int]
        except:
            pass
        dbapi2.register_adapter(int, ObjectAdaptationTests.cast)
        self.cur = self.con.cursor()

    def tearDown(self):
        del dbapi2.adapters[(int, dbapi2.PrepareProtocol)]
        self.cur.close()
        self.con.close()

    def testCasterIsUsed(self):
        self.cur.execute("select ?", (4,))
        val = self.cur.fetchone()[0]
        self.assertEqual(type(val), float)


@unittest.skipUnless(zlib, "requires zlib")
class BinaryConverterTests(common.JPypeTestCase):

    def convert(s):
        return zlib.decompress(s)
    convert = staticmethod(convert)

    def setUp(self):
        common.JPypeTestCase.setUp(self)
        self.con = dbapi2.connect(
            getConnection(), detect_types=dbapi2.PARSE_COLNAMES)
        dbapi2.register_converter("bin", BinaryConverterTests.convert)

    def tearDown(self):
        self.con.close()

    def testBinaryInputForConverter(self):
        testdata = b"abcdefg" * 10
        result = self.con.execute(
            'select ? as "x [bin]"', (memoryview(zlib.compress(testdata)),)).fetchone()[0]
        self.assertEqual(testdata, result)


class DateTimeTests(common.JPypeTestCase):

    def setUp(self):
        common.JPypeTestCase.setUp(self)
        self.con = dbapi2.connect(
            getConnection(), detect_types=dbapi2.PARSE_DECLTYPES)
        self.cur = self.con.cursor()
        self.cur.execute("create table test(d date, ts timestamp)")

    def tearDown(self):
        self.cur.close()
        self.con.close()

    def testSqliteDate(self):
        d = dbapi2.Date(2004, 2, 14)
        self.cur.execute("insert into test(d) values (?)", (d,))
        self.cur.execute("select d from test")
        d2 = self.cur.fetchone()[0]
        self.assertEqual(d, d2)

    def testSqliteTimestamp(self):
        ts = dbapi2.Timestamp(2004, 2, 14, 7, 15, 0)
        self.cur.execute("insert into test(ts) values (?)", (ts,))
        self.cur.execute("select ts from test")
        ts2 = self.cur.fetchone()[0]
        self.assertEqual(ts, ts2)

    def testSqlTimestamp(self):
        now = datetime.datetime.utcnow()
        self.cur.execute("insert into test(ts) values (current_timestamp)")
        self.cur.execute("select ts from test")
        ts = self.cur.fetchone()[0]
        self.assertEqual(type(ts), datetime.datetime)
        self.assertEqual(ts.year, now.year)

    def testDateTimeSubSeconds(self):
        ts = dbapi2.Timestamp(2004, 2, 14, 7, 15, 0, 500000)
        self.cur.execute("insert into test(ts) values (?)", (ts,))
        self.cur.execute("select ts from test")
        ts2 = self.cur.fetchone()[0]
        print(str(ts))
        print(str(ts2))
        self.assertEqual(ts, ts2)

    def testDateTimeSubSecondsFloatingPoint(self):
        ts = dbapi2.Timestamp(2004, 2, 14, 7, 15, 0, 510241)
        self.cur.execute("insert into test(ts) values (?)", (ts,))
        self.cur.execute("select ts from test")
        ts2 = self.cur.fetchone()[0]
        print(str(ts))
        print(str(ts2))
        self.assertEqual(ts, ts2)