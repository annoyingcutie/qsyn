/****************************************************************************
  PackageName  [ util ]
  Synopsis     [ RAII wrapper for temporary files and directories ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace dvlab {

namespace utils {

namespace detail {

std::filesystem::path create_tmp_dir(std::string_view prefix);
std::filesystem::path create_tmp_file(std::string_view prefix);

}  // namespace detail

class TmpDir {
public:
    TmpDir() : _dir{detail::create_tmp_dir(std::filesystem::temp_directory_path().string() + "/dvlab-")} {}
    TmpDir(std::string_view prefix) : _dir{detail::create_tmp_dir(prefix)} {}
    ~TmpDir() { std::filesystem::remove_all(_dir); }
    // deletes copy ctors and assignment operators because we don't want to copy the directory
    TmpDir(TmpDir const&)            = delete;
    TmpDir& operator=(TmpDir const&) = delete;

    TmpDir(TmpDir&&)            = default;
    TmpDir& operator=(TmpDir&&) = default;

    std::filesystem::path path() const { return _dir; }

private:
    std::filesystem::path _dir;
};

class TmpFile {
public:
    TmpFile() : _path{detail::create_tmp_file(std::filesystem::temp_directory_path().string() + "/dvlab-")} {}
    TmpFile(std::string_view prefix) : _path{detail::create_tmp_file(prefix)} {}
    ~TmpFile() { std::filesystem::remove(_path); }

    // deletes copy ctors and assignment operators because we don't want to copy the file
    TmpFile(TmpFile const&)            = delete;
    TmpFile& operator=(TmpFile const&) = delete;

    TmpFile(TmpFile&&)            = default;
    TmpFile& operator=(TmpFile&&) = default;

    std::filesystem::path path() const { return _path; }

private:
    std::filesystem::path _path;
};

}  // namespace utils

}  // namespace dvlab
