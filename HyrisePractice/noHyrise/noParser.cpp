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
  return true;
}

//START OF INSERT STUFF ----------------------------------------------------------------------------------------------------

/**
 * insertInt function helps getInsertInfo function with integers
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
 * insertFloat function helps getInsertInfo function with floating points
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
 * insertString function helps getInsertInfo function with strings
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

//START OF SELECT STUFF --------------------------------------------------------------------------------------------------


/**
 * selectInt function helps getSelectInfo function with integers(column indices)
 * 
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 * 
 * @return the new source/table name where the select data is stored without extra characters
 */ 
string selectInt(vector<string> fieldVec, vector<string> sourceVec){
  string newVal = sourceVec[0];
  return newVal+"_secret";
}

/**
 * selectFloat function helps getSelectInfo function with floats(column indices?)
 * 
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 * 
 * @return the new source/table name where the select data is stored without extra characters
 */ 
string selectFloat(vector<string> fieldVec, vector<string> sourceVec){
  string newVal = sourceVec[0];
  return newVal+"_secret";
}

/**
 * selectString function helps getSelectInfo function with strings(column names)
 * 
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 * 
 * @return the new source/table name where the select data is stored without extra characters
 */ 
string selectString(vector<string> fieldVec, vector<string> sourceVec){
  string newVal = sourceVec[0];
  return newVal+"_secret";
}

/**
 * selectStar function helps getSelectInfo function with *(all)
 * 
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 * 
 * @return the new source/table name where the select data is stored without extra characters
 */ 
string selectStar(vector<string> sourceVec){
  string newVal = sourceVec[0];
  return newVal+"_secret";
}

/**
 * recreateSelect function recreates the select sql string with updated sources
 * 
 * @param fieldVec vector of strings of the fields
 * @param sourceVec vector of string of the new sources
 * 
 * @return the new sql query
 */ 
string recreateSelect(vector<string> fieldVec, vector<string> sourceVec){
  string fields = "";
  string sources = "";
  for(int i = 0; i<fieldVec.size(); i++){
    if(i!=fieldVec.size()-1){
      fields+=fieldVec[i]+", ";
    }else{
      fields+=fieldVec[i];
    }
  }
  
  for(int i = 0; i<sourceVec.size(); i++){
    if(i!=sourceVec.size()-1){
      sources+=sourceVec[i]+", ";
    }else{
      sources+=sourceVec[i];
    }
  }
  string newQuery = "SELECT "+fields+" FROM "+sources+";";
  return newQuery;
}

string getSelectInfo(SelectStatement* stmt){
  vector<string> fieldVec;//Vector of the field names
  vector<string> sourceVec;//Vector of the source names
  vector<int> colTypes;//Vector of the select types
  printSelectStatementInfo(stmt, 0);
  float fvalue;
  int ivalue;
  string svalue;

  if(stmt->fromTable!=nullptr){//Get the source of where we are selecting from
    sourceVec.push_back(stmt->fromTable->name);
  }
  string tableN = sourceVec[0];//Temporary, I'm not sure why this wouldnt work yet
  for(Expr* val : *stmt->selectList){//Get values, prepare them for selection
    switch (val->type) {//Check type of value we are selecting from
      case kExprLiteralFloat://Float
        fvalue = val->fval;
        fieldVec.push_back(to_string(fvalue));
        colTypes.push_back(0);
        break;
      case kExprLiteralInt://Integer
        ivalue = val->ival;
        cout<<ivalue<<endl;
        fieldVec.push_back(to_string(ivalue));
        colTypes.push_back(1);
        break;
      case kExprLiteralString://String
        svalue = val->name;
        cout<<svalue<<endl;
        fieldVec.push_back(svalue);
        colTypes.push_back(2);
        break;
      case kExprStar://All
        fieldVec.push_back("*");
        colTypes.push_back(3);
        break;
      case kExprColumnRef://Column Reference(AKA column name)
        svalue = val->name;
        cout<<svalue<<endl;
        fieldVec.push_back(svalue);
        colTypes.push_back(2);
        break;
      default:
          break;
      }
  }
  vector<string> newSource;//Vector of new sources we want the user to select from(ONLY CHANGES TABLENAME RIGHT NOW)
  for(int i=0; i<sourceVec.size(); i++){//Select relevant data, make new table where extra character is removed.
    if(colTypes[i]==0){//Float(Not sure when this would be used)
      newSource.push_back(selectFloat(fieldVec, sourceVec));
    }else if(colTypes[i]==1){//Integer(Column Index)
      newSource.push_back(selectInt(fieldVec, sourceVec));
    }else if(colTypes[i]==2){//String(Column Name)
      newSource.push_back(selectString(fieldVec, sourceVec));
    }
    else if(colTypes[i]==3){//*
      newSource.push_back(selectStar(sourceVec));//Don't even need to send the * string
    }
  } 
  string newTable = newSource[0];
  return recreateSelect(fieldVec, newSource);//Create new sql string to send back
}
//END OF SELECT STUFF --------------------------------------------------------------------------------------------------
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
          return "NO";//return "NO" to show it is a create and can't be parsed by hyrise
          break;
        case(kStmtPrepare): //Prepare
          cout<<"PREPARE"<<endl;
          break;
        case kStmtDelete: //Delete
          cout<<"DELETE"<<endl;
          break;
        case kStmtSelect: //Select
          cout<<"SELECT"<<endl;
          selState = (SelectStatement*) sql;
          return getSelectInfo(selState);
          //selectStatement(sql);
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
 * getInfoString function is the same as getInfo but parses by string instead of using the hyrise parser
 * Returns the columns, types, and values if required
 * 
 * @param query the sql statement in question
 * @return Array of cols, types, values
 */
string getInfoString(string query){
  string newQuery = query;
  if(query.substr(0, 6)=="CREATE"){
    cout<<"CREATE"<<endl;
  }else{
    return "";
  }
  return newQuery;
}
/**
 * parseString function
 * 
 * @param query the entire sql statement
 * @return the new entire sql statement
 */ 
bool parseString(string query){//Maybe check every semicolon then go from there
  // parse a given query

  SQLParserResult result;
  SQLParser::parse(query, &result);

  string newQuery; //THIS IS WHAT WE RETURN TO THE SHIM
  
  // check whether the parsing was successful
  if (false) {//result.isValid() put this back maybe
    printf("Parsed successfully!\n");
    printf("Number of statements: %lu\n", result.size());
    for (auto i = 0u; i < result.size(); ++i) {
      newQuery+=getInfo(result.getStatement(i));//Send new sql string into vector one by one
    }
  }else {//The parser does not recognize something so we need to go one by one
    int maxLen = query.size()-1;
    if(query[maxLen]!=';'){//If the string doesn't end with ; then it is bad
      cout<<"Error: BAD SQL STRING."<<endl;
      return false;
    }
    printf("Error, checking one by one now.\n");
    int start = 0;
    int end = query.find(';')+1;
    string stmt;
    while(query!=""){
      stmt = query.substr(start, end);
      query = query.substr(end, maxLen);
      cout<<"Manually Parsing: "<<stmt<<endl;

      //Try to parse each individual statement normally
      SQLParserResult result;
      SQLParser::parse(stmt, &result);
      if (result.isValid()) {
        printf("Parsed successfully!\n");
        printf("Number of statements: %lu\n", result.size());
        for (auto i = 0u; i < result.size(); ++i) {
          string temp = getInfo(result.getStatement(i));
          if(temp=="NO"){//If it returns NO it means it should not be parsed by hyrise
            newQuery+=getInfoString(stmt);
          }else{
            newQuery+=temp;//Send new sql string into vector one by one
          }
        }
      }else{
        cout<<"The statement: "<<stmt<<" is being manually parsed.\n";
        string temp = getInfoString(stmt);
        if(temp==""){//Return "" if the query is unparseable
          cout<<"ERROR: "<<stmt<<" is not a valid statement."<<endl;
          return false;
        }
        newQuery+=getInfoString(stmt);//Send this new sql string into vector
      }
      end = query.find(';')+1;
      maxLen=query.size();
    }
  }
  cout<<newQuery<<endl;//Print out the final new query
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

/**
 * Known issues:
 * 1. Doubles/floating points seem to be broken again but hopefully its just the display. @Kurt
 * 2. One thought: What if we just have a secret DB where we do not insert extra characters. 
 *      -It would help some statements be faster like select
 *      -BUT it is the complete opposite of what our project is for
 * 3. SELECT only works with the simplest select statements(where field=*, colIndex, colName)
 *      -I think this is fixable by adding in the extra cases in the sqlhelper.cpp file for printSelectStatementInfo() function
 * 4. NEWSOURCE Vector in getSelectInfo needs to modify all the elements in sourceVec not just make a new tableName
 * 5. Need to finish all the selectStar, etc to actually make the new table and sources but thats for when  DB is connected
 */ 
