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
  bool GetDocxData(std::string& text);

  bool GetHtmlDataFromString(const std::string& utf8_html_string,
                             std::wstring& text, Links& links);
  bool GetTextFromPdfFile(const std::string& path,
                          std::string& text);
  // only absolute path!
  bool GetTextFromDocxFile(const std::string& path,
                           std::string& text);

private:
  bool GetLinks(GumboNode* node, Links& links);
  bool GetText(GumboNode* node, std::string& text);

private:
  curlpp::Cleanup init_handler_;
  curlpp::Easy request_handler_;
  std::string url_;
};


#endif // DATA_COLLECTOR_H

