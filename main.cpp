#include "DataCollector.h"
#include <iostream>
#include <cstdlib>

int main() {
  setlocale(LC_ALL, "Russian");

  DataCollector data_collector;
  data_collector.SetUrl("http://www.apmath.spbu.ru/ru/");
  std::wstring clean_text;
  DataCollector::Links links;
  if (data_collector.GetHtmlData(clean_text, links)) {
    std::cout << "*** CLEAN TEXT OF HTML PAGE ***\n\n";
    std::wcout << clean_text << L"\n\n";
    std::cout << "*** PART OF LINKS ***\n\n";
    size_t max_l = links.size() > 20 ? 20 : links.size();
    for (size_t i = 0; i < max_l; ++i) {
      std::cout << links[i] << std::endl;
    }
  } else {
    std::cout << "Error during perform request\n";
  }

  data_collector.Reset();
  data_collector.SetUrl("http://tools.thermofisher.com/content/sfs/manuals/16310013.pdf");
  std::string pdf_text;
  if (data_collector.GetPdfData(pdf_text)) {
    std::cout << "*** TEXT FROM DOWNLOADED PDF ***\n\n";
    std::cout << pdf_text;
  } else {
    std::cout << "Error during perform request\n";
  }

  system("pause");
  return 0;
}
