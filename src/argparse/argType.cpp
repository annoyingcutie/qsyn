/****************************************************************************
  FileName     [ argType.cpp ]
  PackageName  [ argparser ]
  Synopsis     [ Define parser argument types ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "./argType.hpp"

#include <filesystem>

#include "util/ordered_hashset.hpp"
#include "util/trie.hpp"
#include "util/util.hpp"

using namespace std;

namespace ArgParse {

static_assert(IsContainerType<std::vector<int>> == true);
static_assert(IsContainerType<std::vector<std::string>> == true);
static_assert(IsContainerType<ordered_hashset<float>> == true);
static_assert(IsContainerType<std::string> == false);
static_assert(IsContainerType<std::array<int, 3>> == false);

ArgType<std::string>::ConstraintType choices_allow_prefix(std::vector<std::string> choices) {
    ranges::for_each(choices, [](std::string& str) { str = toLowerString(str); });
    dvlab::utils::Trie trie{choices.begin(), choices.end()};

    return [choices, trie](std::string const& val) -> bool {
        auto isExactMatchToChoice = [&val, &trie](std::string const& choice) -> bool {
            return toLowerString(val) == choice;
        };
        auto freq = trie.frequency(toLowerString(val));

        if (freq == 1) return true;
        if (std::ranges::any_of(choices, isExactMatchToChoice)) return true;

        if (freq > 1) {
            fmt::println(stderr, "Error: ambiguous choice \"{}\": could match {}!!\n",
                         val, fmt::join(choices | views::filter([&val](std::string const& choice) { return choice.starts_with(toLowerString(val)); }), ", "));
        } else {
            fmt::println(stderr, "Error: invalid choice \"{}\": please choose from {{{}}}!!\n",
                         val, fmt::join(choices, ", "));
        }
        return false;
    };
}

ArgType<std::string>::ConstraintType const path_readable = {
    [](std::string const& filepath) {
        namespace fs = std::filesystem;
        if (!fs::exists(filepath)) {
            fmt::println(stderr, "Error: the file \"{}\" does not exist!!", filepath);
            return false;
        }
        return true;
    }};

ArgType<std::string>::ConstraintType const path_writable = {
    [](std::string const& filepath) {
        namespace fs = std::filesystem;
        auto dir = fs::path{filepath}.parent_path();
        if (dir.empty()) dir = ".";
        if (!fs::exists(dir)) {
            fmt::println(stderr, "Error: the directory for file \"{}\" does not exist!!", filepath);
            return false;
        }
        return true;
    }};

ArgType<std::string>::ConstraintType starts_with(std::vector<std::string> const& prefixes) {
    return [prefixes](std::string const& str) {
        if (std::ranges::none_of(prefixes, [&str](std::string const& prefix) { return str.starts_with(prefix); })) {
            fmt::println(stderr, "Error: string \"{}\" should start with one of \"{}\"!!",
                         str, fmt::join(prefixes, "\", \""));
            return false;
        }
        return true;
    };
}

ArgType<std::string>::ConstraintType ends_with(std::vector<std::string> const& suffixes) {
    return [suffixes](std::string const& str) {
        if (std::ranges::none_of(suffixes, [&str](std::string const& suffix) { return str.ends_with(suffix); })) {
            fmt::println(stderr, "Error: string \"{}\" should start end one of \"{}\"!!",
                         str, fmt::join(suffixes, "\", \""));
            return false;
        }
        return true;
    };
}

ArgType<std::string>::ConstraintType allowed_extension(std::vector<std::string> const& extensions) {
    return [extensions](std::string const& str) {
        if (std::ranges::none_of(extensions, [&str](std::string const& ext) { return str.substr(std::min(str.find_last_of('.'), str.size())) == ext; })) {
            fmt::println(stderr, "Error: file \"{}\" must have one of the following extension: \"{}\"!!",
                         str, fmt::join(extensions, "\", \""));
            return false;
        }
        return true;
    };
}

/**
 * @brief generate a callback that sets the argument to true.
 *        This function also set the default value to false.
 *
 * @param arg
 * @return ArgParse::ActionCallbackType
 */
ActionCallbackType storeTrue(ArgType<bool>& arg) {
    arg.defaultValue(false);
    arg.nargs(0ul);
    return [&arg](TokensView) { arg.appendValue(true); return true; };
}

/**
 * @brief generate a callback that sets the argument to false.
 *        This function also set the default value to true.
 *
 * @param arg
 * @return ArgParse::ActionCallbackType
 */
ActionCallbackType storeFalse(ArgType<bool>& arg) {
    arg.defaultValue(true);
    arg.nargs(0ul);
    return [&arg](TokensView) { arg.appendValue(false); return true; };
}

ActionCallbackType help(ArgType<bool>& arg) {
    arg.markAsHelpAction();
    arg.nargs(0ul);
    return [&arg](TokensView) -> bool {
        return true;
    };
}

ActionCallbackType version(ArgType<bool>& arg) {
    arg.markAsVersionAction();
    arg.nargs(0ul);
    return [&arg](TokensView) -> bool {
        return true;
    };
}

}  // namespace ArgParse