#include <cpprest/http_listener.h>
 
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
 
#include <iostream>
using namespace std;

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
using namespace rapidjson;

/* handlers implementation */

void handle_get1(const http_request& request)
{
   cout<<"getAll\n";
    char _id[20];
        
    StringBuffer JSONStrBuffer;
    Writer<StringBuffer> writer(JSONStrBuffer);
    writer.StartArray(); 

    for (int i = 0; i < 100; ++i) {
        writer.StartObject(); 
        snprintf(_id, sizeof(_id), "item-%d", i);
        writer.Key("id"); writer.String(_id);
        writer.Key("name"); writer.String("Hello World");
        writer.Key("type"); writer.String("application");
        writer.EndObject();
    }
    writer.EndArray();

    http_response response(status_codes::OK);
    response.headers().add("Connection", "keep-alive");
    response.set_body(JSONStrBuffer.GetString());

    request.reply(response);
}
void handle_get2(http_request message)
{
   cout<<"get1\n";
   char _id[20];
      
   StringBuffer JSONStrBuffer;
   Writer<StringBuffer> writer(JSONStrBuffer);
   writer.StartArray(); 

   for (int i = 0; i < 2; ++i) {
      writer.StartObject(); 
      snprintf(_id, sizeof(_id), "item-%d", i);
      writer.Key("id"); writer.String(_id);
      writer.Key("name"); writer.String("Hello World");
      writer.Key("type"); writer.String("application");
      writer.EndObject();
   }
   writer.EndArray();

   http_response response(status_codes::OK);
   response.headers().add("Connection", "keep-alive");
   response.set_body(JSONStrBuffer.GetString());

   message.reply(response);
}
 
int main()
{
   //Might need to use client here not listener?
   http_request req;
   http_listener listener1("http://127.0.0.1:9000/getall");
   listener1.support(methods::GET, handle_get1);
   http_listener listener2("http://127.0.0.1:9000/get1");
   listener2.support(methods::GET, handle_get2);

   try {
      listener1
        .open()
        .wait();
      listener2
         .open()
         .wait();

      while (true);
   } catch (exception const & e) {
      cout << e.what() << endl;
   }

   listener1
    .close()
    .wait();
   listener2
    .close()
    .wait();

   return 0;
}