#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <vector>
#include "curlpp/Easy.hpp"
#include "curlpp/cURLpp.hpp"
#include "curlpp/Options.hpp"
#include "gumbo.h"

class DataCollector {
public:
  typedef std::vector<std::string> Links;
  DataCollector();
  ~DataCollector();

  void SetUrl(const std::string& url);
  void Reset();

  bool GetHtmlData(std::wstring& text, Links& links);
  bool GetPdfData(std::string& text);
  bool GetDocData(std::string& text);

private:
  bool GetLinks(GumboNode* node, Links& links);
  bool GetText(GumboNode* node, std::string& text);

private:
  curlpp::Cleanup init_handler_;
  curlpp::Easy request_handler_;
  std::string url_;
};


#endif // DATA_COLLECTOR_H

