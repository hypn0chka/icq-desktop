//-< TESTINDEX.CPP >-------------------------------------------------*--------*
// GigaBASE                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     10-Dec-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Performance test for index searches
//-------------------------------------------------------------------*--------*

#include "gigabase.h"
#include <stdio.h>

USE_GIGABASE_NAMESPACE

const int nRecords = 1000000;
const int arraySize = 100;

class Record {
  public:
    db_int8       intKey;
    char_t const* strKey;

    TYPE_DESCRIPTOR((KEY(intKey, INDEXED), KEY(strKey, INDEXED)));
};

REGISTER(Record);

#ifdef _WIN32
#define GET_TIME() GetTickCount()
#else
#define GET_TIME() time(NULL)
#endif

int __cdecl main(int argc, char* argv[])
{
    int i, j, n;
    char_t  strKey[32];
    db_int8 intKey;
    dbArray<db_int8> keyArr;

    dbDatabase db(dbDatabase::dbAllAccess, 4*1024); // 32Mb page pool
    if (db.open(_T("testindex.dbs"))) {
        dbQuery q1, q2, q3, q4, q5;
        dbCursor<Record> cursor1;
        dbCursor<Record> cursor2;
        q1 = _T("intKey="),intKey;
        q2 = _T("strKey="),strKey;
#if defined(_MSC_VER) && _MSC_VER+0 < 1300
        q3.add(_T("intKey in")).append(dbQueryElement::qVarArrayOfInt8, &keyArr);
#else
        q3 = _T("intKey in"),keyArr;
#endif
        q4 = _T("order by intKey");
        q5 = _T("order by strKey");
        db_nat8 key = 1999;
        unsigned start = GET_TIME();
        for (i = 0; i < nRecords; i++) {
            Record rec;
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            SPRINTF(SPRINTF_BUFFER(strKey), T_INT8_FORMAT _T("."), key);
            rec.strKey = strKey;
            rec.intKey = (db_int8)key;
            insert(rec);
            //if (i % 100000 == 0) {
            //    printf("Iteration %d\r", i);
            //}
        }
        db.commit();
        printf("Elapsed time for inserting %d record: %d\n",
               nRecords, int(GET_TIME() - start));
        start = GET_TIME();
        key = 1999;
        for (i = 0; i < nRecords; i++) {
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            SPRINTF(SPRINTF_BUFFER(strKey), T_INT8_FORMAT _T("."), key);
            intKey = (db_int8)key;
            n = cursor1.select(q1);
            assert(n == 1);
            n = cursor2.select(q2);
            assert(n == 1);
            assert(cursor1.currentId() == cursor2.currentId());
        }
        printf("Elapsed time for %d index searches: %d\n",
               nRecords*2, int(GET_TIME() - start));

        start = GET_TIME();
        n = cursor1.select(q4);
        assert(n == nRecords);
        key = 0;
        do {
            assert((db_int8)key <= cursor1->intKey);
            key = cursor1->intKey;
        } while (cursor1.next());
        printf("Elapsed time for sequential search through %d records with sorting by integer key: %d\n",
               nRecords, int(GET_TIME() - start));

        start = GET_TIME();
        n = cursor2.select(q5);
        assert(n == nRecords);
        strKey[0] = '\0';
        do {
            assert(STRCMP(strKey, cursor2->strKey) <= 0);
            STRCPY(strKey, cursor2->strKey);
        } while (cursor2.next());
        printf("Elapsed time for sequential search through %d records with sorting by string key: %d\n",
               nRecords, int(GET_TIME() - start));

        start = GET_TIME();
        key = 1999;
        keyArr.resize(arraySize);
        db_int8* items = (db_int8*)keyArr.base();
        for (i = 0; i < nRecords/arraySize; i++) {
            for (j = 0; j < arraySize; j++) { 
                key = (3141592621u*key + 2718281829u) % 1000000007u;
                items[j] = (db_int8)key;
            }
            n = cursor1.select(q3);
            assert(n == arraySize);
        }
        printf("Elapsed time for %d 'in' searches: %d\n",
               nRecords/arraySize, int(GET_TIME() - start));
        start = GET_TIME();
        key = 1999;
        for (i = 0; i < nRecords; i++) {
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            intKey = (db_int8)key;
            n = cursor1.select(q1, dbCursorForUpdate);
            assert(n == 1);
            cursor1.remove();
        }
        printf("Elapsed time for deleting all %d records: %d\n",
               nRecords, int(GET_TIME() - start));
        db.close();
        return EXIT_SUCCESS;
    } else {
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}




