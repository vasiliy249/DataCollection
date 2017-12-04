#include "stdafx.h"
#include <stdlib.h>

#include <string>
#include <codecvt>
#include <iostream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>

#include <io.h>
#include <fcntl.h>

#include "json.hpp"
using json = nlohmann::json;

struct Document {
	std::wstring url;
	std::wstring text;
};

class DB {
private:
	std::string host = "http://levizi.sytes.net:5984/";
	int total_rows, current_rows;
	curlpp::Cleanup myCleanup;
	json getJSON(std::string curHost) {
		std::ostringstream os;
		os << curlpp::options::Url(curHost);
		return json::parse(os.str());
	}
public:
	DB(std::string dbName) {
		host += dbName;
		json j = getJSON(host);
		total_rows = j["doc_count"];
		current_rows = 0;
		host += "/_all_docs?include_docs=true&limit=1&skip=";
	}
	int length() {
		return total_rows;
	}
	boolean hasNext() {
		return current_rows < total_rows;
	}
	Document next() {
		Document doc;
		json j = getJSON(host + std::to_string(current_rows++));
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		doc.url = conv.from_bytes(j["rows"][0]["doc"]["_id"]);
		doc.text = conv.from_bytes(j["rows"][0]["doc"]["text"]);
		return doc;
	}
};

int main(int, char **) {
	setlocale(LC_ALL, "");
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);

	DB db("spbu_ru");
	std::wcout << db.length() << std::endl;
	while (db.hasNext()) {
		Document doc = db.next();
		std::wcout << "--url  : " << doc.url << std::endl;
		std::wcout << "--text : " << doc.text << std::endl << std::endl;
	}

	system("pause");
	return 0;
}