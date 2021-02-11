#include "hsql/SQLParser.h"
#include "hsql/util/sqlhelper.h"
#include <iostream>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <stdio.h> 
#include <bits/stdc++.h>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>

using namespace std;
using namespace hsql;

/**
 * dropTable function
 * 
 * @param sql the drop sql statement
 * @return the same drop statement
 */ 
string dropTable(string sql){
  cout<<sql;//Change to syslog
  //Remove stuff from secret dbs and hashes
  return sql;
}

/**
 * createTable function
 * 
 * @param sql the create sql statement
 * @return the new create statement
 */ 
string createTable(string sql){
  string newSql;
  //call theos function to alter
  return newSql;
}

/**
 * selectStatement function
 * 
 * @param sql the select sql statement
 * @return the new select statement
 */ 
string selectStatement(string sql){
  string newSql;
  return newSql;
}

/**
 * insertInt function helps insertStatement function with integers
 * 
 * @param val the integer value in string form
 * @param direction left/right
 * @param insertLocation how far from the left/right
 * @return the new value
 */ 
string insertInt(string val, int direction, int insertLocation){
  int len = val.length();
  string newVal = "";
    if (len >= insertLocation) {
        for (int i = 0; i < len; i++) {
            newVal += val[i];
            if(direction){
                if (i == insertLocation-1) {
                    string s1 = to_string(1);
                    newVal += s1;
                }
            }else{
                if (i == len-insertLocation) {
                    string s1 = to_string(1);
                    newVal += s1;
                }
            }
        }
    } else {//Number is shorter than designated watermark index, so stick it at the end
        newVal = val+to_string(1);
    }
  return newVal;
}

/**
 * insertDec function helps insertStatement function with floating points
 * 
 * @param val the decimal value in string form
 * @param direction left/right
 * @param insertLocation how far from the left/right
 * @return the new value
 */ 
string insertDec(double val, int direction, int insertLocation){
  double ret = 0;
    if(direction){
        double leftPart = (int)val / (int)pow(10, insertLocation);
        ret += leftPart * pow(10, insertLocation + 1);
        ret += 1 * pow(10, insertLocation);
        ret += fmod(val, pow(10, insertLocation));        
    }else{
        double rightPart = fmod(val, pow(10, -1 * insertLocation));
        double leftPart = val - rightPart;
        ret += leftPart + 1 * pow(10, -1 * (insertLocation + 1)) + rightPart * pow(10, -1);
    }
  return to_string(ret);
}

/**
 * insertString function helps insertStatement function with strings
 * 
 * @param val the string value in string form
 * @param direction left/right
 * @param insertLocation how far from the left/right
 * @return the new value
 */ 
string insertString(string val, int direction, int insertLocation){
  string newVal = "";
    int len = val.length();
    if (len >= insertLocation) {
        for (int i = 0; i < len; i++) {
            newVal += val[i];
            if(direction){
                if (i == insertLocation-1) {
                    char s1 = char('a'-1 + 1);
                    newVal += s1;
                }
            }else{
                if (i == len-insertLocation) {
                    char s1 = char('a'-1 + 1);
                    newVal += s1;
                }
            }
        }
    } else {//Number is shorter than designated watermark index, so stick it at the end
        newVal = val+to_string(1);
    }
  return newVal;
}

/**
 * insertStatement function
 * 
 * @param sql the insert sql statement
 * @return the new insert statement
 */ 
string insertStatement(string sql){
  string newSql;
  int direction = 0; //Read from secret db (0 = left, 1 = right)
  int insertLocation = 2; //Read from secret db
  int type = 1; //Read from secret db (0 = integer, 1 = decimal, 2 = string)
  
  //long long num = 1;
  //float dec = 1.0;
  //string str = "insert here";
  string val = "100";
  if(type==0){
    cout<<"Int: "<<insertInt(val, direction, insertLocation)<<endl;
  }else if(type==1){
    double test = 1000;
    cout<<"Double: "<<insertDec(test, direction, insertLocation)<<endl;
  }else if(type==2){
    cout<<"String: "<<insertString(val, direction, insertLocation)<<endl;
  }
  return newSql;
}

/**
 * parseString function
 * 
 * @param query the entire sql statement
 * @return the new entire sql statement
 */ 
bool parseString(string query){
  // parse a given query
  SQLParserResult result;
  SQLParser::parse(query, &result);

  // check whether the parsing was successful
  if (result.isValid()) {
    printf("Parsed successfully!\n");
    printf("Number of statements: %lu\n", result.size());
    for (auto i = 0u; i < result.size(); ++i) {
      // Print a statement summary.
      //printStatementInfo(result.getStatement(i));
      //int typ = result.getStatement(i)->type();

      InsertStatement* insState;//The different statement types for grabbing more info
      CreateStatement* creState;
      SelectStatement* selState;
      ImportStatement* impState;
      ExportStatement* expState;
      TransactionStatement* tranState;//The different statement types for grabbing more info

      vector<string> col_names;
      vector<string> data_vec;
      switch(result.getStatement(i)->type()){
        case(kStmtUpdate):
          cout<<"UPDATE"<<endl;
          break;
        case(kStmtDrop): //DROP
          cout<<"DROP"<<endl;
          dropTable(query);
          break;
        case(kStmtCreate): //Create (Big problems here with datatypes)
          cout<<"CREATE"<<endl;
          creState = (CreateStatement*) result.getStatement(i);
          printCreateStatementInfo(creState, 0);
          for(char* col_name : *creState->columns){
            cout<<col_name<<endl;
          } 
          insertStatement(query);
          createTable(query);
          break;
        case(kStmtPrepare):
          cout<<"PREPARE"<<endl;
          break;
        case kStmtDelete:
          cout<<"DELETE"<<endl;
          break;
        case kStmtSelect: //Select
          cout<<"SELECT"<<endl;
          selectStatement(query);
          break;
        case kStmtInsert: //Insert
          cout<<"INSERT"<<endl;
          insState = (InsertStatement*) result.getStatement(i);
          printInsertStatementInfo(insState, 0);
          for(char* col_name : *insState->columns){
            cout<<col_name<<endl;
          } 
          insertStatement(query);
          break;
        case(kStmtExecute):
          cout<<"EXECUTE"<<endl;
          break;
        case(kStmtRename):
          cout<<"RENAME"<<endl;
          break;
        case(kStmtAlter):
          cout<<"ALTER"<<endl;
          break;
        case(kStmtShow):
          cout<<"SHOW"<<endl;
          break;
        case(kStmtTransaction):
          cout<<"TRANSACTION"<<endl;
          break;
        default:
          break;
      }
    }
    return true;
  } else {
    fprintf(stderr, "Given string is not a valid SQL query.\n");
    fprintf(stderr, "%s (L%d:%d)\n",
            result.errorMsg(),
            result.errorLine(),
            result.errorColumn());
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: ./example \"SELECT * FROM test;\"\n");
    return -1;
  }

  if(!parseString(argv[1])){
    return 1;
  }

  return 0;
}