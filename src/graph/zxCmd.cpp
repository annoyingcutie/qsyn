/****************************************************************************
  FileName     [ zxCmd.cpp ]
  PackageName  [ graph ]
  Synopsis     [ Define zx package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "phase_argparse.h"
// --- include before zxCmd.h
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

#include "cmdMacros.h"
#include "textFormat.h"
#include "zx2tsMapper.h"
#include "zxCmd.h"
#include "zxGraphMgr.h"

namespace TF = TextFormat;

using namespace std;

ZXGraphMgr zxGraphMgr{"ZXGraph"};
using namespace ArgParse;
extern size_t verbose;

unique_ptr<ArgParseCmdType> ZXCHeckoutCmd();
unique_ptr<ArgParseCmdType> ZXNewCmd();
unique_ptr<ArgParseCmdType> ZXResetCmd();
unique_ptr<ArgParseCmdType> ZXDeleteCmd();
unique_ptr<ArgParseCmdType> ZXPrintCmd();
unique_ptr<ArgParseCmdType> ZXCopyCmd();
unique_ptr<ArgParseCmdType> ZXComposeCmd();
unique_ptr<ArgParseCmdType> ZXTensorCmd();
unique_ptr<ArgParseCmdType> ZXGTraverseCmd();
unique_ptr<ArgParseCmdType> ZX2TSCmd();
unique_ptr<ArgParseCmdType> ZXGADjointCmd();
unique_ptr<ArgParseCmdType> ZXGTestCmd();
unique_ptr<ArgParseCmdType> ZXGDrawCmd();
unique_ptr<ArgParseCmdType> ZXGEditCmd();
// unique_ptr<ArgParseCmdType> ZXGPrintCmd();

// unique_ptr<ArgParseCmdType> ZXGWriteCmd();

bool initZXCmd() {
    if (!(cmdMgr->regCmd("ZXCHeckout", 4, ZXCHeckoutCmd()) &&
          cmdMgr->regCmd("ZXNew", 3, ZXNewCmd()) &&
          cmdMgr->regCmd("ZXReset", 3, ZXResetCmd()) &&
          cmdMgr->regCmd("ZXDelete", 3, ZXDeleteCmd()) &&
          cmdMgr->regCmd("ZXCOPy", 5, ZXCopyCmd()) &&
          cmdMgr->regCmd("ZXCOMpose", 5, ZXComposeCmd()) &&
          cmdMgr->regCmd("ZXTensor", 3, ZXTensorCmd()) &&
          cmdMgr->regCmd("ZXPrint", 3, ZXPrintCmd()) &&
          cmdMgr->regCmd("ZXGPrint", 4, make_unique<ZXGPrintCmd>()) &&
          cmdMgr->regCmd("ZXGTest", 4, ZXGTestCmd()) &&
          cmdMgr->regCmd("ZXGEdit", 4, ZXGEditCmd()) &&
          cmdMgr->regCmd("ZXGADJoint", 6, ZXGADjointCmd()) &&
          cmdMgr->regCmd("ZXGASsign", 5, make_unique<ZXGAssignCmd>()) &&
          cmdMgr->regCmd("ZXGTRaverse", 5, ZXGTraverseCmd()) &&
          cmdMgr->regCmd("ZXGDraw", 4, ZXGDrawCmd()) &&
          cmdMgr->regCmd("ZX2TS", 5, ZX2TSCmd()) &&
          cmdMgr->regCmd("ZXGRead", 4, make_unique<ZXGReadCmd>()) &&
          cmdMgr->regCmd("ZXGWrite", 4, make_unique<ZXGWriteCmd>()))) {
        cerr << "Registering \"zx\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

ArgType<size_t>::ConstraintType const validZXGraphId = {
    [](size_t const &id) {
        return zxGraphMgr.isID(id);
    },
    [](size_t const &id) {
        cerr << "Error: ZXGraph " << id << " does not exist!!\n";
    }};

ArgType<size_t>::ConstraintType const validZXVertexId = {
    [](size_t const &id) {
        return zxGraphMgr.get()->isId(id);
    },
    [](size_t const &id) {
        cerr << "Error: Cannot find vertex with ID " << id << " in the ZXGraph!!\n";
    }};

ArgType<size_t>::ConstraintType const notExistingZXInputQubitId = {
    [](size_t const &qid) {
        return !zxGraphMgr.get()->isInputQubit(qid);
    },
    [](size_t const &qid) {
        cerr << "Error: This qubit's input already exists!!\n";
    }};

ArgType<size_t>::ConstraintType const notExistingZXOutputQubitId = {
    [](size_t const &qid) {
        return !zxGraphMgr.get()->isOutputQubit(qid);
    },
    [](size_t const &qid) {
        cerr << "Error: This qubit's output already exists!!\n";
    }};

bool zxGraphMgrNotEmpty(std::string const &command) {
    if (zxGraphMgr.empty()) {
        cerr << "Error: ZXGraph list is empty now. Please ZXNew before " << command << ".\n";
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
//    ZXCHeckout <(size_t id)>
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXCHeckoutCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXCHeckout");
    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("checkout to Graph <id> in ZXGraphMgr");
        parser.addArgument<size_t>("id")
            .constraint(validZXGraphId)
            .help("the ID of the ZXGraph");
    };
    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.checkout(parser["id"]);
        return CMD_EXEC_DONE;
    };
    return cmd;
}

//----------------------------------------------------------------------
//    ZXNew [(size_t id)]
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXNewCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXNew");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("create a new ZXGraph to ZXGraphMgr");

        parser.addArgument<size_t>("id")
            .nargs(NArgsOption::OPTIONAL)
            .help("the ID of the ZXGraph");

        parser.addArgument<bool>("-Replace")
            .action(storeTrue)
            .help("if specified, replace the current ZXGraph; otherwise store to a new one");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        size_t id = (parser["id"].isParsed()) ? parser.get<size_t>("id") : zxGraphMgr.getNextID();

        if (zxGraphMgr.isID(id)) {
            if (!parser["-Replace"].isParsed()) {
                cerr << "Error: ZXGraph " << id << " already exists!! Specify `-Replace` if needed." << endl;
                return CMD_EXEC_ERROR;
            }
            zxGraphMgr.set(make_unique<ZXGraph>(id));
            return CMD_EXEC_DONE;
        }

        zxGraphMgr.add(id);
        return CMD_EXEC_DONE;
    };
    return cmd;
}

//----------------------------------------------------------------------
//    ZXReset
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXResetCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXReset");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("reset ZXGraphMgr");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.reset();
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXDelete <(size_t id)>
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXDeleteCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXDelete");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("remove a ZXGraph from ZXGraphMgr");

        parser.addArgument<size_t>("id")
            .constraint(validZXGraphId)
            .help("the ID of the ZXGraph");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.remove(parser["id"]);
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXPrint [-Summary | -Focus | -Num]
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXPrintCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXPrint");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("print info of ZXGraphMgr");
        auto mutex = parser.addMutuallyExclusiveGroup().required(false);

        mutex.addArgument<bool>("-summary")
            .action(storeTrue)
            .help("print summary of all ZXGraphs");
        mutex.addArgument<bool>("-focus")
            .action(storeTrue)
            .help("print the info of the ZXGraph in focus");
        mutex.addArgument<bool>("-list")
            .action(storeTrue)
            .help("print a list of ZXGraph");
        mutex.addArgument<bool>("-number")
            .action(storeTrue)
            .help("print the number of ZXGraph managed");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        if (parser["-focus"].isParsed())
            zxGraphMgr.printFocus();
        else if (parser["-number"].isParsed())
            zxGraphMgr.printListSize();
        else if (parser["-list"].isParsed())
            zxGraphMgr.printList();
        else
            zxGraphMgr.printMgr();
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXCOPy [(size_t id)]
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXCopyCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXCOPy");

    cmd->precondition = []() { return zxGraphMgrNotEmpty("ZXCOPy"); };

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("copy a ZXGraph to ZXGraphMgr");

        parser.addArgument<size_t>("id")
            .nargs(NArgsOption::OPTIONAL)
            .help("the ID copied ZXGraph to be stored");

        parser.addArgument<bool>("-Replace")
            .defaultValue(false)
            .action(storeTrue)
            .help("replace the current focused ZXGraph");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        size_t id = (parser["id"].isParsed()) ? parser.get<size_t>("id") : zxGraphMgr.getNextID();
        if (zxGraphMgr.isID(id)) {
            if (!parser["-Replace"].isParsed()) {
                cerr << "Error: ZXGraph " << id << " already exists!! Specify `-Replace` if needed." << endl;
                return CMD_EXEC_ERROR;
            }
            zxGraphMgr.copy(id, false);
            return CMD_EXEC_DONE;
        }

        zxGraphMgr.copy(id);
        return CMD_EXEC_DONE;
    };
    return cmd;
}

//----------------------------------------------------------------------
//    ZXCOMpose <size_t id>
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXComposeCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXCOMpose");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("compose a ZXGraph");

        parser.addArgument<size_t>("id")
            .constraint(validZXGraphId)
            .help("the ID of the ZXGraph to compose with");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.get()->compose(*zxGraphMgr.findByID(parser["id"]));
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXTensor <size_t id>
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXTensorCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXTensor");
    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("tensor a ZXGraph");
        parser.addArgument<size_t>("id")
            .constraint(validZXGraphId)
            .help("the ID of the ZXGraph");
    };
    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.get()->tensorProduct(*zxGraphMgr.findByID(parser["id"]));
        return CMD_EXEC_DONE;
    };
    return cmd;
}

//----------------------------------------------------------------------
//    ZXGTest [-Empty | -Valid | -GLike | -IDentity]
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXGTestCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXGTest");

    cmd->precondition = []() { return zxGraphMgrNotEmpty("ZXGTest"); };

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("test ZXGraph structures and functions");

        auto mutex = parser.addMutuallyExclusiveGroup().required(true);

        mutex.addArgument<bool>("-empty")
            .action(storeTrue)
            .help("check if the ZXGraph is empty");
        mutex.addArgument<bool>("-valid")
            .action(storeTrue)
            .help("check if the ZXGraph is valid");
        mutex.addArgument<bool>("-glike")
            .action(storeTrue)
            .help("check if the ZXGraph is graph-like");
        mutex.addArgument<bool>("-identity")
            .action(storeTrue)
            .help("check if the ZXGraph is equivalent to identity");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        if (parser["-empty"].isParsed()) {
            if (zxGraphMgr.get()->isEmpty())
                cout << "The graph is empty!" << endl;
            else
                cout << "The graph is not empty!" << endl;
        } else if (parser["-valid"].isParsed()) {
            if (zxGraphMgr.get()->isValid())
                cout << "The graph is valid!" << endl;
            else
                cout << "The graph is invalid!" << endl;
        } else if (parser["-glike"].isParsed()) {
            if (zxGraphMgr.get()->isGraphLike())
                cout << "The graph is graph-like!" << endl;
            else
                cout << "The graph is not graph-like!" << endl;
        } else if (parser["-identity"].isParsed()) {
            if (zxGraphMgr.get()->isIdentity())
                cout << "The graph is an identity!" << endl;
            else
                cout << "The graph is not an identity!" << endl;
        }
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//-----------------------------------------------------------------------------------------------------------
//    ZXGPrint [-Summary | -Inputs | -Outputs | -Vertices | -Edges | -Qubits | -Neighbors | -Analysis | -Density]
//-----------------------------------------------------------------------------------------------------------
// unique_ptr<ArgParseCmdType> ZXGPrintCmd() {
//     auto cmd = make_unique<ArgParseCmdType>("ZXGPrint");

//     cmd->parserDefinition = [](ArgumentParser &parser) {
//         parser.help("print info of ZXGraph");

//         auto mutex = parser.addMutuallyExclusiveGroup();

//         mutex.addArgument<bool>("-summary")
//             .action(storeTrue)
//             .help("print the summary info of ZXGraph");
//         mutex.addArgument<bool>("-io")
//             .action(storeTrue)
//             .help("print the I/O info of ZXGraph");
//         mutex.addArgument<bool>("-inputs")
//             .action(storeTrue)
//             .help("print the inputs info of ZXGraph");
//         mutex.addArgument<bool>("-outputs")
//             .action(storeTrue)
//             .help("print the outputs info of ZXGraph");
//         mutex.addArgument<bool>("-vertices")
//             .action(storeTrue)
//             .help("print the vertices info of ZXGraph");
//         mutex.addArgument<bool>("-edges")
//             .action(storeTrue)
//             .help("print the edges info of ZXGraph");
//         mutex.addArgument<bool>("-qubits")
//             .action(storeTrue)
//             .help("print the qubits info of ZXGraph");
//         mutex.addArgument<bool>("-neighbors")
//             .action(storeTrue)
//             .help("print the neighbors info of ZXGraph");
//         mutex.addArgument<bool>("-density")
//             .action(storeTrue)
//             .help("print the density of ZXGraph");
//     };

//     cmd->onParseSuccess = [](ArgumentParser const &parser) {
//         ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGPrint");
//         //TODO - `-vertices`, `-qubits`, `-neighbors` specific printing
//         if(parser["-summary"].isParsed()){
//             zxGraphMgr->getGraph()->printGraph();
//             cout << setw(30) << left << "#T-gate: " << zxGraphMgr->getGraph()->TCount() << "\n";
//             cout << setw(30) << left << "#Non-(Clifford+T)-gate: " << zxGraphMgr->getGraph()->nonCliffordCount(false) << "\n";
//             cout << setw(30) << left << "#Non-Clifford-gate: " << zxGraphMgr->getGraph()->nonCliffordCount(true) << "\n";
//         }
//         else if(parser["-io"].isParsed()) zxGraphMgr->getGraph()->printIO();
//         else if(parser["-inputs"].isParsed()) zxGraphMgr->getGraph()->printInputs();
//         else if(parser["-outputs"].isParsed()) zxGraphMgr->getGraph()->printOutputs();
//         else if(parser["-vertices"].isParsed()) zxGraphMgr->getGraph()->printVertices();
//         else if(parser["-edges"].isParsed()) zxGraphMgr->getGraph()->printEdges();
//         else if(parser["-qubits"].isParsed()) zxGraphMgr->getGraph()->printQubits();
//         else if(parser["-neighbors"].isParsed()){}
//         else if(parser["-density"].isParsed()){
//             cout << "Density: " << zxGraphMgr->getGraph()->Density() << endl;
//         }
//         else zxGraphMgr->getGraph()->printGraph();
//         return CMD_EXEC_DONE;
//     };

//     return cmd;
// }

CmdExecStatus
ZXGPrintCmd::exec(std::stop_token, const string &option) {
    // check option
    vector<string> options;
    if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;
    // string token;
    // if (!CmdExec::lexSingleOption(option, token)) return CMD_EXEC_ERROR;

    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGPrint");

    if (options.empty())
        zxGraphMgr.get()->printGraph();
    else if (myStrNCmp("-Summary", options[0], 2) == 0) {
        zxGraphMgr.get()->printGraph();
        cout << setw(30) << left << "#T-gate: " << zxGraphMgr.get()->TCount() << "\n";
        cout << setw(30) << left << "#Non-(Clifford+T)-gate: " << zxGraphMgr.get()->nonCliffordCount() << "\n";
        cout << setw(30) << left << "#Non-Clifford-gate: " << zxGraphMgr.get()->nonCliffordPlusTCount() << "\n";
    } else if (myStrNCmp("-Inputs", options[0], 2) == 0)
        zxGraphMgr.get()->printInputs();
    else if (myStrNCmp("-Outputs", options[0], 2) == 0)
        zxGraphMgr.get()->printOutputs();
    else if (myStrNCmp("-IO", options[0], 3) == 0)
        zxGraphMgr.get()->printIO();
    else if (myStrNCmp("-Vertices", options[0], 2) == 0) {
        if (options.size() == 1)
            zxGraphMgr.get()->printVertices();
        else {
            vector<size_t> candidates;
            for (size_t i = 1; i < options.size(); i++) {
                unsigned id;
                if (myStr2Uns(options[i], id)) candidates.emplace_back(id);
            }
            zxGraphMgr.get()->printVertices(candidates);
        }
    } else if (myStrNCmp("-Edges", options[0], 2) == 0)
        zxGraphMgr.get()->printEdges();
    else if (myStrNCmp("-Qubits", options[0], 2) == 0) {
        vector<int> candidates;
        for (size_t i = 1; i < options.size(); i++) {
            int qid;
            if (myStr2Int(options[i], qid))
                candidates.emplace_back(qid);
            else {
                cout << "Warning: " << options[i] << " is not a valid qubit ID!!" << endl;
            }
        }
        zxGraphMgr.get()->printQubits(candidates);
    } else if (myStrNCmp("-Neighbors", options[0], 2) == 0) {
        CMD_N_OPTS_EQUAL_OR_RETURN(options, 2);

        unsigned id;
        ZXVertex *v;
        ZX_CMD_ID_VALID_OR_RETURN(options[1], id, "Vertex");
        ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id, v);

        v->printVertex();
        cout << "----- Neighbors -----" << endl;
        for (auto [nb, _] : v->getNeighbors()) {
            nb->printVertex();
        }
    } else if (myStrNCmp("-Analysis", options[0], 2) == 0) {
        cout << setw(30) << left << "#T-gate: " << zxGraphMgr.get()->TCount() << "\n";
        cout << setw(30) << left << "#Non-(Clifford+T)-gate: " << zxGraphMgr.get()->nonCliffordCount() << "\n";
        cout << setw(30) << left << "#Non-Clifford-gate: " << zxGraphMgr.get()->nonCliffordPlusTCount() << "\n";
        return CMD_EXEC_DONE;
    }

    else
        return errorOption(CMD_OPT_ILLEGAL, options[0]);
    return CMD_EXEC_DONE;
}

void ZXGPrintCmd::usage() const {
    cout << "Usage: ZXGPrint [-Summary | -Inputs | -Outputs | -Vertices | -Edges | -Qubits | -Neighbors | -Analysis]" << endl;
}

void ZXGPrintCmd::summary() const {
    cout << setw(15) << left << "ZXGPrint: "
         << "print info of ZXGraph" << endl;
}

unique_ptr<ArgParseCmdType> ZXGEditCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXGEdit");

    cmd->precondition = []() { return zxGraphMgrNotEmpty("ZXGEdit"); };

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("edit ZXGraph");

        auto subparsers = parser.addSubParsers().required(true);

        auto removeVertexParser = subparsers.addParser("-rmvertex");

        removeVertexParser.addArgument<size_t>("ids")
            .constraint(validZXVertexId)
            .nargs(NArgsOption::ZERO_OR_MORE)
            .help("the IDs of vertices to remove");

        removeVertexParser.addArgument<bool>("-isolated")
            .action(storeTrue)
            .help("if set, remove all isolated vertices");

        auto removeEdgeParser = subparsers.addParser("-rmedge");

        removeEdgeParser.addArgument<size_t>("ids")
            .nargs(2)
            .constraint(validZXVertexId)
            .metavar("(vs, vt)")
            .help("the IDs to the two vertices to remove edges in between");

        removeEdgeParser.addArgument<string>("etype")
            .constraint(choices_allow_prefix({"simple", "hadamard", "all"}))
            .help("the edge type to remove. Options: simple, hadamard, all (i.e., remove both)");

        auto addVertexParser = subparsers.addParser("-addvertex");

        addVertexParser.addArgument<size_t>("qubit")
            .help("the qubit ID the ZXVertex belongs to");

        addVertexParser.addArgument<string>("vtype")
            .constraint(choices_allow_prefix({"zspider", "xspider", "hbox"}))
            .help("the type of ZXVertex");

        addVertexParser.addArgument<Phase>("phase")
            .nargs(NArgsOption::OPTIONAL)
            .defaultValue(Phase(0))
            .help("phase of the ZXVertex (default = 0)");

        auto addInputParser = subparsers.addParser("-addinput");

        addInputParser.addArgument<size_t>("qubit")
            .constraint(notExistingZXInputQubitId)
            .help("the qubit ID of the input");

        auto addOutputParser = subparsers.addParser("-addoutput");

        addOutputParser.addArgument<size_t>("qubit")
            .constraint(notExistingZXOutputQubitId)
            .help("the qubit ID of the output");

        auto addEdgeParser = subparsers.addParser("-addedge");

        addEdgeParser.addArgument<size_t>("ids")
            .nargs(2)
            .constraint(validZXVertexId)
            .metavar("(vs, vt)")
            .help("the IDs to the two vertices to add edges in between");

        addEdgeParser.addArgument<string>("etype")
            .constraint(choices_allow_prefix({"simple", "hadamard"}))
            .help("the edge type to add. Options: simple, hadamard");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        std::string subparser = parser.getActivatedSubParserName();

        if (subparser == "-rmvertex") {
            auto ids = parser.get<vector<size_t>>("ids");
            auto vertices_range = ids |
                                  views::transform([](size_t id) { return zxGraphMgr.get()->findVertexById(id); }) |
                                  views::filter([](ZXVertex *v) { return v != nullptr; });
            zxGraphMgr.get()->removeVertices({vertices_range.begin(), vertices_range.end()});

            if (parser["-isolated"].isParsed()) {
                cout << "Note: removing isolated vertices..." << endl;
                zxGraphMgr.get()->removeIsolatedVertices();
            }
            return CMD_EXEC_DONE;
        }
        if (subparser == "-rmedge") {
            auto ids = parser.get<std::vector<size_t>>("ids");
            auto v0 = zxGraphMgr.get()->findVertexById(ids[0]);
            auto v1 = zxGraphMgr.get()->findVertexById(ids[1]);
            assert(v0 != nullptr && v1 != nullptr);

            auto etype = std::invoke([&parser]() {
                auto str = parser.get<std::string>("etype");
                switch (std::tolower(str[0])) {
                    case 's':
                        return EdgeType::SIMPLE;
                    case 'h':
                        return EdgeType::HADAMARD;
                    default:
                        return EdgeType::ERRORTYPE;
                }
            });

            if (etype == EdgeType::ERRORTYPE) {  // corresponds to choice "ALL"
                zxGraphMgr.get()->removeAllEdgesBetween(v0, v1);
            } else {
                zxGraphMgr.get()->removeEdge(v0, v1, etype);
            }

            return CMD_EXEC_DONE;
        }
        if (subparser == "-addvertex") {
            auto vtype = std::invoke([&parser]() {
                auto vtypeStr = parser.get<std::string>("vtype");
                switch (std::tolower(vtypeStr[0])) {
                    case 'z':
                        return VertexType::Z;
                    case 'x':
                        return VertexType::X;
                    case 'h':
                        return VertexType::H_BOX;
                    default:
                        return VertexType::ERRORTYPE;
                }
            });
            assert(vtype != VertexType::ERRORTYPE);

            zxGraphMgr.get()->addVertex(parser.get<size_t>("qubit"), vtype, parser.get<Phase>("phase"));

            return CMD_EXEC_DONE;
        }
        if (subparser == "-addinput") {
            zxGraphMgr.get()->addInput(parser.get<size_t>("qubit"));
            return CMD_EXEC_DONE;
        }
        if (subparser == "-addoutput") {
            zxGraphMgr.get()->addOutput(parser.get<size_t>("qubit"));
            return CMD_EXEC_DONE;
        }
        if (subparser == "-addedge") {
            auto ids = parser.get<std::vector<size_t>>("ids");
            auto v0 = zxGraphMgr.get()->findVertexById(ids[0]);
            auto v1 = zxGraphMgr.get()->findVertexById(ids[1]);
            assert(v0 != nullptr && v1 != nullptr);

            auto etype = std::invoke([&parser]() {
                auto str = parser.get<std::string>("etype");
                switch (std::tolower(str[0])) {
                    case 's':
                        return EdgeType::SIMPLE;
                    case 'h':
                        return EdgeType::HADAMARD;
                    default:
                        return EdgeType::ERRORTYPE;
                }
            });
            assert(etype != EdgeType::ERRORTYPE);

            zxGraphMgr.get()->addEdge(v0, v1, etype);

            return CMD_EXEC_DONE;
        }
        return CMD_EXEC_ERROR;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXGTRaverse
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXGTraverseCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXGTRaverse");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("traverse ZXGraph and update topological order of vertices");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.get()->updateTopoOrder();
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXGDraw [-CLI]
//    ZXGDraw <string (path.pdf)>
//----------------------------------------------------------------------

unique_ptr<ArgParseCmdType> ZXGDrawCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXGDraw");

    cmd->precondition = []() { return zxGraphMgrNotEmpty("ZXGDraw"); };

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("draw ZXGraph");

        parser.addArgument<string>("filepath")
            .nargs(NArgsOption::OPTIONAL)
            .constraint(dir_for_file_exists)
            .constraint(allowed_extension({".pdf"}))
            .help("the output path. Supported extension: .pdf");

        parser.addArgument<bool>("-CLI")
            .help("print to the console. Note that only horizontal wires will be printed");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        if (parser["filename"].isParsed()) {
            if (!zxGraphMgr.get()->writePdf(parser["filename"])) return CMD_EXEC_ERROR;
        }
        if (parser["-CLI"].isParsed()) {
            zxGraphMgr.get()->draw();
        }

        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZX2TS
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZX2TSCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZX2TS");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("convert ZXGraph to tensor");
    };

    cmd->onParseSuccess = [](std::stop_token st, ArgumentParser const &parser) {
        ZX2TSMapper mapper{zxGraphMgr.get(), st};
        mapper.map();
        return CMD_EXEC_DONE;
    };

    return cmd;
}

//----------------------------------------------------------------------
//    ZXGRead <string Input.(b)zx> [-KEEPid] [-Replace]
//----------------------------------------------------------------------
CmdExecStatus
ZXGReadCmd::exec(std::stop_token, const string &option) {  // check option
    vector<string> options;

    if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;

    CMD_N_OPTS_BETWEEN_OR_RETURN(options, 1, 3);

    bool doReplace = false;
    bool doKeepID = false;
    size_t eraseIndexReplace = 0;
    size_t eraseIndexBZX = 0;
    string fileName = "";
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        if (myStrNCmp("-Replace", options[i], 2) == 0) {
            if (doReplace)
                return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
            doReplace = true;
            eraseIndexReplace = i;
        } else if (myStrNCmp("-KEEPid", options[i], 5) == 0) {
            if (doKeepID)
                return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
            doKeepID = true;
            eraseIndexBZX = i;
        } else {
            if (fileName.size())
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
            fileName = options[i];
        }
    }
    string replaceStr = options[eraseIndexReplace];
    string bzxStr = options[eraseIndexBZX];
    if (doReplace)
        std::erase(options, replaceStr);
    if (doKeepID)
        std::erase(options, bzxStr);
    if (options.empty())
        return CmdExec::errorOption(CMD_OPT_MISSING, (eraseIndexBZX > eraseIndexReplace) ? bzxStr : replaceStr);

    auto bufferGraph = make_unique<ZXGraph>();
    if (!bufferGraph->readZX(fileName, doKeepID)) {
        // REVIEW - This error message is not always accurate
        // cerr << "Error: The format in \"" << fileName << "\" has something wrong!!" << endl;
        return CMD_EXEC_ERROR;
    }

    if (doReplace) {
        if (zxGraphMgr.empty()) {
            cout << "Note: ZXGraph list is empty now. Create a new one." << endl;
            zxGraphMgr.add(zxGraphMgr.getNextID());
        } else {
            cout << "Note: original ZXGraph is replaced..." << endl;
        }
    } else {
        zxGraphMgr.add(zxGraphMgr.getNextID());
    }
    zxGraphMgr.set(std::move(bufferGraph));
    return CMD_EXEC_DONE;
}

void ZXGReadCmd::usage() const {
    cout << "Usage: ZXGRead <string Input.(b)zx> [-KEEPid] [-Replace]" << endl;
}

void ZXGReadCmd::summary() const {
    cout << setw(15) << left << "ZXGRead: "
         << "read a file and construct the corresponding ZXGraph" << endl;
}

//----------------------------------------------------------------------
//    ZXGWrite <string Output.<zx | tikz | tex>> [-Complete]
//----------------------------------------------------------------------
CmdExecStatus
ZXGWriteCmd::exec(std::stop_token, const string &option) {
    vector<string> options;

    if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;
    if (options.empty()) return CmdExec::errorOption(CMD_OPT_MISSING, "");

    bool doComplete = false;
    size_t eraseIndexComplete = 0;
    string fileName;
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        if (myStrNCmp("-Complete", options[i], 2) == 0) {
            if (doComplete)
                return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
            doComplete = true;
            eraseIndexComplete = i;
        } else {
            if (fileName.size())
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
            fileName = options[i];
        }
    }
    string completeStr = options[eraseIndexComplete];
    if (doComplete)
        std::erase(options, completeStr);
    if (options.empty())
        return CmdExec::errorOption(CMD_OPT_MISSING, completeStr);

    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXWrite");

    size_t extensionPosition = fileName.find_last_of(".");
    // REVIEW - should we guard the case of no file extension?
    if (extensionPosition != string::npos) {
        string extensionString = fileName.substr(extensionPosition);
        if (
            myStrNCmp(".zx", extensionString, 3) == 0 ||
            myStrNCmp(".bzx", extensionString, 4) == 0) {
            if (!zxGraphMgr.get()->writeZX(fileName, doComplete)) {
                cerr << "Error: fail to write ZXGraph to \"" << fileName << "\"!!" << endl;
                return CMD_EXEC_ERROR;
            }
        } else if (myStrNCmp(".tikz", extensionString, 5) == 0) {
            if (!zxGraphMgr.get()->writeTikz(fileName)) {
                cerr << "Error: fail to write Tikz to \"" << fileName << "\"!!" << endl;
                return CMD_EXEC_ERROR;
            }
        } else if (myStrNCmp(".tex", extensionString, 4) == 0) {
            if (!zxGraphMgr.get()->writeTex(fileName)) {
                cerr << "Error: fail to write tex to \"" << fileName << "\"!!" << endl;
                return CMD_EXEC_ERROR;
            }
        }
    } else {
        if (!zxGraphMgr.get()->writeZX(fileName, doComplete)) {
            cerr << "Error: fail to write ZXGraph to \"" << fileName << "\"!!" << endl;
            return CMD_EXEC_ERROR;
        }
    }

    return CMD_EXEC_DONE;
}

void ZXGWriteCmd::usage() const {
    cout << "Usage: ZXGWrite <string Output.<zx | tikz>> [-Complete]" << endl;
}

void ZXGWriteCmd::summary() const {
    cout << setw(15) << left << "ZXGWrite: "
         << "write a ZXGraph to a file\n";
}

//----------------------------------------------------------------------
//    ZXGASsign <size_t qubit> <I|O> <VertexType vt> <string Phase>
//----------------------------------------------------------------------
CmdExecStatus
ZXGAssignCmd::exec(std::stop_token, const string &option) {
    // check option
    vector<string> options;
    if (!CmdExec::lexOptions(option, options))
        return CMD_EXEC_ERROR;

    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGASsign");

    CMD_N_OPTS_EQUAL_OR_RETURN(options, 4);
    int qid;
    ZX_CMD_QUBIT_ID_VALID_OR_RETURN(options[0], qid);

    bool isInput;
    if (options[1] == "I") {
        isInput = true;
    } else if (options[1] == "O") {
        isInput = false;
    } else {
        cerr << "Error: a boundary must be either \"I\" or \"O\"!!" << endl;
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
    }

    if (!(isInput ? zxGraphMgr.get()->isInputQubit(qid) : zxGraphMgr.get()->isOutputQubit(qid))) {
        cerr << "Error: the specified boundary does not exist!!" << endl;
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
    }

    VertexType vt;
    ZX_CMD_VERTEX_TYPE_VALID_OR_RETURN(options[2], vt);

    Phase phase;
    ZX_CMD_PHASE_VALID_OR_RETURN(options[3], phase);

    zxGraphMgr.get()->assignBoundary(qid, (options[1] == "I"), vt, phase);
    return CMD_EXEC_DONE;
}

void ZXGAssignCmd::usage() const {
    cout << "Usage: ZXGASsign <size_t qubit> <I|O> <VertexType vt> <string Phase>" << endl;
}

void ZXGAssignCmd::summary() const {
    cout << setw(15) << left << "ZXGASsign: "
         << "assign quantum states to input/output vertex\n";
}

//----------------------------------------------------------------------
//    ZXGADJoint
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXGADjointCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXGADjoint");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("adjoint ZXGraph");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr.get()->adjoint();
        return CMD_EXEC_DONE;
    };

    return cmd;
}
