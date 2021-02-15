#include "hsql/SQLParser.h"
#include "hsql/util/sqlhelper.h"
#include <iostream>
#include <string.h>
#include <string>
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
bool dropTable(const SQLStatement* sql){
  //cout<<sql;//Change to syslog
  //Remove stuff from secret dbs and hashes
  return true;
}

/**
 * createTable function
 * 
 * @param sql the create sql statement
 * @return the new create statement
 */ 
bool createTable(const SQLStatement* sql){
  string newSql;
  //call theos function to alter
  return true;
}

/**
 * selectStatement function
 * 
 * @param sql the select sql statement
 * @return the new select statement
 */ 
bool selectStatement(const SQLStatement* sql){
  string newSql;
  return true;
}

bool getCreateInfo(CreateStatement* stmt){
  printCreateStatementInfo(stmt, 0);
  for(char* col_name : *stmt->viewColumns){
    cout<<col_name<<endl;
  } 
  return true;
}

//START OF INSERT STUFF ----------------------------------------------------------------------------------------------------

/**
 * insertInt function helps insertStatement function with integers
 * 
 * @param val the integer value in string form
 * @param colName the column name being inserted on
 * 
 * @return the new value
 */ 
string insertInt(string val, string colName){
  int direction = 1;     //GET FROM SECRET DB USING COLNAME
  int insertLocation = 1;//GET FROM SECRET DB USING COLNAME
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
 * @param colName The column name being inserted on
 *
 * @return the new value
 */ 
string insertFloat(string sval, string colName){
  int direction = 1;     //GET FROM SECRET DB USING COLNAME
  int insertLocation = 1;//GET FROM SECRET DB USING COLNAME
  double ret = 0;
  double val = stod(sval);
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
 * @param colName the column name being inserted on
 * 
 * @return the new value
 */ 
string insertString(string val, string colName){
  int direction = 1;     //GET FROM SECRET DB USING COLNAME
  int insertLocation = 1;//GET FROM SECRET DB USING COLNAME
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
 * recreateInsert function recreates the insert sql string with updated values
 * 
 * @param newVals vector of strings of the new values
 * @param colNames vector of string of the colNames
 * @param tableN table name being inserted on
 * 
 * @return the new sql query
 */ 
string recreateInsert(vector<string> newVals, vector<string> colNames, string tableN){
  string cols = "(";
  string vals = "(";
  for(int i = 0; i<colNames.size(); i++){
    if(i!=colNames.size()-1){
      cols+=colNames[i]+", ";
      vals+=newVals[i]+", ";
    }else{
      cols+=colNames[i]+")";
      vals+=newVals[i]+")";
    }
  }
  string newQuery = "INSERT INTO "+tableN+cols+" VALUES"+vals+";";
  return newQuery;
}

/**
 * getInsertInfo function helps the getInfo function
 * Finds the colNames, table name, and modifies values and sends them to the recreateInsert function 
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getInsertInfo(InsertStatement* stmt){
  //printInsertStatementInfo(insState, 0);

  float fvalue;
  int ivalue;
  string svalue;

  vector<string> colNames;//Vector of the column names
  vector<int> colTypes;//Vector of the column types  
  vector<string> valVec;//Vector of the values in string form

  for(char* col_name : *stmt->columns){//Get columns
    colNames.push_back(col_name);
  } 

  for(Expr* val : *stmt->values){//Get values, prepare them for insertion
    switch (val->type) {//Check type of value we are inserting into
      case kExprLiteralFloat://Float
        fvalue = val->fval;
        colTypes.push_back(0);
        valVec.push_back(to_string(fvalue));
        break;
      case kExprLiteralInt://Integer
        ivalue = val->ival;
        colTypes.push_back(1);
        valVec.push_back(to_string(ivalue));
        break;
      case kExprLiteralString://String
        svalue = val->name;
        colTypes.push_back(2);
        valVec.push_back(svalue);
        break;
      default:
        break;
    }
  }

  vector<string> newValues;
  for(int i=0; i<colNames.size(); i++){//Insert extra character into values
    if(colTypes[i]==0){//Float
      newValues.push_back(insertFloat(valVec[i], colNames[i]));
    }else if(colTypes[i]==1){//Integer
      newValues.push_back(insertInt(valVec[i], colNames[i]));
    }else if(colTypes[i]==2){//String
      newValues.push_back(insertString(valVec[i], colNames[i]));
    }
  } 
  string tableN = stmt->tableName; //get table name
  return recreateInsert(newValues, colNames, tableN);//Create new sql string to send back
}
//END OF INSERT STUFF ----------------------------------------------------------------------------------------------------


/**
 * getInfo function
 * Returns the columns, types, and values if required
 * 
 * @param query the sql statement in question
 * @return Array of cols, types, values
 */
string getInfo(const SQLStatement* sql){
      InsertStatement* insState;//The different statement types for grabbing more info
      CreateStatement* creState;
      SelectStatement* selState;
      ImportStatement* impState;
      ExportStatement* expState;
      TransactionStatement* tranState;//The different statement types for grabbing more info

      switch(sql->type()){
        case(kStmtUpdate): //Update
          cout<<"UPDATE"<<endl;
          break;
        case(kStmtDrop): //DROP
          cout<<"DROP"<<endl;
          dropTable(sql);
          break;
        case(kStmtCreate): //Create (Big problems here with datatypes)
          cout<<"CREATE"<<endl;
          creState = (CreateStatement*) sql;
          getCreateInfo(creState);
          break;
        case(kStmtPrepare): //Prepare
          cout<<"PREPARE"<<endl;
          break;
        case kStmtDelete: //Delete
          cout<<"DELETE"<<endl;
          break;
        case kStmtSelect: //Select
          cout<<"SELECT"<<endl;
          selectStatement(sql);
          break;
        case kStmtInsert: //Insert
          cout<<"INSERT"<<endl;
          insState = (InsertStatement*) sql;
          return getInsertInfo(insState);
          break;
        case(kStmtExecute): //Execute
          cout<<"EXECUTE"<<endl;
          break;
        case(kStmtRename): //Rename
          cout<<"RENAME"<<endl;
          break;
        case(kStmtAlter): //Alter
          cout<<"ALTER"<<endl;
          break;
        case(kStmtShow): //Show
          cout<<"SHOW"<<endl;
          break;
        case(kStmtTransaction)://Transaction
          cout<<"TRANSACTION"<<endl;
          break;
        default:
          break;
      }
    return "";
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

  string newQuery; //THIS IS WHAT WE RETURN TO THE SHIM

  // check whether the parsing was successful
  if (result.isValid()) {
    printf("Parsed successfully!\n");
    printf("Number of statements: %lu\n", result.size());
    for (auto i = 0u; i < result.size(); ++i) {
      newQuery+=getInfo(result.getStatement(i));//Send new sql string into vector one by one
    }
    cout<<newQuery<<endl;
  }else {
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