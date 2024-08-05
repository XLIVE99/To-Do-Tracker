#include "MongoDatabase.h"

using namespace std;

// Don't destroy this until we no longer need database operations
static const mongocxx::instance mongoInstance = mongocxx::instance{};
std::unique_ptr<MongoDatabase> MongoDatabase::instance = nullptr;
std::unique_ptr<mongocxx::client> MongoDatabase::currentClient = nullptr;

std::string MongoDatabase::GetEnvironmentVariable(std::string environmentVarKey)
{
	char* pBuffer = nullptr;
	size_t size = 0;
	auto key = environmentVarKey.c_str();
	// Use the secure version of getenv, ie. _dupenv_s to fetch environment variable.
	if (_dupenv_s(&pBuffer, &size, key) == 0 && pBuffer != nullptr)
	{
		std::string environmentVarValue(pBuffer);
		free(pBuffer);
		return environmentVarValue;
	}
	else
	{
		return "";
	}
}

MongoDatabase::MongoDatabase()
{
	// Create instance
	MongoDatabase::instance = std::unique_ptr<MongoDatabase>(this);
	//if(mongoInstance == nullptr)
	//	mongoInstance = new mongocxx::instance();

#if _DEBUG
	MongoDatabase::mongoURILocalStr = GetEnvironmentVariable("MONGODB_LOCAL");
#else
	// I couldn't find how to set environment variable for release
	// Hardcode the local mongo uri
	MongoDatabase::mongoURILocalStr = "mongodb://127.0.0.1:27017/";
#endif
}

// Get all database names
vector<string> MongoDatabase::GetDatabases()
{
	return (*currentClient).list_database_names();
}

int MongoDatabase::Connect(std::string databaseUri)
{
	//mongocxx::instance inst {};
	mongocxx::options::client client_options;

	// Trim the string (both from start and end)
	rtrim(databaseUri);
	ltrim(databaseUri);
	if (databaseUri.empty())
	{
#if _DEBUG
		std::cout << "database uri is empty, changed with local uri" << std::endl;
#endif
		databaseUri = mongoURILocalStr;
	}

	mongocxx::uri mongoURI;
	try
	{
		mongoURI = mongocxx::uri{ databaseUri };
	}
	catch (const std::exception&)
	{
#if _DEBUG
		cout << "invalid uri" << endl;
#endif
		throw std::exception("Invalid uri");
		return 1;
	}

	auto api = mongocxx::options::server_api{mongocxx::options::server_api::version::k_version_1};
	client_options.server_api_opts(api);

	currentClient = std::unique_ptr<mongocxx::client>(new mongocxx::client(mongoURI, client_options));

	try
	{
		// Shortest way to check if we established connection to the database
		currentClient->list_database_names();
	}
	catch (const std::exception& e)
	{
		cout << "There is an error " << e.what() << endl;
		throw e;
		return 2;
	}

#if _DEBUG
	cout << "Connected to the " << databaseUri << endl;
#endif
	return 0;
}

MongoDatabase* MongoDatabase::GetInstance()
{
	if (MongoDatabase::instance == nullptr)
	{
#if _DEBUG
		cout << "Created database instance!" << endl;
#endif
		new MongoDatabase();
	}
	return MongoDatabase::instance.get();
}

mongocxx::client* MongoDatabase::GetClient()
{
	return currentClient.get();
}