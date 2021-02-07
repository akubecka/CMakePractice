#include "hsql/SQLParser.h"
#include "hsql/util/sqlhelper.h"
#include <iostream>

using namespace std;
using namespace hsql;


bool dropTable(SQLParserResult *sql){
  cout<<sql->getStatement(0);
  return true;
}
bool createTable(SQLParserResult *sql){
  return true;
}
bool selectStatement(SQLParserResult *sql){
  return true;
}
bool insertStatement(SQLParserResult *sql){
  return true;
}


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
      int typ = result.getStatement(i)->type();
      switch(typ){
        case(1): //Select
          cout<<"SELECT"<<endl;
          selectStatement(&result);
          break;
        case(2):
          cout<<2<<endl;
          break;
        case(3): //Insert
          cout<<"INSERT"<<endl;
          insertStatement(&result);
          break;
        case(4):
          cout<<4<<endl;
          break;
        case(5):
          cout<<5<<endl;
          break;
        case(6): //Create (Big problems here with datatypes)
          cout<<"CREATE"<<endl;
          createTable(&result);
          break;
        case(7): //DROP
          cout<<"DROP"<<endl;
          dropTable(&result);
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