/****************************************************************************
  FileName     [ argparseArgument.h ]
  PackageName  [ argparser ]
  Synopsis     [ Define class ArgParse::Argument member functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef QSYN_ARG_PARSE_ARGUMENT_H
#define QSYN_ARG_PARSE_ARGUMENT_H

#include <any>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>

#include "argparseArgTypes.h"
#include "argparseDef.h"
#include "argparseErrorMsg.h"

namespace ArgParse {

/**
 * @brief A type-erased interface to all argument types that
 *        `ArgParse::ArgParser` admits.
 *
 */
class Argument {
public:
    using ActionType = std::function<ParseResult(Argument&)>;

    template <typename ArgType>
    Argument(ArgType arg)
        : _pimpl{std::make_unique<ArgumentModel<ArgType>>(std::move(arg))},
          _traits() {}

    template <size_t N>
    Argument(char const (&arg)[N])
        : _pimpl{std::make_unique<ArgumentModel<std::string>>(std::move(std::string(arg)))},
          _traits() {}

    Argument(Argument const& other)
        : _pimpl(other._pimpl->clone()), _traits(other._traits) {}

    template <typename ArgType>
    Argument& operator=(ArgType const& other) {
        _pimpl = std::make_unique<ArgumentModel<ArgType>>(std::move(other));
        return *this;
    }

    Argument& operator=(Argument const& other) {
        other._pimpl->clone().swap(_pimpl);
        _traits = other._traits;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, Argument const& arg) {
        return arg.isParsed()
                   ? arg._pimpl->doPrint(os)
                   : (arg.hasDefaultValue()
                          ? arg._pimpl->doPrint(os) << " (default)"
                          : os << "(unparsed)");
    }

    template <typename T>
    operator T&() const;

    // argument decorators

    Argument& name(std::string const& name) {
        setName(name);
        return *this;
    }

    Argument& optional() {
        setOptional(true);
        return *this;
    }

    template <typename T>
    Argument& defaultValue(T const& val) {
        setDefaultValue(val);
        return *this;
    }

    Argument& help(std::string const& help) {
        setHelp(help);
        return *this;
    }

    Argument& action(ActionType const& action) {
        setAction(action);
        return *this;
    }

    // actions

    ParseResult parse(std::span<TokenPair> tokens) {
        ParseResult result = hasAction() ? getAction()(*this) : _pimpl->doParse(tokens);
        setParsed(true);
        return result;
    }

    // getters and attributes
    std::string const& getName() const { return _traits.name; }
    std::string const& getHelp() const { return _traits.help; }
    std::string getTypeString() const { return _pimpl->doTypeString(); }
    size_t getNumMandatoryChars() const { return _traits.numMandatoryChars; }
    ActionType const& getAction() const { return _traits.action; }
    ActionType const& getResetCallback() const { return _traits.resetCallback; }

    bool isMandatory() const { return !_traits.optional; }
    bool isOptional() const { return _traits.optional; }
    template <typename T>
    bool isOfType() const { return dynamic_cast<ArgumentModel<T>*>(_pimpl.get()) != nullptr; }
    bool isParsed() const { return _traits.parsed; }

    bool hasDefaultValue() const { return _traits.hasDefaultVal; }
    bool hasAction() const { return _traits.action != nullptr; }
    bool hasResetCallback() const { return _traits.resetCallback != nullptr; }

    // print functions
    void printHelpString() const;
    void printStatus() const;

    template <typename T>
    static ActionType storeConst(T const& constant) {
        // must pass `constant` by copy so the callback remenber its state!
        return [constant](Argument& arg) -> ParseResult {
            try {
                arg = constant;
            } catch (bad_arg_cast& e) {
                detail::printArgumentCastErrorMsg(arg);
                return ParseResult::error;
            }
            return ParseResult::success;
        };
    }

    static ActionType storeTrue() {
        return storeConst(true);
    }

    static ActionType storeFalse() {
        return storeConst(false);
    }

private:
    // only use for dummy return

    friend class ArgumentParser;
    struct ArgumentConcept;

    std::unique_ptr<ArgumentConcept> _pimpl;

    struct ArgumentTraits {
        ArgumentTraits()
            : name{}, help{}, numMandatoryChars{0}, parsed{false}, optional{false}, hasDefaultVal{false}, action{}, resetCallback{} {}
        std::string name;
        std::string help;
        size_t numMandatoryChars;

        bool parsed;
        bool optional;
        bool hasDefaultVal;
        ActionType action;
        ActionType resetCallback;
    };

    ArgumentTraits _traits;

    // pretty printing helpers
    std::string getSyntaxString() const;
    std::string typeBracket(std::string const& str) const;
    std::string optionBracket(std::string const& str) const;
    std::string formattedType() const;
    std::string formattedName() const;

    // setters
    void reset() {
        setParsed(false);
        if (hasResetCallback()) getResetCallback()(*this);
    }

    template <typename T>
    void setDefaultValue(T const& val) {
        try {
            *this = val;
        } catch (bad_arg_cast& e) {
            detail::printDefaultValueErrorMsg(*this);
        }
        _traits.optional = true;
        _traits.hasDefaultVal = true;
        _traits.resetCallback = resetToDefault(val);
    }
    void setHelp(std::string const& help) { _traits.help = help; }
    void setAction(ActionType const& action) { _traits.action = action; }
    void setName(std::string const& name) { _traits.name = name; }
    void setNumMandatoryChars(size_t n) { _traits.numMandatoryChars = n; }
    void setOptional(bool isOpt) { _traits.optional = isOpt; }
    void setParsed(bool parsed) { _traits.parsed = parsed; }

    template <typename T>
    static ActionType resetToDefault(T const& val) {
        // must pass `val` by copy so the callback remenber its state!
        return [val](Argument& arg) -> ParseResult {
            try {
                arg = val;
            } catch (bad_arg_cast& e) {
                detail::printArgumentCastErrorMsg(arg);
                return ParseResult::error;
            }
            return ParseResult::success;
        };
    }

    /**
     * @brief Interface representing the concept of an Argument.
     *
     */
    struct ArgumentConcept {
        virtual ~ArgumentConcept() {}

        // type erasure indirection layer
        virtual std::unique_ptr<ArgumentConcept> clone() const = 0;
        virtual std::string doTypeString() const = 0;
        virtual std::ostream& doPrint(std::ostream& os) const = 0;
        virtual ParseResult doParse(std::span<TokenPair> tokens) = 0;
    };

    /**
     * @brief type specialization of for each type of Argument.
     *
     */
    template <typename ArgType>
    struct ArgumentModel : public ArgumentConcept {
        ArgumentModel(ArgType arg)
            : _arg(std::move(arg)) {}

        ArgType _arg;

        // type erasure indirection layer
        std::unique_ptr<ArgumentConcept> clone() const override;
        std::string doTypeString() const override;
        std::ostream& doPrint(std::ostream& os) const override;
        ParseResult doParse(std::span<TokenPair> tokens) override {
            return detail::parse(_arg, tokens);
        }
    };
};

//----------------------------------------
// Argument functions
//----------------------------------------

/**
 * @brief cast the `Argument` to specific type. The target type has to be of the same type as
 *        the type stored under `Argument::ArgumentModel`. The user is expected to know the
 *        underlying type because this information is necessary for the constructor of this
 *        class. If the casting fails, throw `ArgParse::bad_arg_cast` errors.
 *
 * @tparam T
 * @return T
 */
template <typename T>
Argument::operator T&() const {
    if (auto ptr = dynamic_cast<ArgumentModel<T>*>(_pimpl.get())) {
        if (!isParsed() && !hasDefaultValue())
            detail::printArgumentUnparsedErrorMsg(*this);
        return ptr->_arg;
    } else {
        detail::printArgumentCastErrorMsg(*this);
        throw bad_arg_cast{};
    }
}

//----------------------------------------
// Argument::ArgumentModel functions
//----------------------------------------

/**
 * @brief clone an ArgumentModel. This function enables the
 *        type-erased copying of class `Argument`.
 *
 * @tparam ArgType
 * @return std::unique_ptr<Argument::ArgumentConcept >
 */
template <typename ArgType>
std::unique_ptr<Argument::ArgumentConcept> Argument::ArgumentModel<ArgType>::clone() const {
    return std::make_unique<ArgumentModel>(*this);
}

/**
 * @brief get the type string of an ArgumentModel. This function enables the
 *        type-erased `Argument::getTypeString()`.
 *
 * @tparam ArgType
 * @return std::unique_ptr<Argument::ArgumentConcept >
 */
template <typename ArgType>
std::string Argument::ArgumentModel<ArgType>::doTypeString() const {
    return detail::getTypeString(_arg);
}

/**
 * @brief print an ArgumentModel. This function enables the
 *        type-erased `std::ostream& operator<< (std::ostream&, Argument const&)`.
 *
 * @tparam ArgType
 * @return std::unique_ptr<Argument::ArgumentConcept >
 */
template <typename ArgType>
std::ostream& Argument::ArgumentModel<ArgType>::doPrint(std::ostream& os) const {
    return os << _arg;
}

}  // namespace ArgParse

#endif  // QSYN_ARG_PARSE_ARGUMENT_H