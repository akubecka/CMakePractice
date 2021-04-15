#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <syslog.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h> 
#include <bits/stdc++.h>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>

#include "hsql/SQLParser.h"
#include "hsql/util/sqlhelper.h"
#include <pqxx/pqxx>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

//#include "tf_common.h"
//#include "sql_parser.h"

using namespace std;
using namespace hsql;
using namespace pqxx;
using namespace rapidjson;


const string SECRET_DB_HANDLE = "dbname = secret_metadata user = sdp password = rumble hostaddr = 127.0.0.1 port = 5432";
const string POSTGRES_DB_HANDLE = "dbname = postgres user = postgres password = postgres hostaddr = 127.0.0.1 port = 5432";
const string SUPERUSER_DB_HANDLE = "dbname = sdp_test user = postgres password = postgres hostaddr = 127.0.0.1 port = 5432"; //superuser, change later 

/**
 * insertInt function helps getInsertInfo function with integers
 *
 * @param val the integer value in string form
 * @param colName the column name being inserted on
 *
 * @return the new value
 */
string insertInt(string val, string colName) {
    int direction = 1;     //GET FROM SECRET DB USING COLNAME
    int insertLocation = 1;//GET FROM SECRET DB USING COLNAME
    int len = val.length();
    string newVal = "";
    if (len >= insertLocation) {
        for (int i = 0; i < len; i++) {
            newVal += val[i];
            if (direction) {
                if (i == insertLocation - 1) {
                    string s1 = to_string(1);
                    newVal += s1;
                }
            }
            else {
                if (i == len - insertLocation) {
                    string s1 = to_string(1);
                    newVal += s1;
                }
            }
        }
    }
    else {//Number is shorter than designated watermark index, so stick it at the end
        newVal = val + to_string(1);
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
string insertFloat(string sval, string colName) {
    int direction = 1;     //GET FROM SECRET DB USING COLNAME
    int insertLocation = 1;//GET FROM SECRET DB USING COLNAME
    double ret = 0;
    double val = stod(sval);
    if (direction) {
        double leftPart = (int)val / (int)pow(10, insertLocation);
        ret += leftPart * pow(10, insertLocation + 1);
        ret += 1 * pow(10, insertLocation);
        ret += fmod(val, pow(10, insertLocation));
    }
    else {
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
string insertString(string val, string colName) {
    int direction = 1;     //GET FROM SECRET DB USING COLNAME
    int insertLocation = 1;//GET FROM SECRET DB USING COLNAME
    string newVal = "";
    int len = val.length();
    if (len >= insertLocation) {
        for (int i = 0; i < len; i++) {
            newVal += val[i];
            if (direction) {
                if (i == insertLocation - 1) {
                    char s1 = char('a' - 1 + 1);
                    newVal += s1;
                }
            }
            else {
                if (i == len - insertLocation) {
                    char s1 = char('a' - 1 + 1);
                    newVal += s1;
                }
            }
        }
    }
    else {//Number is shorter than designated watermark index, so stick it at the end
        newVal = val + to_string(1);
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
string recreateInsert(vector<string> newVals, vector<string> colNames, string tableN) {
    string cols = "(";
    string vals = "(";
    for (int i = 0; i < colNames.size(); i++) {
        if (i != colNames.size() - 1) {
            cols += colNames[i] + ", ";
            vals += newVals[i] + ", ";
        }
        else {
            cols += colNames[i] + ")";
            vals += newVals[i] + ")";
        }
    }
    //Rehash here
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    StringBuffer json_str_tbl;

    result select_col{ txn.exec("select data from metadata where table_name='"+tableN+"';") }; //get the table names, and column associate
    Document d;
    for(auto const &row: select_col){
        field const field = row[0];
        d.Parse(field.c_str());
    }
    int i=0;
    for (int j = 0; j < colNames.size(); j++) {
        int cl = colNames[i].length();
        char charC[cl+1];
        strcpy(charC, colNames[i].c_str());
        Value colName;
        colName.SetString(StringRef(charC));
        Value& item = d["cols"];
        for(int i=0; i<(int)item.Size(); i++){
            if(item[i]["name"].GetString()==colName){
                if(d["cols"].GetArray()[i].HasMember("hashes")){
                    Value hashArr;
                    hashArr.SetArray();
                    //hashArr=d["cols"].GetArray()[i];//Copy hashes array. I need to copy but gotta be dereferenced or something
                    //Fix the hashArr with whatever the hash algo says
                    d["cols"].GetArray()[i].RemoveMember("hashes");
                    d["cols"].GetArray()[i].AddMember("hashes", hashArr, d.GetAllocator());
                }else{
                    Value hashArr;
                    hashArr.SetArray();
                    //Fill the hashArr with whatever the hash algo says
                    d["cols"].GetArray()[i].AddMember("hashes", hashArr, d.GetAllocator());
                }
            }
        }
        Writer<StringBuffer> writer(json_str_tbl);
        d.Accept(writer);
        string output = json_str_tbl.GetString();
        txn.exec0("update metadata set data = '" + output + "';");
        json_str_tbl.Clear();
    }
        
    txn.commit();

    string newQuery = "INSERT INTO " + tableN + cols + " VALUES" + vals + ";";
    return newQuery;
}

/**
 * getInsertInfo function helps the getInfo function
 * Finds the colNames, table name, and modifies values and sends them to the recreateInsert function
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getInsertInfo(InsertStatement* stmt) {
    //printInsertStatementInfo(insState, 0);

    float fvalue;
    int ivalue;
    string svalue;

    vector<string> colNames;//Vector of the column names
    vector<int> colTypes;//Vector of the column types  
    vector<string> valVec;//Vector of the values in string form

    for (char* col_name : *stmt->columns) {//Get columns
        colNames.push_back(col_name);
    }

    for (Expr* val : *stmt->values) {//Get values, prepare them for insertion
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
    for (int i = 0; i < colNames.size(); i++) {//Insert extra character into values
        if (colTypes[i] == 0) {//Float
            newValues.push_back(insertFloat(valVec[i], colNames[i]));
        }
        else if (colTypes[i] == 1) {//Integer
            newValues.push_back(insertInt(valVec[i], colNames[i]));
        }
        else if (colTypes[i] == 2) {//String
            newValues.push_back(insertString(valVec[i], colNames[i]));
        }
    }
    string tableN = stmt->tableName; //get table name
    return recreateInsert(newValues, colNames, tableN);//Create new sql string to send back
}

//END OF INSERT STUFF ----------------------------------------------------------------------------------------------------

//START OF SELECT STUFF --------------------------------------------------------------------------------------------------

/**
 * getInt function converts an int value back to its normal value without added character
 *
 * @param num the integer value in string form
 * @param colName The column name it belongs to
 *
 * @return the new value
 */
int getInt(string s, string colName) {
    bool side = true;     //GET FROM SECRET DB USING COLNAME
    int distance = 1;     //GET FROM SECRET DB USING COLNAME
    stringstream ss;
    string newStr = "";
    int len = s.length();
    if (len > distance) {
        for (int i = 0; i < len; i++) {
            if(side){
                if (i != distance) {
                    newStr += s[i];
                }
            }else{
                if (i != len-distance) {
                    newStr += s[i];
                }
            }
        }
    }else {
        for (int i = 0; i < len; i++) {
            if (i != len-1) {
                newStr += s[i];
            }
        }
    }
    ss.clear();
    ss << newStr;
    int ret;
    ss >> ret;
    cout<<"ret: "<<ret<<endl;
    return ret;
}

/**
 * getFloat function converts a float value back to its normal value without added character
 *
 * @param num the decimal value in string form
 * @param colName The column name it belongs to
 *
 * @return the new value
 */
double getFloat(double num, string colName) {
    bool side = true;   //GET FROM SECRET DB USING COLNAME
    int distance = 1;   //GET FROM SECRET DB USING COLNAME
    double ret = 0;
    if(side){
        double leftPart = (int)num / (int)pow(10, distance + 1);
        ret += leftPart * pow(10, distance) + fmod(num, pow(10, distance));        
    }else{
        double rightPart = fmod(num, pow(10, -1 * (distance + 1)));
        double rightSub = fmod(num, pow(10, -1 * (distance)));
        double leftPart = num - rightSub;
        ret += leftPart + rightPart * pow(10, 1);
    }
    return ret;
}

/**
 * getString function converts a string value back to its normal value without added character
 *
 * @param val the string value in string form
 * @param colName the column name it belongs to
 *
 * @return the new value
 */
string getString(string val, string colName) {
    bool side = true;   //GET FROM SECRET DB USING COLNAME
    int distance = 1;   //GET FROM SECRET DB USING COLNAME
    int len = val.size();
    int realLen=len;
    while(val[realLen-1]==' '){
        realLen--;
    }
    string newStr = "";
    if (len > distance) {
        for (int i = 0; i < realLen; i++) {
            if(side){
                if (i != distance) {
                    newStr += val[i];
                }
            }else{
                if (i != realLen-distance) {
                    newStr += val[i];
                }
            }
        }
    }else {
        for (int i = 0; i < realLen; i++) {
            if (i != len-1) {
                newStr += val[i];
            }
        }
    }
    return newStr;
}
/**
 * selectInt function helps getSelectInfo function with integers(column indices)
 *
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 *
 * @return the new source/table name where the select data is stored without extra characters
 */
string selectInt(vector<string> fieldVec, vector<string> sourceVec) {
    string newVal = sourceVec[0];
    return newVal + "_secret";
}

/**
 * selectFloat function helps getSelectInfo function with floats(column indices?)
 *
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 *
 * @return the new source/table name where the select data is stored without extra characters
 */
string selectFloat(vector<string> fieldVec, vector<string> sourceVec) {
    string newVal = sourceVec[0];
    return newVal + "_secret";
}

/**
 * selectString function helps getSelectInfo function with strings(column names)
 *
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 *
 * @return the new source/table name where the select data is stored without extra characters
 */
string selectString(vector<string> fieldVec, vector<string> sourceVec) {
    string newVal = sourceVec[0];
    return newVal + "_secret";
}

/**
 * selectStar function helps getSelectInfo function with *(all)
 *
 * @param fieldVec vector of the different fields we are selecting from in string form
 * @param sourceVec vector of the different sources we are selecting from(usually table name)
 *
 * @return the new source/table name where the select data is stored without extra characters
 */
string selectStar(vector<string> sourceVec) {
    string newVal = sourceVec[0];
    return newVal + "_secret";
}

/**
 * recreateSelect function recreates the select sql string with updated sources
 *
 * @param fieldVec vector of strings of the fields
 * @param sourceVec vector of string of the new sources
 *
 * @return the new sql query
 */
string recreateSelect(vector<string> fieldVec, vector<string> sourceVec, string end) {
    string fields = "";
    string sources = "";
    for (int i = 0; i < fieldVec.size(); i++) {
        if (i != fieldVec.size() - 1) {
            fields += fieldVec[i] + ", ";
        }
        else {
            fields += fieldVec[i];
        }
    }
    sources = sourceVec[0];
    /* for (int i = 0; i < sourceVec.size(); i++) {
        if (i != sourceVec.size() - 1) {
            sources += sourceVec[i] + ", ";
        }
        else {
            sources += sourceVec[i];
        }
    } */
/* 
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    string tableN = sourceVec[0];
    vector<string> colTypes;//Fill this by checking the types in secret_metadata
    vector<string> colNames;
    result cTypes{txn.exec("SELECT data FROM metadata where table_name = '"+tableN+"';")};
    Document d2;
    for(auto const &row: cTypes){
        field const field = row[0];
        d2.Parse(field.c_str());
    }
    int i=0;
    cout<<d2["cols"].GetArray()[i]["data_type"].GetString()<<endl;
    for (auto& v : d2["cols"].GetArray()) {
        colTypes.push_back(d2["cols"].GetArray()[i]["data_type"].GetString());
        colNames.push_back(d2["cols"].GetArray()[i]["name"].GetString());
        i++;
    }
    string colNs = "(";
    for(int i=0; i<colNames.size(); i++){
        if(i==colNames.size()-1){
            colNs+=colNames[i]+")";
        }else{
            colNs+=colNames[i]+", ";
        }
    }
    Document d;
    i = 0;
    result r{txn.exec("SELECT * FROM "+tableN+";")};
    txn.exec("create table if not exists "+tableN+"_temp as table "+tableN+" with no data;");
    txn.commit();
    work tx{c};
    string dataVec="";
    for (auto row = r.begin(); row != r.end(); row++){
        int count=0;
        for(auto field = row.begin(); field!=row.end(); field++){
            if(colTypes[i]=="int"||colTypes[i]=="bigint"){
                if(count!=row.size()-1){
                    dataVec += to_string(getInt(field->c_str(), tableN));
                    dataVec+=", ";
                }else{
                    dataVec += to_string(getInt(field->c_str(), tableN));
                }
                count++;
            }else if(colTypes[i]=="varchar"){
                if(count!=row.size()-1){
                    dataVec +=  getString(field->c_str(), tableN);
                    dataVec+=", ";
                }else{
                    dataVec +=  getString(field->c_str(), tableN);
                }
                count++;
            }else if(colTypes[i]=="float"){
                if(count!=row.size()-1){
                    dataVec += to_string(getFloat(field.as<double>(), tableN));
                    dataVec+=", ";
                }else{
                    dataVec +=  to_string(getFloat(field.as<double>(), tableN));
                }
                count++;
            }
        }
        tx.exec("insert into "+tableN+"_temp"+colNs+" values("+dataVec+");");
        dataVec="";
    }
    tx.commit(); */
    //work dlt{c};
    //dlt.exec0("delete")
    string newQuery = "SELECT " + fields + " FROM temp "+end+";";
    return newQuery;
}

/**
 * getSelectInfo function helps the getInfo function
 * Finds the fields and sources, modifies values and sends them to the recreateSelect function
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getSelectInfo(SelectStatement* stmt) {
    vector<string> fieldVec;//Vector of the field names
    vector<string> sourceVec;//Vector of the source names
    vector<int> colTypes;//Vector of the select types
    printSelectStatementInfo(stmt, 0);
    float fvalue;
    int ivalue;
    string svalue;
    int limit=-1;
    int offset=-1;
    string end="";

    if (stmt->fromTable != nullptr) {//Get the source of where we are selecting from
        sourceVec.push_back(stmt->fromTable->name);
    }

/*     if (stmt->order != nullptr) {
      inprint("OrderBy:", numIndent + 1);
      printOrderBy(stmt->order, numIndent + 2);
    }
 */
    if (stmt->limit != nullptr && stmt->limit->limit != nullptr) {//Check if limit
      cout<<"Limit: "+to_string(stmt->limit->limit->ival) <<endl;
      limit = stmt->limit->limit->ival;
    }

    if (stmt->limit != nullptr && stmt->limit->offset != nullptr) {//Check if offset
      cout<<"Offset: "+ to_string(stmt->limit->offset->ival) <<endl;
      offset = stmt->limit->offset->ival;
    }

    if(offset!=-1||limit!=-1){
        if(offset!=-1&&limit!=-1){
            end+="limit "+to_string(limit)+" offset "+to_string(offset);
        }
    }

    string tableN = sourceVec[0];//Temporary, I'm not sure why this wouldnt work yet
    for (Expr* val : *stmt->selectList) {//Get values, prepare them for selection
        switch (val->type) {//Check type of value we are selecting from
        case kExprLiteralFloat://Float
            fvalue = val->fval;
            fieldVec.push_back(to_string(fvalue));
            colTypes.push_back(0);
            break;
        case kExprLiteralInt://Integer
            ivalue = val->ival;
            //cout << ivalue << endl;
            fieldVec.push_back(to_string(ivalue));
            colTypes.push_back(1);
            break;
        case kExprLiteralString://String
            svalue = val->name;
            //cout << svalue << endl;
            fieldVec.push_back(svalue);
            colTypes.push_back(2);
            break;
        case kExprStar://All
            fieldVec.push_back("*");
            colTypes.push_back(3);
            break;
        case kExprColumnRef://Column Reference(AKA column name)
            svalue = val->name;
            //cout << svalue << endl;
            fieldVec.push_back(svalue);
            colTypes.push_back(2);
            break;
        default:
            break;
        }
    }
    vector<string> newSource;//Vector of new sources we want the user to select from(ONLY CHANGES TABLENAME RIGHT NOW)
    for (int i = 0; i < sourceVec.size(); i++) {//Select relevant data, make new table where extra character is removed.
        if (colTypes[i] == 0) {//Float(Not sure when this would be used)
            //newSource.push_back(selectFloat(fieldVec, sourceVec));
        }
        else if (colTypes[i] == 1) {//Integer(Column Index)
            //newSource.push_back(selectInt(fieldVec, sourceVec));
        }
        else if (colTypes[i] == 2) {//String(Column Name)
            //newSource.push_back(selectString(fieldVec, sourceVec));
        }
        else if (colTypes[i] == 3) {//*
            //newSource.push_back(selectStar(sourceVec));//Don't even need to send the * string
        }
    }
    //string newTable = newSource[0];
    return recreateSelect(fieldVec, sourceVec, end);//Create new sql string to send back
}
//END OF SELECT STUFF --------------------------------------------------------------------------------------------------

/**
 * getInfo function
 * Returns the columns, types, and values if required
 *
 * @param query the sql statement in question
 * @return Array of cols, types, values
 */
string getInfo(const SQLStatement* sql) {
    InsertStatement* insState;//The different statement types for grabbing more info
    CreateStatement* creState;
    SelectStatement* selState;
    ImportStatement* impState;
    ExportStatement* expState;
    TransactionStatement* tranState;//The different statement types for grabbing more info

    switch (sql->type()) {
    case(kStmtUpdate): //Update
        cout << "UPDATE" << endl;
        return "NO";
        break;
    case(kStmtDrop): //DROP
        cout << "DROP" << endl;
        return "NO";
        break;
    case(kStmtCreate): //Create (Big problems here with datatypes)
        cout << "CREATE" << endl;
        return "NO";//return "NO" to show it is a create and can't be parsed by hyrise
        break;
    case(kStmtPrepare): //Prepare
        cout << "PREPARE" << endl;
        break;
    case kStmtDelete: //Delete
        cout << "DELETE" << endl;
        return "NO";
        break;
    case kStmtSelect: //Select
        cout << "SELECT" << endl;
        selState = (SelectStatement*)sql;
        return getSelectInfo(selState);
        break;
    case kStmtInsert: //Insert
        cout << "INSERT" << endl;
        insState = (InsertStatement*)sql;
        return getInsertInfo(insState);
        break;
    case(kStmtExecute): //Execute
        cout << "EXECUTE" << endl;
        break;
    case(kStmtRename): //Rename
        cout << "RENAME" << endl;
        break;
    case(kStmtAlter): //Alter
        cout << "ALTER" << endl;
        return "NO";
        break;
    case(kStmtShow): //Show
        cout << "SHOW" << endl;
        break;
    case(kStmtTransaction)://Transaction
        cout << "TRANSACTION" << endl;
        break;
    default:
        cout << "NO TYPE FOUND" << endl;
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
string recreateCreate(string tableN, string leftSide, vector<string> colNames, vector<string> dataTypes) {
    string rightSide = "";
    for (int i = 0; i < colNames.size(); i++) {
        if (i != colNames.size() - 1) {
            rightSide += colNames[i] + " " + dataTypes[i] + ", ";
        }else {
            rightSide += colNames[i] + " " + dataTypes[i] + ");";
        }
    }

    //Add table into the json
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    string json_col = "";
    string json_cols = "";

    bool overflowBool = false; //Replace with shared function
    string overflowStr;
    if(overflowBool){
        overflowStr = "true";
    }else{
        overflowStr="false";
    }
    int insert_loc = 2; //Replace with shared function
    bool leftBool = false;//Replace with shared function
    string leftStr;
    if(leftBool){
        leftStr = "true";
    }else{
        leftStr="false";
    }
    for(int j=0; j<colNames.size(); j++){
        if(j!=colNames.size()-1){
            json_col = "{\"overflow\": " + overflowStr + ", \"insert_loc\": " + to_string(insert_loc) + ", \"left\": " + leftStr + ", \"data_type\": \"" + dataTypes[j] + "\", \"name\": \"" + colNames[j] + "\"}, ";
        }else{
            json_col = "{\"overflow\": " + overflowStr + ", \"insert_loc\": " + to_string(insert_loc) + ", \"left\": " + leftStr + ", \"data_type\": \"" + dataTypes[j] + "\", \"name\": \"" + colNames[j] + "\"}";
        }
        json_cols+=json_col;

    }
    int overlap = 4; //Replace with shared function
    int bubble = 2; //Replace with shared function
    cout<<"executing"<<endl;
    string jsonStr = "{ \"cols\": ["+json_cols+"], \"overlap\": " + to_string(overlap) + ", \"bubble\": " + to_string(bubble) +"}";
    txn.exec0("insert into metadata (table_name, data) values( '" + tableN + "', '" + jsonStr + "');");
    txn.commit();

    return leftSide + rightSide;
}

/**
 * convertDatatypes function converts the datatypes in create table statement to next highest
 *
 * @param dTs vector of datatypes from sql statement
 *
 * @return vector of new datatypes
 */
vector<string> convertDatatypes(vector<string> dTs) {
    vector<string> newVec;
    for (string dt : dTs) {
        if (dt == "smallint") {
            newVec.push_back("int");
        }
        else if (dt == "int") {
            newVec.push_back("bigint");
        }
        else if (dt == "bigint") {
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
string getCreateInfo(string stmt) {//Can't have a space after the comma...

    vector<string> colNames;
    vector<string> dataTypes;
    bool nameB = true; //Check if reading colname
    bool typeB = false; //Check if reading coltype
    string name = "";
    string type = "";
    int maxLen = stmt.length();
    int end = stmt.find('(');//error check if not found
    string newStmt = stmt.substr(0, end + 1);
    string colData = stmt.substr(end, maxLen);
    string tableN = newStmt.substr(13, end-13);
    for (int i = 1; i < colData.length(); i++) {
        if (colData[i] == ' ') {
            colNames.push_back(name);
            name = "";
            typeB = true;
            nameB = false;
        }
        else if (colData[i] == ',') {
            dataTypes.push_back(type);
            type = "";
            typeB = false;
            nameB = true;
        }
        else if (colData[i] == ')') {
            dataTypes.push_back(type);
            type = "";
            typeB = false;
            nameB = true;
            break;
        }
        else {
            if (typeB) {
                type += colData[i];
            }
            else {
                name += colData[i];
            }
        }
    }
    //Send to function to increase the scale by 1 of each data type
    return recreateCreate(tableN, newStmt, colNames, convertDatatypes(dataTypes));//Create and return new sql string
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
string recreateDropTable(string tableN) {
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    txn.exec0("delete from metadata where table_name = '"+tableN +"';");
    txn.commit();
    return "DROP TABLE " + tableN + ";";
}
/**
 * recreateDropDatabase function recreates the drop database sql string with updated data if necesary(not yet)
 *
 * @param dbN name of database being dropped
 *
 * @return the new sql query
 */
string recreateDropDatabase(string dbN) {
    return "DROP DATABASE " + dbN + ";";
}

/**
 * getDropInfo function helps the getInfoString function
 * Finds the table name or database name, removes them from our data, and sends them to the recreateDropTable or Database function
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getDropInfo(string stmt) {
    int maxLen = stmt.size();
    int sz;
    if (stmt.substr(5, 5) == "TABLE") {
        cout << "DROP TABLE" << endl;
        sz = maxLen - 12;
        string tableN = stmt.substr(11, sz);//The table name we are dropping
        //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
        return recreateDropTable(tableN);
    }
    else if (stmt.substr(5, 8) == "DATABASE") {
        cout << "DROP DATABASE" << endl;
        sz = maxLen - 15;
        string dbN = stmt.substr(14, sz);//The table name we are dropping
        //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
        return recreateDropDatabase(dbN);
    }
    else {
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
string recreateDeleteTable(string tableN) {
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    StringBuffer json_str_tbl;

    result select_col{ txn.exec("select data from metadata where table_name="+tableN+";") }; //get the table names, and column associated
    Document d;
    for(auto const &row: select_col){
        field const field = row[0];
        d.Parse(field.c_str());
    }
    int i=0;
    for (auto& v : d["cols"].GetArray()) {
        d["cols"].GetArray()[i].RemoveMember("hashes");
        i++;
        Writer<StringBuffer> writer(json_str_tbl);
        d.Accept(writer);
        string output = json_str_tbl.GetString();
        txn.exec0("update metadata set data = '" + output + "';");
        json_str_tbl.Clear();
    }
    txn.commit();
    
    return "DELETE FROM " + tableN + ";";
}

/**
 * getDeleteInfo function helps the getInfoString function
 * Finds the delete info, deletes and rehashes using temp tables and sends them to the recreateDelete functions
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getDeleteInfo(string stmt) {
    int maxLen = stmt.size();
    int sz;
    if (stmt.substr(7, 4) == "FROM") {
        cout << "DELETE FROM" << endl;
        sz = maxLen - 13;
        string tableN = stmt.substr(12, sz);//The table name we are dropping
        return recreateDeleteTable(tableN);
    }
    else {
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
string getTruncateInfo(string stmt) {
    int maxLen = stmt.size();
    int sz;
    cout << "TRUNCATE" << endl;
    sz = maxLen - 10;
    string tableN = stmt.substr(9, sz);//The table name we are dropping

    //HERE WE CHECK OUR SECRET DB FOR DATA WITH THE TABLE NAME AND DELETE IT
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    StringBuffer json_str_tbl;

    result select_col{ txn.exec("select data from metadata where table_name="+tableN+";") }; //get the table names, and column associated
    Document d;
    for(auto const &row: select_col){
        field const field = row[0];
        d.Parse(field.c_str());
    }
    int i=0;
    for (auto& v : d["cols"].GetArray()) {
        d["cols"].GetArray()[i].RemoveMember("hashes");
        i++;
        Writer<StringBuffer> writer(json_str_tbl);
        d.Accept(writer);
        string output = json_str_tbl.GetString();
        txn.exec0("update metadata set data = '" + output + "';");
        json_str_tbl.Clear();
    }
    txn.commit();
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
string recreateUpdateTable(string tableN, string setCol, string setVal, string whereCol, string whereVal) {
    //Gotta do all the select statements to get the real values
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    vector<string> colTypes;//Fill this by checking the types in secret_metadata
    vector<string> colNames;
    result cTypes{txn.exec("SELECT data FROM metadata where table_name = '"+tableN+"';")};
    Document d2;
    for(auto const &row: cTypes){
        field const field = row[0];
        d2.Parse(field.c_str());
    }
    int i=0;
    cout<<d2["cols"].GetArray()[i]["data_type"].GetString()<<endl;
    for (auto& v : d2["cols"].GetArray()) {
        colTypes.push_back(d2["cols"].GetArray()[i]["data_type"].GetString());
        colNames.push_back(d2["cols"].GetArray()[i]["name"].GetString());
        i++;
    }
    string colNs = "(";
    for(int i=0; i<colNames.size(); i++){
        if(i==colNames.size()-1){
            colNs+=colNames[i]+")";
        }else{
            colNs+=colNames[i]+", ";
        }
    }
    Document d;
    i = 0;
    result r{txn.exec("SELECT * FROM "+tableN+";")};
    txn.exec("create table if not exists "+tableN+"_temp as table "+tableN+" with no data;");//Create 
    txn.commit();
    return "UPDATE " + tableN + " SET " + setCol + "=" + setVal + " WHERE " + whereCol + "=" + whereVal + ";";
}

/**
 * getUpdateInfo function helps the getInfoString function
 * Finds the delete info, deletes and rehashes using temp tables and sends them to the recreateDelete functions
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getUpdateInfo(string stmt) {
    int maxLen = stmt.size();
    int set = stmt.find("SET") + 4;
    int where = stmt.find("WHERE");
    string tableN = stmt.substr(7, set - 12);
    string setCol;
    string setVal;
    string whereCol;
    string whereVal;
    int setEq = stmt.find("=");
    string setStr;
    string whereStr;
    if (where >= 0) {
        setStr = stmt.substr(set, where - set - 1);
        whereStr = stmt.substr(where + 6, maxLen - where - 7);
        int whereEq = whereStr.find("=");
        whereCol = whereStr.substr(0, whereEq);
        whereVal = whereStr.substr(whereEq + 1, whereStr.size() - whereEq);
    }
    else {
        setStr = stmt.substr(set, maxLen - set - 1);
    }
    setEq = setStr.find("=");
    setCol = setStr.substr(0, setEq);
    setVal = setStr.substr(setEq + 1, setStr.size() - setEq);
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
string recreateAlterAddTable(string tableN, string colN, string colType) {
    //Add the new col into secret data
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    StringBuffer json_str_tbl;

    result select_col{ txn.exec("select data from metadata where table_name="+tableN+";") }; //get the table names, and column associated
    Document d;
    for(auto const &row: select_col){
        field const field = row[0];
        d.Parse(field.c_str());
    }
    Value newCol;
    newCol.SetObject();

    bool overflowBool = false; //Replace with shared function
    int insert_loc = 2; //Replace with shared function
    bool leftBool = false;//Replace with shared function
    int tl = colType.length();
    int cl = colN.length();
    char charT[tl+1];
    char charC[cl+1];
    strcpy(charT, colType.c_str());
    strcpy(charC, colN.c_str());
    Value colName;
    colName.SetString(StringRef(charC));
    Value colT;
    colT.SetString(StringRef(charT));

    newCol.AddMember("overflow", overflowBool, d.GetAllocator());
    newCol.AddMember("insert_loc", insert_loc, d.GetAllocator());
    newCol.AddMember("left", leftBool, d.GetAllocator());
    newCol.AddMember("data_type", colT, d.GetAllocator());
    newCol.AddMember("name", colName, d.GetAllocator());
    d["cols"].PushBack(newCol, d.GetAllocator());
    Writer<StringBuffer> writer(json_str_tbl);
    d.Accept(writer);
    string output = json_str_tbl.GetString();
    txn.exec0("update metadata set data = '" + output + "';");
    json_str_tbl.Clear();
    txn.commit();

    return "ALTER TABLE " + tableN + " ADD " + colN + " " + colType + ";";
}

/**
 * recreateAlterDropTable function recreates the alter add sql string with updated datatype
 *
 * @param tableN name of table being deleted from
 * @param colN name of column being added
 *
 * @return the new sql query
 */
string recreateAlterDropTable(string tableN, string colN) {
    //Delete the dropped column from secret data
    connection c{SECRET_DB_HANDLE};
    work txn{c};
    StringBuffer json_str_tbl;

    result select_col{ txn.exec("select data from metadata where table_name="+tableN+";") }; //get the table names, and column associated
    Document d;
    for(auto const &row: select_col){
        field const field = row[0];
        d.Parse(field.c_str());
    }
    Value newCol;
    newCol.SetObject();

    int cl = colN.length();
    char charC[cl+1];
    strcpy(charC, colN.c_str());
    Value colName;
    colName.SetString(StringRef(charC));
    Value& items = d["cols"];
    for(int i=0; i<(int)items.Size(); i++){
        if(items[i]["name"].GetString()==colName){
            d["cols"].Erase(d["cols"].Begin()+i);
        }
    }
    Writer<StringBuffer> writer(json_str_tbl);
    d.Accept(writer);
    string output = json_str_tbl.GetString();
    txn.exec0("update metadata set data = '" + output + "';");
    json_str_tbl.Clear();
    txn.commit();

    return "ALTER TABLE " + tableN + " DROP COLUMN " + colN + ";";
}

/**
 * getAlterInfo function helps the getInfoString function
 * Finds the tableName, columnName, and columnType of ADD alter statements
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getAlterInfo(string stmt) {
    string tableN;
    string colN;
    string colType;
    vector<string> dataType;
    int maxLen = stmt.size();
    int add = stmt.find("ADD");
    int drop = stmt.find("DROP");
    if (add >= 0) {
        tableN = stmt.substr(12, add - 13);
        bool first = false;
        for (int i = add + 4; i < maxLen - 1; i++) {
            if (stmt[i] == ' ') {
                first = true;
                continue;
            }
            if (!first) {
                colN += stmt[i];
            }
            else {
                colType += stmt[i];
            }
        }
        dataType.push_back(colType);
        return recreateAlterAddTable(tableN, colN, convertDatatypes(dataType)[0]);
    }
    else {
        tableN = stmt.substr(12, drop - 13);
        colN = stmt.substr(drop + 12, maxLen - drop - 13);
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
string getInfoString(string query) {
    //string newQuery = query;
    if (query.substr(0, 6) == "CREATE") {
      return getCreateInfo(query);
    }
    else if (query.substr(0, 4) == "DROP") {
        return getDropInfo(query);
    }
    else if (query.substr(0, 6) == "DELETE") {
        return getDeleteInfo(query);
    }
    else if (query.substr(0, 6) == "UPDATE") {
        return getUpdateInfo(query);
    }
    else if (query.substr(0, 8) == "TRUNCATE") {
        return getTruncateInfo(query);
    }
    else if (query.substr(0, 5) == "ALTER") {
        return getAlterInfo(query);
    }
    else {
      cout<<"NONE"<<endl;
        return "";
    }
    //return newQuery;
}

/**
 * parseString function
 *
 * @param query the entire sql statement
 * @return the new entire sql statement
 */
string parseString(string query) {//Maybe check every semicolon then go from there
  // parse a given query

    SQLParserResult result;
    SQLParser::parse(query, &result);

    string newQuery; //THIS IS WHAT WE RETURN TO THE SHIM

    // check whether the parsing was successful
    if (false) {//result.isValid() put this back maybe
        //printf("Parsed successfully!\n");
        //printf("Number of statements: %lu\n", result.size());
        for (auto i = 0u; i < result.size(); ++i) {
            printStatementInfo(result.getStatement(i));
            newQuery += getInfo(result.getStatement(i));//Send new sql string into vector one by one
        }
    }
    else {//The parser does not recognize something so we need to go one by one
        int maxLen = query.size() - 1;
        if (query[maxLen] != ';') {//If the string doesn't end with ; then it is bad
            //cout << "Error: BAD SQL STRING." << endl;
            return "";
        }
        printf("Error, checking one by one now.\n");
        int start = 0;
        int end = query.find(';') + 1;
        string stmt;
        while (query != "") {
            stmt = query.substr(start, end);
            query = query.substr(end, maxLen);
            //cout << "Manually Parsing: " << stmt << endl;

            //Try to parse each individual statement normally
            SQLParserResult result;
            SQLParser::parse(stmt, &result);
            if (result.isValid()) {
                //printf("Parsed successfully!\n");
                //printf("Number of statements: %lu\n", result.size());
                for (auto i = 0u; i < result.size(); ++i) {
                    string temp = getInfo(result.getStatement(i));
                    if (temp == "NO") {//If it returns NO it means it should not be parsed by hyrise
                        newQuery += getInfoString(stmt);
                    }
                    else {
                        newQuery += temp;//Send new sql string into vector one by one
                    }
                }
            }
            else {
                //cout << "The statement: " << stmt << " is being manually parsed.\n";
                string temp = getInfoString(stmt);
                if (temp == "") {//Return "" if the query is unparseable
                    //cout << "ERROR: " << stmt << " is not a valid statement." << endl;
                    return "";
                }
                newQuery += temp;//Send this new sql string into vector
            }
            end = query.find(';') + 1;
            maxLen = query.size();
        }
    }
    //cout << newQuery << endl;//Print out the final new query
    return newQuery;
}
const map<string, string> UPSIZE_TYPES{
    {"integer", "bigint"},
    {"bigint","numeric(20)"}, //scale is how many significant digits it can have, upsizing by 1
    {"smallint","integer"},
    {"real", "double precision"},
    {"numeric", "numeric"},
    {"varchar", "varchar"}
}; //numeric precision doesnt exist sometime, need a default

const vector<string> OVERFLOW_TYPES{
    "numeric",
    "decimal"
};

const map<string, int> SIZE_TYPES{ //TODO: get right sizes for each type
    {"smallint", 5}, // -32768 to +32767
    {"integer", 10}, // -2147483648 to +2147483647
    {"bigint", 19}, // -9223372036854775808 to +9223372036854775807
    {"numeric", 10}, // arbitrary for now
    {"real", 0}, //TODO: not yet implemented
    {"double precision", 0}, //TODO: not yet implemented
    {"text", 30} //arbitrary num for now
};

string UpsizeAlterCol(const string& name, const string& type) {//optimization would be to query the max length as well instead of parsing here
    //string builder = " alter column " + name + " type ";
    string var = "";
    if (UPSIZE_TYPES.find(type) == UPSIZE_TYPES.end()) { //build alter string if possible, prepare for weird types here ie varchar
        int size;
        int place_holder;
        if (sscanf(type.c_str(), "varchar(%d)", &size)) { //for varying characters
            ++size;
            var += "varchar(" + to_string(size) + ")";
        }else if (sscanf(type.c_str(), "char(%d)", &size)) { //for characters
            ++size;
            var += "char(" + to_string(size) + ")";
        }
        else if (sscanf(type.c_str(), "numeric(%d, %d)", &size, &place_holder)) { //TODO: OVERFLOWS
            if (size == 1000) {
                return var + "numeric(" + to_string(size) + "," + to_string(place_holder) + ")";
            }
            ++size;
            var += "numeric(" + to_string(size) + "," + to_string(place_holder) + ")";
        }
        else {
            return " ";
        }
        return var + ",";
    }
    return UPSIZE_TYPES.at(type); //if type exists in the hashmap return the next largest type
}

string startParse(string sql) {
    //syslog(LOG_INFO, "Parsing string: ");//add the sql query to the log
    cout<<UpsizeAlterCol("name", "integer")<<endl;
    return "";
    //return parseString(sql);
}

int getInsertDirection(char* table, char* col_name){
    Document d;
    ifstream file("secret.json");
    IStreamWrapper isw{ file };
    d.ParseStream(isw);
    Value colN;
    colN.SetString(StringRef(col_name));
    Value tableN;
    tableN.SetString(StringRef(table));
    //d["tables"].GetArray();
    for(auto& v: d["tables"].GetArray()){
      if(v["table_name"].GetString()==tableN){
        return v[colN][1].GetInt();
      }
    }
    return -1;
}

int getInsertDistance(char* table, char* col_name){
    Document d;
    ifstream file("secret.json");
    IStreamWrapper isw{ file };
    d.ParseStream(isw);
    Value colN;
    colN.SetString(StringRef(col_name));
    Value tableN;
    tableN.SetString(StringRef(table));
    d.AddMember("name", true, d.GetAllocator());
    //d["tables"].GetArray();
    for(auto& v: d["tables"].GetArray()){
      if(v["table_name"].GetString()==tableN){
        return v[colN][0].GetInt();
      }
    }
    return -1;
}

bool addThenRemove(char* table, char* col_name){
    Document d;
    StringBuffer json_str_tbl;
    StringBuffer json_str_tbl1;
    ifstream file("secret.json");
    IStreamWrapper isw{ file };
    d.ParseStream(isw);
    Value colN;
    colN.SetString(StringRef(col_name));
    Value tableN;
    tableN.SetString(StringRef(table));
    d.AddMember("name", true, d.GetAllocator());
    Writer<StringBuffer> writer(json_str_tbl);
    d.Accept(writer);
    //cout << json_str_tbl.GetString() << endl;
    json_str_tbl.Clear();
    d.RemoveMember("name");
    Writer<StringBuffer> writer1(json_str_tbl1);
    d.Accept(writer1);
    //cout << json_str_tbl1.GetString() << endl;
    return 0;
}

result selectAll(){
  connection c{SECRET_DB_HANDLE};
  work txn{c};

  result r{txn.exec("SELECT * FROM pretemp;")};
  for (auto row: r)
    cout << row["name"].c_str()<< " makes " << row[1].as<int>() << "." << endl;

  txn.commit();

  // Connection object goes out of scope here.  It closes automatically.
  return r;
}

enum InsertDirection { //insert direction can be either left or right
	left_ins,
	right_ins
};

struct ColInfo { //column information struct for inserting secret vals
	std::string name;
	bool is_text;
	bool is_overflow;
	int insert_location;
	enum InsertDirection insert_direction;
};
string GenerateRandomChar() {
    return "K";
}

/**
 * takes in a limit and generates a random number between
 * note: limit and zero is inclusive
 *
 * @param limit the upper bound of the data
 * @return int
 */
int GenerateRandomNum(int limit) {
    std::random_device r;
    default_random_engine eng{ r() };
    uniform_real_distribution<double> urd(0, limit);
    return urd(eng);
}
/**
 * Insert the secret character
 *
 * we insert into the string based on its type
 *
 * @param val the string value
 * @param ColInfo is struct containing: if its text, insert loc, overflow
 * @return the modified string
 */
string InsertSecret(const string& val, const ColInfo col_info) {
    //InsertDirection insert_direction = right_ins;
    string cp = val;
    int sz = cp.size();
    if (col_info.is_text) {
        string random_char = GenerateRandomChar();
        switch (col_info.insert_direction) {
        case right_ins:
            cp.insert((sz < col_info.insert_location) ? (sz) : (col_info.insert_location), random_char);//insert right!
            break;
        case left_ins:
            cp.insert((sz < col_info.insert_location) ? (0) : (sz - col_info.insert_location), random_char);//insert left
            break;
        default:
            break;
        }
        return "'" + cp + "'";
    }
    string random_num = to_string(GenerateRandomNum(9)); //0-9 digit
    switch (col_info.insert_direction) {
    case right_ins:
        cp.insert((sz < col_info.insert_location) ? (sz) : (col_info.insert_location), random_num);//insert right!
        break;
    case left_ins:
        cp.insert((sz < col_info.insert_location) ? (0) : (sz - col_info.insert_location), random_num);//insert left
        break;
    default:
        break;
    }
    return cp; //return val, inserted
}
bool IsTextType(const string& type) {
    int t;//i just need to check the format is right varchar(%d)
    if (sscanf(type.c_str(), "varchar(%d)", &t) || sscanf(type.c_str(), "char(%d)", &t)) {
        return true;
    }
    if (type.compare("text") == 0) {
        return true;
    }
    return false;
}

/**
 * getInsertInfo function helps the getInfo function
 * Finds the colNames, table name, and modifies values and sends them to the recreateInsert function
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
/* string getInsertInfoString(string stmt) {
    //printInsertStatementInfo(insState, 0);
    cout << "insert info" << endl;
    //INSERT INTO xxx() VALUES();
    int maxLen = stmt.length();
    int par1 = stmt.find("(");

    string tableName = stmt.substr(12,par1-12);
    int par2 = stmt.find(")");
    string cols = stmt.substr(par1+1, par2-par1-1);
    cout<<cols<<endl;
    string vals = stmt.substr(par2+9, maxLen-par2-11);
    cout<<vals<<endl;
    float fvalue;
    int ivalue;
    string svalue;

    vector<string> colNames;//Vector of the column names
    vector<string> colTypes;//Vector of the column types  
    vector<string> valVec;//Vector of the values in string form
    string tempCol = "";
    string tempVal = "";
    for(int i=0; i<cols.length(); i++){
        if(i==cols.length()-1){
            tempCol+=cols[i];
            colNames.push_back(tempCol);
        }
        if(cols[i]==','){
            colNames.push_back(tempCol);
            tempCol="";
        }else{
            tempCol+=cols[i];
        }
    }
    for(int i=0; i<vals.length(); i++){
        if(i==vals.length()-1){
            tempVal+=vals[i];
            valVec.push_back(tempVal);
        }
        if(vals[i]==','){
            valVec.push_back(tempVal);
            tempVal="";
        }else{
            tempVal+=vals[i];
        }
    }
    connection c{ secret_db_handle_ };
    work txn{ c };
    result select_col{ txn.exec("select data from metadata where table_name='" + tableN + "';") }; //get the table names, and column associated
    Document d;
    for (auto const& row : select_col) {
        field const field = row[0];
        d.Parse(field.c_str());
    }
    Value& item = d["cols"];
    for (int i = 0; i < (int)item.Size(); i++) {
        colTypes.push_back(d["cols"].GetArray()[i]["data_type"].GetString())                    
    }
    vector<string> newValues;
    for (int i = 0; i < colNames.size(); i++) {//Insert extra character into values
        InsertDirection dir = left_ins;
        ColInfo col{
            .name = "",
            .is_text = IsTextType(colTypes[i]),
            .is_overflow = IsOverflow(colTypes[i]),
            .insert_location = 2,
            .insert_direction = left_ins
        };
        newValues.push_back(InsertSecret(valVec[i], col));
    }
    
    //cout << "sending to recreate insert" << endl;
    return recreateInsert(newValues, colNames, tableName);//Create new sql string to send back
} */

/**
 * getSelectInfo function helps the getInfo function
 * Finds the fields and sources, modifies values and sends them to the recreateSelect function
 *
 * @param stmt the sql statement in question
 * @return new sql statement
 */
string getSelectInfoString(string stmt) {
    vector<string> fieldVec;//Vector of the field names
    vector<string> sourceVec;//Vector of the source names
    vector<int> colTypes;//Vector of the select types
    //SELECT * from table offset 0 limit 5;
    cout << "select info" << endl;

    int maxLen = stmt.length();
    bool semi = false;
    //Find sources
    int from = stmt.find("FROM");
    //Find fields
    int i=7;
    string fieldName="";
    while(stmt[i]!=' '){//Select statements cant have spaces in between sources or fields
        if(stmt[i]==','){
            fieldVec.push_back(fieldName);
            fieldName="";
        }else{
            fieldName+=stmt[i];
        }
        if(stmt[i+1]==' '){
            fieldVec.push_back(fieldName);
            break;
        }
        i++;
    }

    i=from+5;
    string tableName="";
    while(stmt[i]!=' '){//Select statements cant have spaces in between sources or fields
        if(stmt[i]==','){
            sourceVec.push_back(tableName);
            tableName="";
        }else{
            tableName+=stmt[i];
        }
        if(stmt[i+1]==' '||stmt[i+1]==';'){
            sourceVec.push_back(tableName);
            if(stmt[i+1]==';'){
                semi = true;
            }
            i++;
            break;
        }
        i++;
    }
    int offset = -1;
    int limit = -1;
    string rest = stmt.substr(i+1, maxLen-i);
    if(!semi){//If we havent reached the semi colon parse more
        string lims = "";
        if(rest.find("limit")!=string::npos){//Check for limits
            int lim = rest.find("limit");
            while(rest[lim+6]!=' '&&rest[lim+6]!=';'){
                lims+=rest[lim+6];
                lim++;
            }
            limit = stoi(lims);
        }
        
        string offs = "";
        if(rest.find("offset")!=string::npos){//Check for offsets
            int off = rest.find("offset");
            while(rest[off+7]!=' '&&rest[off+7]!=';'){
                offs+=rest[off+7];
                off++;
            }
            offset = stoi(offs);
        }
    }
    string end = "";
    if (offset != -1 || limit != -1) {//Compute end string
        if (offset != -1 && limit != -1) {
            end += "limit " + to_string(limit) + " offset " + to_string(offset);
        }
    }
   
    return recreateSelect(fieldVec, sourceVec, end);//Create new sql string to send back
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: ./example \"SELECT * FROM test;\"\n");
  }
  cout<<getSelectInfoString("SELECT *,stuff1,stuff2 FROM test limit 0 offset 50;")<<endl;
  //cout<<UpsizeAlterCol("name","numeric(12, 20)")<<endl;
  //cout<<startParse(argv[1])<<endl;
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
 */ 
