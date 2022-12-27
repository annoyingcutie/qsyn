/****************************************************************************
  FileName     [ textFormat.h ]
  PackageName  [ graph ]
  Synopsis     [ Declare text formatting functions such as colors, bold, etc. ]
  Author       [ Mu-Te (Joshua) Lau ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef QSYN_TEXT_FORMAT_H
#define QSYN_TEXT_FORMAT_H
#include <cassert>
#include <iostream>
#include <string>

namespace TextFormat {

std::string SET_FORMAT(const std::string& str, const size_t& code);

std::string BOLD(const std::string& str);
std::string DIM(const std::string& str);
std::string ITALIC(const std::string& str);
std::string ULINE(const std::string& str);
std::string SWAP(const std::string& str);
std::string STRIKE(const std::string& str);

std::string BLACK(const std::string& str);
std::string RED(const std::string& str);
std::string GREEN(const std::string& str);
std::string YELLOW(const std::string& str);
std::string BLUE(const std::string& str);
std::string MAGENTA(const std::string& str);
std::string CYAN(const std::string& str);
std::string WHITE(const std::string& str);

std::string BG_BLACK(const std::string& str);
std::string BG_RED(const std::string& str);
std::string BG_GREEN(const std::string& str);
std::string BG_YELLOW(const std::string& str);
std::string BG_BLUE(const std::string& str);
std::string BG_MAGENTA(const std::string& str);
std::string BG_CYAN(const std::string& str);
std::string BG_WHITE(const std::string& str);

std::string GRAY(const std::string& str);
std::string LIGHT_RED(const std::string& str);
std::string LIGHT_GREEN(const std::string& str);
std::string LIGHT_YELLOW(const std::string& str);
std::string LIGHT_BLUE(const std::string& str);
std::string LIGHT_MAGENTA(const std::string& str);
std::string LIGHT_CYAN(const std::string& str);
std::string LIGHT_WHITE(const std::string& str);

std::string BG_GRAY(const std::string& str);
std::string LIGHT_BG_RED(const std::string& str);
std::string LIGHT_BG_GREEN(const std::string& str);
std::string LIGHT_BG_YELLOW(const std::string& str);
std::string LIGHT_BG_BLUE(const std::string& str);
std::string LIGHT_BG_MAGENTA(const std::string& str);
std::string LIGHT_BG_CYAN(const std::string& str);
std::string LIGHT_BG_WHITE(const std::string& str);

};  // namespace TextFormat

#endif  // QSYN_TEXT_FORMAT_H
