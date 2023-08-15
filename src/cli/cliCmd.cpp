/****************************************************************************
  FileName     [ cliCmd.cpp ]
  PackageName  [ cli ]
  Synopsis     [ Define common commands for any CLI ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cstddef>
#include <cstdlib>
#include <string>

#include "cli/cli.hpp"
#include "util/usage.hpp"
#include "util/util.hpp"

using namespace std;
extern size_t verbose;
extern size_t colorLevel;
extern dvlab_utils::Usage usage;

using namespace ArgParse;

unique_ptr<ArgParseCmdType> helpCmd();
unique_ptr<ArgParseCmdType> quitCmd();
unique_ptr<ArgParseCmdType> dofileCmd();
unique_ptr<ArgParseCmdType> usageCmd();
unique_ptr<ArgParseCmdType> verboseCmd();
unique_ptr<ArgParseCmdType> seedCmd();
unique_ptr<ArgParseCmdType> historyCmd();
unique_ptr<ArgParseCmdType> clearCmd();
unique_ptr<ArgParseCmdType> loggerCmd();

bool initCommonCmd() {
    if (!(cli.regCmd("QQuit", 2, quitCmd()) &&
          cli.regCmd("HIStory", 3, historyCmd()) &&
          cli.regCmd("HELp", 3, helpCmd()) &&
          cli.regCmd("DOfile", 2, dofileCmd()) &&
          cli.regCmd("USAGE", 5, usageCmd()) &&
          cli.regCmd("VERbose", 3, verboseCmd()) &&
          cli.regCmd("SEED", 4, seedCmd()) &&
          cli.regCmd("CLEAR", 5, clearCmd()) &&
          cli.regCmd("LOGger", 3, loggerCmd()))) {
        logger.fatal("Registering \"cli\" commands fails... exiting");
        return false;
    }
    return true;
}

unique_ptr<ArgParseCmdType> helpCmd() {
    auto cmd = make_unique<ArgParseCmdType>("HELp");
    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("shows helping message to commands");

        parser.addArgument<string>("command")
            .defaultValue("")
            .nargs(NArgsOption::OPTIONAL)
            .help("if specified, display help message to a command");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        string command = parser["command"];
        if (command.empty()) {
            cli.printHelps();
        } else {
            CmdExec* e = cli.getCmd(parser["command"]);
            if (!e) {
                fmt::println(stderr, "Error: illegal command!! ({})", parser.get<std::string>("command"));
                return CmdExecResult::ERROR;
            }
            e->help();
        }
        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> quitCmd() {
    auto cmd = make_unique<ArgParseCmdType>("QQuit");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("quit Qsyn");

        parser.addArgument<bool>("-force")
            .action(storeTrue)
            .help("quit without reaffirming");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        bool forced = parser["-force"];
        if (forced) return CmdExecResult::QUIT;

        fmt::print("Are you sure to quit (Yes/No)? [No] ");
        fflush(stdout);
        string ss;
        std::getline(std::cin, ss);

        if (std::cin.eof()) {
            fmt::print("EOF [assumed Yes]");
            return CmdExecResult::QUIT;
        }

        if (size_t s = ss.find_first_not_of(' '); s != string::npos) {
            ss = ss.substr(s);
            if ("yes"s.starts_with(toLowerString(ss)))
                return CmdExecResult::QUIT;
        }
        return CmdExecResult::DONE;  // not yet to quit
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> historyCmd() {
    auto cmd = make_unique<ArgParseCmdType>("HIStory");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("print command history");
        parser.addArgument<size_t>("nPrint")
            .nargs(NArgsOption::OPTIONAL)
            .help("if specified, print the <nprint> latest command history");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        if (parser["nPrint"].isParsed()) {
            cli.printHistory(parser["nPrint"]);
        } else {
            cli.printHistory();
        }
        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> dofileCmd() {
    auto cmd = make_unique<ArgParseCmdType>("DOfile");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("execute the commands in the dofile");

        parser.addArgument<string>("file")
            .constraint(path_readable)
            .help("path to a dofile, i.e., a list of Qsyn commands");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        if (!cli.openDofile(parser["file"])) {
            fmt::println("Error: cannot open file \"{}\"!!", parser.get<std::string>("file"));
            return CmdExecResult::ERROR;
        }

        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> usageCmd() {
    auto cmd = make_unique<ArgParseCmdType>("USAGE");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("report the runtime and/or memory usage");

        auto mutex = parser.addMutuallyExclusiveGroup();

        mutex.addArgument<bool>("-all")
            .action(storeTrue)
            .help("print both time and memory usage");
        mutex.addArgument<bool>("-time")
            .action(storeTrue)
            .help("print time usage");
        mutex.addArgument<bool>("-memory")
            .action(storeTrue)
            .help("print memory usage");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        bool repAll = parser["-all"];
        bool repTime = parser["-time"];
        bool repMem = parser["-memory"];

        if (!repAll && !repTime && !repMem) repAll = true;

        if (repAll) repTime = true, repMem = true;

        usage.report(repTime, repMem);

        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> verboseCmd() {
    auto cmd = make_unique<ArgParseCmdType>("VERbose");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("set verbose level to 0-9 (default: 3)");

        parser.addArgument<size_t>("level")
            .constraint({[](size_t const& val) {
                             return val <= 9 || val == 353;
                         },
                         [](size_t const& val) {
                             fmt::println(stderr, "Error: verbose level should be 0-9!!");
                         }})
            .help("0: silent, 1-3: normal usage, 4-6: detailed info, 7-9: prolix debug info");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        verbose = parser["level"];
        fmt::println("Note: verbose level is set to {}", verbose);

        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> loggerCmd() {
    auto cmd = make_unique<ArgParseCmdType>("LOGger");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        vector<string> logLevels = {"none", "fatal", "error", "warning", "info", "debug", "trace"};
        parser.help("display and set the logger's status");

        auto parsers = parser.addSubParsers();

        auto testParser = parsers.addParser("test");
        testParser.help("Test out logger setting");

        auto levelParser = parsers.addParser("level");
        levelParser.help("set logger level");
        levelParser.addArgument<string>("level")
            .constraint(choices_allow_prefix(logLevels))
            .help("set log levels. Levels (ascending): None, Fatal, Error, Warning, Info, Debug, Trace");

        auto historyParser = parsers.addParser("history");
        historyParser.help("print logger history");
        historyParser.addArgument<size_t>("num_history")
            .nargs(NArgsOption::OPTIONAL)
            .metavar("N")
            .help("print log history. If specified, print the lastest N logs");

        auto maskParser = parsers.addParser("mask");
        maskParser.help("set logger mask");
        maskParser.setOptionPrefix("+-");
        auto addFilterGroup = [&maskParser](std::string const& groupName) {
            auto group = maskParser.addMutuallyExclusiveGroup();
            group.addArgument<bool>("+" + groupName)
                .action(storeTrue)
                .help("unmask " + groupName + " logs");
            group.addArgument<bool>("-" + groupName)
                .action(storeTrue)
                .help("mask " + groupName + " logs");
        };

        for (auto& group : logLevels) {
            if (group == "none") continue;
            addFilterGroup(group);
        }
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        using dvlab_utils::Logger;

        if (parser.usedSubParser("test")) {
            logger.fatal("Test fatal log");
            logger.error("Test error log");
            logger.warning("Test warning log");
            logger.info("Test info log");
            logger.debug("Test debug log");
            logger.trace("Test trace log");
            return CmdExecResult::DONE;
        }

        if (parser.usedSubParser("level")) {
            auto level = Logger::str2LogLevel(parser["level"]);
            assert(level.has_value());
            logger.setLogLevel(*level);
            logger.debug("Setting logger level to {}", Logger::logLevel2Str(*level));
            return CmdExecResult::DONE;
        }

        if (parser.usedSubParser("mask")) {
            vector<string> logLevels = {"fatal", "error", "warning", "info", "debug", "trace"};

            for (auto& group : logLevels) {
                auto level = Logger::str2LogLevel(group);
                assert(level.has_value());
                if (parser["+" + group].isParsed()) {
                    logger.unmask(*level);
                    logger.debug("Unmasked logger level: {}", Logger::logLevel2Str(*level));
                } else if (parser["-" + group].isParsed()) {
                    logger.mask(*level);
                    logger.debug("Masked logger level: {}", Logger::logLevel2Str(*level));
                }
            }
            return CmdExecResult::DONE;
        }

        if (parser.usedSubParser("history")) {
            if (parser["num_history"].isParsed()) {
                logger.printLogs(parser.get<size_t>("num_history"));
            } else {
                logger.printLogs();
            }
            return CmdExecResult::DONE;
        }

        fmt::println("Logger Level: {}", Logger::logLevel2Str(logger.getLogLevel()));
        vector<string> masked;
        vector<string> logLevels = {"fatal", "error", "warning", "info", "debug", "trace"};
        for (auto& level : logLevels) {
            if (logger.isMasked(*Logger::str2LogLevel(level))) {
                masked.push_back(level);
            }
        }

        if (masked.size()) {
            fmt::println("Masked logging levels: {}", fmt::join(masked, ", "));
        }

        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> seedCmd() {
    auto cmd = make_unique<ArgParseCmdType>("SEED");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("set the random seed");

        parser.addArgument<unsigned>("seed")
            .defaultValue(353)
            .nargs(NArgsOption::OPTIONAL)
            .help("random seed value");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        srand(parser["seed"]);
        fmt::println("Note: seed is set to {}", parser.get<unsigned>("seed"));
        return CmdExecResult::DONE;
    };

    return cmd;
}

unique_ptr<ArgParseCmdType> clearCmd() {
    auto cmd = make_unique<ArgParseCmdType>("CLEAR");

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("clear the console");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        cli.clearConsole();

        return CmdExecResult::DONE;
    };

    return cmd;
};
