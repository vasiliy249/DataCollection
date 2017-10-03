#include "DataCollector.h"
#include <sstream>
#include <codecvt>
#include <fstream>

std::string GetTempPath() {
  std::string str_tmp_path;
  char char_path[MAX_PATH];
  if (GetTempPath(MAX_PATH, char_path)) {
    str_tmp_path = char_path;
  }
  return str_tmp_path;
}

struct FileHolder {
  FileHolder(const std::string& file) : file_(file) {}
  ~FileHolder() {
    ::remove(file_.c_str());
  }
  std::string file_;
};

std::wstring utf8_to_wstring(const std::string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.from_bytes(str);
}

DWORD RunAndWait(const std::string& module_path,
                 const std::string& params) {
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  std::string command_line = module_path + " " + params;
  std::vector<char> sz_params(command_line.begin(), command_line.end());
  sz_params.push_back(0);

  if (!::CreateProcessA(module_path.c_str(), &sz_params[0],
                        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
  {
    return 1;
  }
  ::WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 1;
  ::GetExitCodeProcess(pi.hProcess, &code);
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);
  return code;
}

DataCollector::DataCollector() {
  Reset();
}

DataCollector::~DataCollector() {
  
}

void DataCollector::SetUrl(const std::string& url) {
  url_ = url;
}

void DataCollector::Reset() {
  url_.clear();
  request_handler_.reset();
}

bool DataCollector::GetHtmlData(std::wstring& text, Links& links) {
  if (url_.empty()) {
    return false;
  }
  request_handler_.setOpt(cURLpp::Options::Url(url_));
  std::stringstream resp_stream;
  request_handler_.setOpt(cURLpp::Options::WriteStream(&resp_stream));
  request_handler_.perform();

  return GetHtmlDataFromString(resp_stream.str(), text, links);
}

bool DataCollector::GetPdfData(std::string& text) {
  if (url_.empty()) {
    return false;
  }
  request_handler_.setOpt(cURLpp::Options::Url(url_));
  std::string str_tmp_path = GetTempPath();
  FileHolder pdf_file(str_tmp_path + "new.pdf");
  
  std::ofstream resp_stream(pdf_file.file_, std::ofstream::binary);
  request_handler_.setOpt(cURLpp::Options::WriteStream(&resp_stream));
  request_handler_.perform();
  resp_stream.close();

  return GetTextFromPdfFile(pdf_file.file_, text);
}

bool DataCollector::GetDocxData(std::string& text) {
  if (url_.empty()) {
    return false;
  }
  request_handler_.setOpt(cURLpp::Options::Url(url_));
  std::string str_tmp_path = GetTempPath();
  FileHolder docx_file(str_tmp_path + "new.docx");

  std::ofstream resp_stream(docx_file.file_, std::ofstream::binary);
  request_handler_.setOpt(cURLpp::Options::WriteStream(&resp_stream));
  request_handler_.perform();
  resp_stream.close();

  return GetTextFromDocxFile(docx_file.file_, text);
}

bool DataCollector::GetHtmlDataFromString(const std::string& utf8_html_string,
                                          std::wstring& text, Links& links)
{
  GumboOutput* output = gumbo_parse(utf8_html_string.c_str());

  if (!GetLinks(output->root, links)) {
    links.clear();
    return false;
  }

  std::string utf8_text;
  if (!GetText(output->root, utf8_text)) {
    return false;
  } else {
    text = utf8_to_wstring(utf8_text);
  }

  gumbo_destroy_output(&kGumboDefaultOptions, output);
  return true;
}

bool DataCollector::GetTextFromPdfFile(const std::string& path,
                                       std::string& text)
{
  std::string str_tmp_path = GetTempPath();
  FileHolder txt_file(str_tmp_path + "new.txt");
  DWORD code = RunAndWait("pdftotext.exe", path + " " + txt_file.file_);
  if (code != 0) {
    return false;
  }

  std::ifstream txt_stream(txt_file.file_);
  if (txt_stream.is_open()) {
    std::stringstream buff;
    buff << txt_stream.rdbuf();
    text = buff.str();
    txt_stream.close();
    return true;
  }
  return false;
}

bool DataCollector::GetTextFromDocxFile(const std::string& path,
                                        std::string& text)
{
  std::string directory;
  size_t pos = path.find_last_of('\\');
  if (pos == std::string::npos) {
    return false;
  } else {
    directory = path.substr(0, pos + 1);
  }
  DWORD code = RunAndWait("docx2txt.bat", path);
  if (code != 0) {
    return false;
  }
  FileHolder txt_file(directory + "new.txt");

  std::ifstream txt_stream(txt_file.file_);
  if (txt_stream.is_open()) {
    std::stringstream buff;
    buff << txt_stream.rdbuf();
    text = buff.str();
    txt_stream.close();
  }
  return true;
}

bool DataCollector::GetLinks(GumboNode* node, Links& links) {
  if (node == NULL) {
    return false;
  }

  if (node->type != GUMBO_NODE_ELEMENT) {
    return false;
  }

  if (node->v.element.tag == GUMBO_TAG_A) {
    GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
    if (href) {
      links.push_back(href->value);
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    GetLinks(static_cast<GumboNode*>(children->data[i]), links);
  }
  return true;
}

bool DataCollector::GetText(GumboNode* node, std::string& text) {
  if (node == NULL) {
    return false;
  }

  if (node->type == GUMBO_NODE_TEXT) {
    if (!text.empty()) {
      text.append(" ");
    }
    text.append(node->v.text.text);
    return true;
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    std::string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      GetText(static_cast<GumboNode*>(children->data[i]), text);
    }
    return true;
  } else {
    return false;
  }
}