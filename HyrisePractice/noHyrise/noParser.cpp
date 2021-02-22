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

/**
 * getSelectInfo function helps the getInfo function
 * Finds the fields and sources, modifies values and sends them to the recreateSelect function 
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
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
          return "NO";
          break;
        case(kStmtDrop): //DROP
          cout<<"DROP"<<endl;
          return "NO";
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
          return "NO";
          break;
        case kStmtSelect: //Select
          cout<<"SELECT"<<endl;
          selState = (SelectStatement*) sql;
          return getSelectInfo(selState);
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
          return "NO";
          break;
        case(kStmtShow): //Show
          cout<<"SHOW"<<endl;
          break;
        case(kStmtTransaction)://Transaction
          cout<<"TRANSACTION"<<endl;
          break;
        default:
          cout<<"NO TYPE FOUND"<<endl;
          return "NO";
          break;
      }
    return "";
}

//START OF CREATE STUFF ----------------------------------------------------------------------------------------------------

/**
 * recreateCreate function recreates the create sql string with updated datatypes and overflow columns
 * 
 * @param leftSide string of the "CREATE TABLE ____(" statement
 * @param colNames vector of strings of the colNames
 * @param dataTypes vector of string of the col datatypes
 * 
 * @return the new sql query
 */ 
string recreateCreate(string leftSide, vector<string> colNames, vector<string> dataTypes){
  string rightSide = "";
  for(int i = 0; i<colNames.size(); i++){
    cout<<dataTypes[i]<<endl;
    if(i!=colNames.size()-1){
      rightSide+=colNames[i]+" "+dataTypes[i]+ ", ";
    }else{
      rightSide+=colNames[i]+" "+dataTypes[i]+");";
    }
  }
  return leftSide+rightSide;
}

/**
 * convertDatatypes function converts the datatypes in create table statement to next highest
 * 
 * @param dTs vector of datatypes from sql statement
 * 
 * @return vector of new datatypes
 */ 
vector<string> convertDatatypes(vector<string> dTs){
  vector<string> newVec;
  for(string dt : dTs){
    if(dt=="smallint"){
      newVec.push_back("int");
    }else if(dt=="int"){
      newVec.push_back("bigint");
    }else if(dt=="bigint"){
      newVec.push_back("numeric");
    }
  }
  return newVec;
}
/**
 * getCreateInfo function helps the getInfoString function
 * Finds the colNames and dataTypes and modifies dataTypes and sends them to the recreateCreate function 
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getCreateInfo(string stmt){
  vector<string> colNames;
  vector<string> dataTypes;
  bool nameB = true; //Check if reading colname
  bool typeB = false; //Check if reading coltype
  string name = "";
  string type = "";

  int maxLen = stmt.length();
  int end = stmt.find('(');
  string newStmt = stmt.substr(0,end+1);
  string colData = stmt.substr(end, maxLen);
  for(int i=1; i<colData.length(); i++){
    if(colData[i]==' '){
      colNames.push_back(name);
      name="";
      typeB=true;
      nameB=false;
    }else if(colData[i]==','){
      dataTypes.push_back(type);
      type="";
      typeB=false;
      nameB=true;
    }else if(colData[i]==')'){
      dataTypes.push_back(type);
      type="";
      typeB=false;
      nameB=true;
      break;
    }else{
      if(typeB){
        type+=colData[i];
      }else{
        name+=colData[i];
      }
    }
  }
  //Send to function to increase the scale by 1 of each data type
  return recreateCreate(newStmt, colNames, convertDatatypes(dataTypes));//Create and return new sql string
}

//END OF CREATE STUFF ----------------------------------------------------------------------------------------------------

//START OF DROP STUFF ----------------------------------------------------------------------------------------------------

/**
 * recreateDropTable function recreates the drop table sql string with updated data if necesary(not yet)
 * 
 * @param tableN name of table being dropped
 * 
 * @return the new sql query
 */ 
string recreateDropTable(string tableN){
  return "DROP TABLE "+tableN+";";
}
/**
 * recreateDropDatabase function recreates the drop database sql string with updated data if necesary(not yet)
 * 
 * @param dbN name of database being dropped
 * 
 * @return the new sql query
 */ 
string recreateDropDatabase(string dbN){
  return "DROP DATABASE "+dbN+";";
}

/**
 * getDropInfo function helps the getInfoString function
 * Finds the table name or database name, removes them from our data, and sends them to the recreateDropTable or Database function 
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getDropInfo(string stmt){
  int maxLen = stmt.size();
  int sz;
  if(stmt.substr(5, 5)=="TABLE"){
    cout<<"DROP TABLE"<<endl;
    sz = maxLen-12;
    string tableN = stmt.substr(11,sz);//The table name we are dropping
    //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
    return recreateDropTable(tableN);
  }else if(stmt.substr(5, 8)=="DATABASE"){
    cout<<"DROP DATABASE"<<endl;
    sz = maxLen-15;
    string dbN = stmt.substr(14,sz);//The table name we are dropping
    //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
    return recreateDropDatabase(dbN);
  }else{
    return "drop error";
  }
}
//END OF DROP STUFF ----------------------------------------------------------------------------------------------------
/**
 * recreateDeleteTable function recreates the delete table sql string with updated data if necesary(not yet)
 * 
 * @param tableN name of table being deleted from
 * 
 * @return the new sql query
 */ 
string recreateDeleteTable(string tableN){
  return "DELETE FROM "+tableN+";";
}

/**
 * getDeleteInfo function helps the getInfoString function
 * Finds the delete info, deletes and rehashes using temp tables and sends them to the recreateDelete functions
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getDeleteInfo(string stmt){
  int maxLen = stmt.size();
  int sz;
  if(stmt.substr(7, 4)=="FROM"){
    cout<<"DELETE FROM"<<endl;
    sz = maxLen-13;
    string tableN = stmt.substr(12,sz);//The table name we are dropping
    //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
    return recreateDeleteTable(tableN);
  }else{
    return "delete error";
  }
}

/**
 * getTruncateInfo function helps the getInfoString function
 * Finds the table name to delete from our data
 * 
 * @param stmt the sql statement in question
 * @return same sql statement
 */
string getTruncateInfo(string stmt){
  int maxLen = stmt.size();
  int sz;
  cout<<"TRUNCATE"<<endl;
  sz = maxLen-10;
  string tableN = stmt.substr(9,sz);//The table name we are dropping
  cout<<tableN<<endl;
  //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
  return stmt;
}

/**
 * recreateUpdateTable function recreates the update sql string with updated data if necesary(not yet)
 * 
 * @param tableN name of table being updated
 * @param setCol name of column being set
 * @param setVal value being set in col
 * @param whereCol name of column where...
 * @param whereVal value of column where...
 * 
 * @return the new sql query
 */ 
string recreateUpdateTable(string tableN, string setCol, string setVal, string whereCol, string whereVal){
  //Gotta do all the select statements to get the real values
  return "UPDATE "+tableN+" SET "+setCol+"="+setVal+" WHERE "+ whereCol+"="+whereVal+";";
}

/**
 * getUpdateInfo function helps the getInfoString function
 * Finds the delete info, deletes and rehashes using temp tables and sends them to the recreateDelete functions
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getUpdateInfo(string stmt){
  int maxLen = stmt.size();
  int set = stmt.find("SET")+4;
  int where = stmt.find("WHERE");
  string tableN = stmt.substr(7, set-12);
  string setCol;
  string setVal;
  string whereCol;
  string whereVal;
  int setEq = stmt.find("=");
  string setStr;
  string whereStr;
  if(where>=0){
    setStr = stmt.substr(set, where-set-1);
    whereStr = stmt.substr(where+6, maxLen-where-7);
    int whereEq = whereStr.find("=");
    whereCol = whereStr.substr(0,whereEq);
    whereVal = whereStr.substr(whereEq+1, whereStr.size()-whereEq);
  }else{
    setStr = stmt.substr(set, maxLen-set-1);
  }
  setEq = setStr.find("=");
  setCol = setStr.substr(0, setEq);
  setVal = setStr.substr(setEq+1, setStr.size()-setEq);
  return recreateUpdateTable(tableN, setCol, setVal, whereCol, whereVal);
}


/**
 * recreateAlterAddTable function recreates the alter add sql string with updated datatype
 * 
 * @param tableN name of table being deleted from
 * @param colN name of column being added
 * @param colType type of column being added
 * 
 * @return the new sql query
 */ 
string recreateAlterAddTable(string tableN, string colN, string colType){
  //Add the new table into secret data
  return "ALTER TABLE "+tableN+ " ADD "+colN+" "+ colType+";";
}

/**
 * recreateAlterDropTable function recreates the alter add sql string with updated datatype
 * 
 * @param tableN name of table being deleted from
 * @param colN name of column being added
 * 
 * @return the new sql query
 */ 
string recreateAlterDropTable(string tableN, string colN){
  //Delete the dropped column from secret data
  return "ALTER TABLE "+tableN+ " DROP COLUMN "+colN+";";
}

/**
 * getAlterInfo function helps the getInfoString function
 * Finds the tableName, columnName, and columnType of ADD alter statements
 * 
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getAlterInfo(string stmt){
  string tableN;
  string colN;
  string colType;
  vector<string> dataType;
  int maxLen = stmt.size();
  int add = stmt.find("ADD");
  int drop = stmt.find("DROP");
  if(add>=0){
    tableN = stmt.substr(12, maxLen-add-8);
    bool first = false;
    for(int i=add+4; i<maxLen-1; i++){
      if(stmt[i]==' '){
        first=true;
        continue;
      }
      if(!first){
        colN+=stmt[i];
      }else{
        colType+=stmt[i];
      }
    }
    dataType.push_back(colType);
    return recreateAlterAddTable(tableN, colN, convertDatatypes(dataType)[0]);
  }else{
    tableN = stmt.substr(12, maxLen-drop-12);
    colN = stmt.substr(drop+12, maxLen-drop-13);
    cout<<colN<<endl;
    return recreateAlterDropTable(tableN, colN);
  }
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
    return getCreateInfo(query);
  }else if(query.substr(0,4)=="DROP"){
    return getDropInfo(query); 
  }else if(query.substr(0,6)=="DELETE"){
    return getDeleteInfo(query);
  }else if(query.substr(0,6)=="UPDATE"){
    return getUpdateInfo(query);
  }else if(query.substr(0,8)=="TRUNCATE"){
    return getTruncateInfo(query);
  }else if(query.substr(0,5)=="ALTER"){
    return getAlterInfo(query);
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
      printStatementInfo(result.getStatement(i));
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
        newQuery+=temp;//Send this new sql string into vector
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
 * 6. For CREATE still need to increment all the data types
 * 7. For DROPS I only do the simplest statement(DROP TABLE xxx)
 * 8. For UPDATE I need to check if there's no WHERE clause when i recreate. Also need to do SELECTs to get the real values
 * 9. For ALTER I did ADD but need to do DROP too
 */ 
