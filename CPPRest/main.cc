// #include <cpprest/http_listener.h>
 
// using namespace web;
// using namespace web::http;
// using namespace web::http::experimental::listener;
 
// #include <iostream>
// using namespace std;

// #include <rapidjson/writer.h>
// #include <rapidjson/stringbuffer.h>
// using namespace rapidjson;

// /* handlers implementation */

// void handle_get1(const http_request& request)
// {
//    cout<<"getAll\n";
//     char _id[20];
        
//     StringBuffer JSONStrBuffer;
//     Writer<StringBuffer> writer(JSONStrBuffer);
//     writer.StartArray(); 

//     for (int i = 0; i < 100; ++i) {
//         writer.StartObject(); 
//         snprintf(_id, sizeof(_id), "item-%d", i);
//         writer.Key("id"); writer.String(_id);
//         writer.Key("name"); writer.String("Hello World");
//         writer.Key("type"); writer.String("application");
//         writer.EndObject();
//     }
//     writer.EndArray();

//     http_response response(status_codes::OK);
//     response.headers().add("Connection", "keep-alive");
//     response.set_body(JSONStrBuffer.GetString());

//     request.reply(response);
// }
// void handle_get2(http_request message)
// {
//    cout<<"get1\n";
//    char _id[20];
      
//    StringBuffer JSONStrBuffer;
//    Writer<StringBuffer> writer(JSONStrBuffer);
//    writer.StartArray(); 

//    for (int i = 0; i < 2; ++i) {
//       writer.StartObject(); 
//       snprintf(_id, sizeof(_id), "item-%d", i);
//       writer.Key("id"); writer.String(_id);
//       writer.Key("name"); writer.String("Hello World");
//       writer.Key("type"); writer.String("application");
//       writer.EndObject();
//    }
//    writer.EndArray();

//    http_response response(status_codes::OK);
//    response.headers().add("Connection", "keep-alive");
//    response.set_body(JSONStrBuffer.GetString());

//    message.reply(response);
// }
 
// int main()
// {
//    //Might need to use client here not listener?
//    http_request req;
//    http_listener listener1("http://127.0.0.1:9000/getall");
//    listener1.support(methods::GET, handle_get1);
//    http_listener listener2("http://127.0.0.1:9000/get1");
//    listener2.support(methods::GET, handle_get2);

//    try {
//       listener1
//         .open()
//         .wait();
//       listener2
//          .open()
//          .wait();

//       while (true);
//    } catch (exception const & e) {
//       cout << e.what() << endl;
//    }

//    listener1
//     .close()
//     .wait();
//    listener2
//     .close()
//     .wait();

//    return 0;
// }

// 
// SQL Handler REST Server (Test)
//
// Credit: Marius Bancila (https://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/)
// Note: needed to change wcout to cout and remove any reference to Long (L).
//
// Example 1 (send a SQL command): curl --header "Content-Type: application/json" -X POST --data '{"cmd" : "INSERT INTO Students (Name, City) VALUES ('Rob', 'Hoboken');"}' localhost:9080/tf/sqlhandler
// Example 2 (fetch TF statistics): curl -X GET localhost:9080/tf/sqlhandler

#include <cpprest/http_listener.h>
#include <cpprest/json.h>


using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#include <iostream>
#include <map>
#include <set>
#include <string>
using namespace std;


#define TRACE(msg)            cout << msg
#define TRACE_ACTION(a, k, v) cout << a << " (" <<  k << ", " << v << ")\n"

static int sql_cmd_count    = 0;       // total number of SQL commands received
static int parse_error_count = 0;
static int process_error_count = 0;
static auto start_time = chrono::steady_clock::now();

map<utility::string_t, utility::string_t> dictionary;

void display_json(
    json::value const& jvalue,
    utility::string_t const& prefix)
{
    std::cout << prefix << jvalue.serialize() << endl;
}

void handle_get(http_request request)
{
    auto answer = json::value::object();

    answer["sql_cmd_count"]         = sql_cmd_count;
    answer["parse_error_count"]     = parse_error_count;
    answer["process_error_count"]   = process_error_count;
    auto end = chrono::steady_clock::now();
    answer["uptime_secs"]           = chrono::duration_cast<chrono::seconds>(end - start_time).count();

    request.reply(status_codes::OK, answer);
}

void handle_request(
    http_request request,
    function<void(json::value const&, json::value&)> action)
{
    auto answer = json::value::object();

    request
        .extract_json()
        .then([&answer, &action](pplx::task<json::value> task) {
        try
        {
            auto const& jvalue = task.get();
            display_json(jvalue, "R: ");

            if (!jvalue.is_null())
            {
                action(jvalue, answer);
            }
        }
        catch (http_exception const& e)
        {
            wcout << e.what() << endl;
        }
            })
        .wait();


        request.reply(status_codes::OK, answer);
}

void handle_post(http_request request)
{
    sql_cmd_count++;
   
    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer)
        {
            for (auto const& e : jvalue.as_object())
            {
                if (e.second.is_string()) {
                    auto key = e.first;
                    auto rvalue = e.second.as_string();
                    cout<<"WUHHHH: "<<e.second.as_string()<<endl;

                    answer["ret_cmd"] = json::value::string(rvalue);
                    answer["status"] = json::value::string("ok");
                    break;
                }
                else {
                    parse_error_count++;
                    answer["status"] = json::value::string("Bad input");
                    break;
                }
            }
        });
}

void handle_put(http_request request)
{
    TRACE("\ncannot handle PUT\n");

}

void handle_del(http_request request)
{
    TRACE("\ncannot handle DEL\n");

}

int main()
{
    http_listener listener("http://localhost:9080/tf/sqlhandler");

    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);
    listener.support(methods::PUT, handle_put);
    listener.support(methods::DEL, handle_del);

    try
    {
        listener
            .open()
            .then([&listener]() {TRACE("\nstarting to listen\n"); })
            .wait();

        while (true);
    }
    catch (exception const& e)
    {
        wcout << e.what() << endl;
    }

    return 0;
}