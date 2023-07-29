/****************************************************************************
  FileName     [ cmdParser.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define class CmdParser ]
  Author       [ Design Verification Lab, Chia-Hsu Chuang ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include <iosfwd>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "argparse.h"
#include "cmdCharDef.h"
#include "jthread.hpp"

class CmdParser;

//----------------------------------------------------------------------
//    External declaration
//----------------------------------------------------------------------
extern CmdParser* cmdMgr;

//----------------------------------------------------------------------
//    command execution status
//----------------------------------------------------------------------
enum CmdExecStatus {
    CMD_EXEC_DONE = 0,
    CMD_EXEC_ERROR = 1,
    CMD_EXEC_QUIT = 2,
    CMD_EXEC_NOP = 3,
    CMD_EXEC_EXECUTING = 4,

    // dummy
    CMD_EXEC_TOT
};

//----------------------------------------------------------------------
//    Base class : CmdExec
//----------------------------------------------------------------------

class CmdExec {
public:
    CmdExec() {}
    virtual ~CmdExec() {}

    virtual bool initialize() = 0;
    virtual CmdExecStatus exec(mythread::stop_token, const std::string&) = 0;
    virtual void usage() const = 0;
    virtual void summary() const = 0;
    virtual void help() const = 0;

    void setOptCmd(const std::string& str) { _optCmd = str; }
    bool checkOptCmd(const std::string& check) const;
    const std::string& getOptCmd() const { return _optCmd; }

private:
    std::string _optCmd;
};

/**
 * @brief class specification for commands that uses
 *        ArgParse::ArgumentParser to parse and generate help messages.
 *
 */
class ArgParseCmdType : public CmdExec {
    using ParserDefinition = std::function<void(ArgParse::ArgumentParser&)>;
    using Precondition = std::function<bool()>;
    using Uninterruptible = std::function<CmdExecStatus(ArgParse::ArgumentParser const&)>;
    using Interruptible = std::function<CmdExecStatus(mythread::stop_token, ArgParse::ArgumentParser const&)>;
    using OnParseSuccess = std::variant<Uninterruptible, Interruptible>;

public:
    ArgParseCmdType(std::string const& name) { _parser.name(name); }
    ~ArgParseCmdType() {}

    bool initialize() override;
    CmdExecStatus exec(mythread::stop_token, std::string const& option) override;
    void usage() const override { _parser.printUsage(); }
    void summary() const override { _parser.printSummary(); }
    void help() const override { _parser.printHelp(); }

    ParserDefinition parserDefinition;  // define the parser's arguments and traits
    Precondition precondition;          // define the parsing precondition

    OnParseSuccess onParseSuccess;  // define the action to take on parse success

private:
    ArgParse::ArgumentParser _parser;

    void printMissingParserDefinitionErrorMsg() const;
    void printMissingOnParseSuccessErrorMsg() const;
};

//----------------------------------------------------------------------
//    Base class : CmdParser
//----------------------------------------------------------------------

class CmdParser {
#define READ_BUF_SIZE 65536
#define PG_OFFSET 10

    using CmdMap = std::map<const std::string, std::unique_ptr<CmdExec>>;
    using CmdRegPair = std::pair<const std::string, std::unique_ptr<CmdExec>>;

public:
    CmdParser(const std::string& p)
        : _prompt{p}, _specialChars{"\"\' "}, _dofile(nullptr), _readBuf{}, _cursorPosition{0}, _historyIdx(0), _tabPressCount(0), _tempCmdStored(false), _currCmd{std::nullopt} {
        _readBuf.reserve(READ_BUF_SIZE);
    }
    virtual ~CmdParser() {}

    bool openDofile(const std::string& dof);
    void closeDofile();

    bool regCmd(const std::string&, unsigned, std::unique_ptr<CmdExec>&&);
    CmdExecStatus execOneCmd();
    void printHelps() const;

    void addArgument(std::string const& val) {
        _arguments.emplace_back(val);
        _variables.emplace(std::to_string(_arguments.size()), val);
    }

    // public helper functions
    void printHistory() const;
    void printHistory(size_t nPrint) const;
    CmdExec* getCmd(std::string);

    void sigintHandler(int signum);

private:
    // Private member functions
    void resetBufAndPrintPrompt() {
        _readBuf.clear();
        _cursorPosition = 0;
        _tabPressCount = 0;
        printPrompt();
    }
    ParseChar getChar(std::istream&) const;
    bool readCmd(std::istream&);
    std::pair<CmdExec*, std::string> parseCmd();
    void listCmd(const std::string&);
    bool listCmdDir(const std::string&);
    void printPrompt() const;
    bool pushDofile();
    bool popDofile();

    // Helper functions
    bool moveCursor(int);
    bool deleteChar();
    void insertChar(char);
    void deleteLine();
    void reprintCmd();
    void moveToHistory(int index);
    bool addHistory();
    void retrieveHistory();

    std::string replaceVariableKeysWithValues(std::string const& str) const;
    void saveArgumentsInVariables(std::string const& str);

    inline bool isSpecialChar(char ch) const { return _specialChars.find_first_of(ch) != std::string::npos; }
    std::pair<CmdMap::const_iterator, CmdMap::const_iterator> getCmdMatches(std::string const& str);
    void printAsTable(std::vector<std::string> words, size_t widthLimit) const;

    // Data members
    std::string const _prompt;                                // command prompt
    std::string const _specialChars;                          // The characters that are identified as special characters when parsing
    std::ifstream* _dofile;                                   // for command script
    std::string _readBuf;                                     // read buffer
    size_t _cursorPosition;                                   // current cursor postion on the readBuf
    std::vector<std::string> _history;                        // oldest:_history[0],latest:_hist.back()
    int _historyIdx;                                          // (1) Position to insert history string
                                                              //     i.e. _historyIdx = _history.size()
                                                              // (2) When up/down/pgUp/pgDn is pressed,
                                                              //     position to history to retrieve
    size_t _tabPressCount;                                    // The number of tab pressed
    bool _tempCmdStored;                                      // When up/pgUp is pressed, current line
                                                              // will be stored in _history and
                                                              // _tempCmdStored will be true.
                                                              // Reset to false when new command added
    CmdMap _cmdMap;                                           // map from string to command
    std::stack<std::ifstream*> _dofileStack;                  // For recursive dofile calling
    std::optional<mythread::jthread> _currCmd;                // the current (ongoing) command
    std::unordered_map<std::string, std::string> _variables;  // stores the variables key-value pairs, e.g., $1, $INPUT_FILE, etc...
    std::vector<std::string> _arguments;                      // stores the extra dofile arguments given when invoking the program
    std::string _dofileName;
};

#endif  // CMD_PARSER_H
