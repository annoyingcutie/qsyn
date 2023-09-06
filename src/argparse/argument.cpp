/****************************************************************************
  FileName     [ argument.cpp ]
  PackageName  [ argparser ]
  Synopsis     [ Define argument interface for ArgumentParser ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "./argparse.hpp"

using namespace std;

namespace ArgParse {

/**
 * @brief If the argument has a default value, reset to it.
 *
 */
void Argument::reset() {
    _pimpl->do_reset();
}

/**
 * @brief get tokens from `tokens` and takes actions accordingly.
 *        if tokens are not empty, mark the
 *
 * @param tokens
 * @return true if action success, or
 * @return false if action failed or < l argument are available
 */
bool Argument::takeAction(TokensView tokens) {
    if (!_pimpl->do_takeAction(tokens) || !constraintsSatisfied()) return false;

    return true;
}

/**
 * @brief Get a range of at most nargs.upper consecutive unparsed tokens.
 *
 * @param tokens
 * @return TokensView
 */
TokensView Argument::getParseRange(TokensView tokens) const {
    auto parse_start = std::find_if(
        tokens.begin(), tokens.end(),
        [](Token& token) { return token.parsed == false; });

    auto parse_end = std::find_if(
        parse_start, tokens.end(),
        [](Token& token) { return token.parsed == true; });
    return tokens.subspan(parse_start - tokens.begin(), std::min(getNArgs().upper, static_cast<size_t>(parse_end - parse_start)));
}

bool Argument::tokensEnoughToParse(TokensView tokens) const {
    return (tokens.size() >= getNArgs().lower);
}
/**
 * @brief If the argument is parsed, print out the parsed value. If not,
 *        print the default value if it has one, or "(unparsed)" if not.
 *
 */
void Argument::printStatus() const {
    fmt::println("  {:<8}   = {}", getName(), std::invoke([this]() {
                     if (isParsed()) {
                         return fmt::format("{}", *this);
                     } else if (hasDefaultValue()) {
                         return fmt::format("{} (default)", *this);
                     } else {
                         return "(unparsed)"s;
                     }
                 }));
}

}  // namespace ArgParse