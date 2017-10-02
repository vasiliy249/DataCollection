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
    std::cout << "*** LINKS ***\n\n";
    for (size_t i = 0; i < links.size(); ++i) {
      std::cout << links[i] << std::endl;
    }
  } else {
    std::cout << "Error during perform request\n";
  }

  system("pause");
  return 0;
}
