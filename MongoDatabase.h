#include <mongocxx/client.hpp>
#include <bsoncxx/builder/stream/document.hpp>
//#include <bsoncxx/json.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <StringInline.h>
#pragma once

using namespace std;
class MongoDatabase
{
public:
	std::string mongoURILocalStr;
	//static const mongocxx::uri mongoURI;
	vector<string> GetDatabases();

	// Leaving databaseUri empty will connect to the local database
	int Connect(std::string databaseUri);

	static MongoDatabase* GetInstance();
	mongocxx::client* GetClient();
private:
	static std::unique_ptr<MongoDatabase> instance;
	static std::unique_ptr<mongocxx::client> currentClient;

	MongoDatabase();
	std::string GetEnvironmentVariable(std::string environmentVarKey);
};