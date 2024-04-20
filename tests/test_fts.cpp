#include <fstream>
#include <cstdlib>
#include <string>
#include <limits>

#include <cxxopts.hpp>
#include <sqlite3.h>
#include <logger.h>

using namespace cxxopts;

/*
 * 测试用SQL语句
 */
#define SQL_PREFIX_SEARCH "select name from movies where name match '^假面*'"

int main(int argc, char* argv[]) {
	ParseResult args;
	Options options("test_fts", "SQLite3 FTS5 test app");

	try {
		options.add_options()
			("f,dbf", "Database data file", value<std::string>()->default_value("test.dbf"))
			("h,help", "Print usage")
		;
		args = options.parse(argc, argv);
	} catch (cxxopts::OptionException &e){
		fprintf(stderr, "parse error: %s", e.what());
		exit(-1);
	}

	if (args.count("help")) {
		_info("%s", options.help().c_str());
		exit(0);
	}

	std::string dbf = args["dbf"].as<std::string>();
	std::ifstream f(dbf.c_str());
	if (!f.good()) {
		_error("database datafile `%s` not exist, abort!", dbf.c_str());
		return -1;
	}

	sqlite3* db;
	char *zErrMsg = 0;
	int ret = 0;

	ret = sqlite3_open(dbf.c_str(), &db);
	if (ret) {
		_error("can't open database: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(-1);
	}

	const char* pstr;
	std::string sql = SQL_PREFIX_SEARCH;
	sql += ";";

	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);

	_info("exec: %s", sql.c_str());
	while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
		pstr = (const char*)sqlite3_column_text(stmt, 0);
		_info("movie name: %s", pstr);
	}
	_info("ret= %d", ret);
	sqlite3_finalize(stmt);

	return 0;
}
