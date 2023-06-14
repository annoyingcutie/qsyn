/****************************************************************************
  FileName     [ zxCmd.cpp ]
  PackageName  [ graph ]
  Synopsis     [ Define zx package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "zxCmd.h"

#include <cstddef>  // for size_t
#include <iomanip>
#include <iostream>
#include <string>

#include "apCmd.h"
#include "cmdMacros.h"   // for CMD_N_OPTS_EQUAL_OR_RETURN, CMD_N_OPTS_AT_LE...
#include "phase.h"      // for Phase
#include "textFormat.h"  // for TextFormat
#include "zxGraph.h"     // for ZXGraph, ZXVertex
#include "zxGraphMgr.h"  // for ZXGraphMgr

namespace TF = TextFormat;

using namespace std;
using namespace ArgParse;
extern ZXGraphMgr *zxGraphMgr;
extern size_t verbose;

// unique_ptr<ArgParseCmdType> ZXCHeckoutCmd();
unique_ptr<ArgParseCmdType> ZXNewCmd();
unique_ptr<ArgParseCmdType> ZXResetCmd();
unique_ptr<ArgParseCmdType> ZXDeleteCmd();
unique_ptr<ArgParseCmdType> ZXPrintCmd();
unique_ptr<ArgParseCmdType> ZXCopyCmd();
unique_ptr<ArgParseCmdType> ZXComposeCmd();
// unique_ptr<ArgParseCmdType> ZXGWriteCmd();

bool initZXCmd() {
    zxGraphMgr = new ZXGraphMgr;
    if (!(cmdMgr->regCmd("ZXCHeckout", 4, make_unique<ZXCHeckoutCmd>()) &&
          cmdMgr->regCmd("ZXNew", 3, ZXNewCmd()) &&
          cmdMgr->regCmd("ZXReset", 3, ZXResetCmd()) &&
          cmdMgr->regCmd("ZXDelete", 3, ZXDeleteCmd()) &&
          cmdMgr->regCmd("ZXCOPy", 5, ZXCopyCmd()) &&
          cmdMgr->regCmd("ZXCOMpose", 5, ZXComposeCmd()) &&
          cmdMgr->regCmd("ZXTensor", 3, make_unique<ZXTensorCmd>()) &&
          cmdMgr->regCmd("ZXPrint", 3, ZXPrintCmd()) &&
          cmdMgr->regCmd("ZXGPrint", 4, make_unique<ZXGPrintCmd>()) &&
          cmdMgr->regCmd("ZXGTest", 4, make_unique<ZXGTestCmd>()) &&
          cmdMgr->regCmd("ZXGEdit", 4, make_unique<ZXGEditCmd>()) &&
          cmdMgr->regCmd("ZXGADJoint", 6, make_unique<ZXGAdjointCmd>()) &&
          cmdMgr->regCmd("ZXGASsign", 5, make_unique<ZXGAssignCmd>()) &&
          cmdMgr->regCmd("ZXGTRaverse", 5, make_unique<ZXGTraverseCmd>()) &&
          cmdMgr->regCmd("ZXGDraw", 4, make_unique<ZXGDrawCmd>()) &&
          cmdMgr->regCmd("ZX2TS", 5, make_unique<ZX2TSCmd>()) &&
          cmdMgr->regCmd("ZXGRead", 4, make_unique<ZXGReadCmd>()) &&
          cmdMgr->regCmd("ZXGWrite", 4, make_unique<ZXGWriteCmd>())
          )) {
        cerr << "Registering \"zx\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

ArgType<size_t>::ConstraintType validZXGraphId = {
    [](ArgType<size_t> &arg) {
        return [&arg]() {
            return zxGraphMgr->isID(arg.getValue());
        };
    },
    [](ArgType<size_t> const &arg) {
        return [&arg]() {
            cerr << "Error: ZXGraph " << arg.getValue() << " does not exist!!\n";
        };
    }};





//----------------------------------------------------------------------
//    ZXCHeckout <(size_t id)>
//----------------------------------------------------------------------

CmdExecStatus
ZXCHeckoutCmd::exec(const string &option) {
    string token;
    if (!CmdExec::lexSingleOption(option, token)) return CMD_EXEC_ERROR;

    if (token.empty())
        return CmdExec::errorOption(CMD_OPT_MISSING, "");
    else {
        unsigned id;
        ZX_CMD_ID_VALID_OR_RETURN(token, id, "Graph");
        ZX_CMD_GRAPH_ID_EXISTS_OR_RETURN(id);
        zxGraphMgr->checkout2ZXGraph(id);
    }
    return CMD_EXEC_DONE;
}

void ZXCHeckoutCmd::usage() const {
    cout << "Usage: ZXCHeckout <(size_t id)>" << endl;
}

void ZXCHeckoutCmd::summary() const {
    cout << setw(15) << left << "ZXCHeckout: "
         << "checkout to Graph <id> in ZXGraphMgr" << endl;
}


//----------------------------------------------------------------------
//    ZXNew [(size_t id)]
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXNewCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXNew");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("create a new ZX-graph to ZXGraphMgr");

        parser.addArgument<size_t>("id")
            .required(false)
            .help("the ID of the ZX-graph");
        
        parser.addArgument<bool>("-Replace")
            .action(storeTrue)
            .help("if specified, replace the current ZX-graph; otherwise store to a new one");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        if (parser["id"].isParsed()) {
            if (zxGraphMgr->isID(parser["id"])) {
                if (parser["-Replace"].isParsed()) {
                    size_t repId = parser["id"];
                    ZXGraph *zxgraph = new ZXGraph(repId);
                    zxGraphMgr->setGraph(zxgraph);
                } else
                    cerr << "Error: ZXGraph " << parser["id"] << " already exists!! Specify `-Replace` if needed." << endl;
            } else
                zxGraphMgr->addZXGraph(parser["id"]);
        } else {
            zxGraphMgr->addZXGraph(zxGraphMgr->getNextID());
        }
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
        zxGraphMgr->reset();
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
        parser.help("remove a ZX-graph from ZXGraphMgr");

        parser.addArgument<size_t>("id")
            .constraint(validZXGraphId)
            .help("the ID of the ZX-graph");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr->removeZXGraph(parser["id"]);
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
        auto mutex = parser.addMutuallyExclusiveGroup();

        mutex.addArgument<bool>("-summary")
            .action(storeTrue)
            .help("print summary of all ZX-graph");
        mutex.addArgument<bool>("-focus")
            .action(storeTrue)
            .help("print the info of the ZX-graph in focus");
        mutex.addArgument<bool>("-number")
            .action(storeTrue)
            .help("print number of ZX-graph");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        if(parser["-focus"].isParsed()) zxGraphMgr->printGListItr();
        else if(parser["-number"].isParsed()) zxGraphMgr->printGraphListSize();
        else zxGraphMgr->printZXGraphMgr();
        return CMD_EXEC_DONE;
    };

    return cmd;
}


//----------------------------------------------------------------------
//    ZXCOPy [(size_t id)]
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> ZXCopyCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXCOPy");

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("copy a ZX-graph to ZXGraphMgr");

        parser.addArgument<size_t>("id")
            .required(false)
            .help("the ID copied ZX-graph to be stored");

        parser.addArgument<bool>("-Replace")
            .defaultValue(false)
            .action(storeTrue)
            .help("replace the current focused ZX-graph");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXCOPy");
        if (parser["id"].isParsed()) {
            if (zxGraphMgr->isID(parser["id"])) {
                if (parser["-Replace"].isParsed()) {
                    zxGraphMgr->copy(parser["id"], false);
                } else
                    cerr << "Error: ZXGraph " << parser["id"] << " already exists!! Specify `-Replace` if needed." << endl;
            } else
                zxGraphMgr->copy(parser["id"]);
        } else {
            zxGraphMgr->copy(zxGraphMgr->getNextID());
        }
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
        parser.help("compose a ZX-graph");

        parser.addArgument<size_t>("id")
            .constraint(validZXGraphId)
            .help("the ID of the ZX-graph to compose with");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        zxGraphMgr->getGraph()->compose(zxGraphMgr->findZXGraphByID(parser["id"]));
        return CMD_EXEC_DONE;
    };

    return cmd;
}




//----------------------------------------------------------------------
//    ZXTensor <size_t id>
//----------------------------------------------------------------------
CmdExecStatus
ZXTensorCmd::exec(const string &option) {
    string token;
    if (!CmdExec::lexSingleOption(option, token)) return CMD_EXEC_ERROR;
    if (token.empty()) {
        cerr << "Error: the ZX-graph id you want to tensor must be provided!" << endl;
        return CmdExec::errorOption(CMD_OPT_MISSING, token);
    } else {
        unsigned id;
        ZX_CMD_ID_VALID_OR_RETURN(token, id, "Graph");
        ZX_CMD_GRAPH_ID_EXISTS_OR_RETURN(id);
        zxGraphMgr->getGraph()->tensorProduct(zxGraphMgr->findZXGraphByID(id));
    }

    return CMD_EXEC_DONE;
}

void ZXTensorCmd::usage() const {
    cout << "Usage: ZXTensor <size_t id>" << endl;
}

void ZXTensorCmd::summary() const {
    cout << setw(15) << left << "ZXTensor: "
         << "tensor a ZX-graph" << endl;
}

//----------------------------------------------------------------------
//    ZXGTest [-GCX | -Empty | -Valid | -GLike | -IDentity]
//----------------------------------------------------------------------
CmdExecStatus
ZXGTestCmd::exec(const string &option) {
    // check option
    string token;
    if (!CmdExec::lexSingleOption(option, token)) return CMD_EXEC_ERROR;
    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGTest");

    if (token.empty() || myStrNCmp("-GCX", token, 4) == 0) {
        zxGraphMgr->getGraph()->generateCNOT();
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-Empty", token, 2) == 0) {
        if (zxGraphMgr->getGraph()->isEmpty()) {
            cout << "The graph is empty!" << endl;
        } else {
            cout << "The graph is not empty!" << endl;
        }
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-Valid", token, 2) == 0) {
        if (zxGraphMgr->getGraph()->isValid()) {
            cout << "The graph is valid!" << endl;
        } else {
            cout << "The graph is invalid!" << endl;
        }
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-GLike", token, 3) == 0) {
        if (zxGraphMgr->getGraph()->isGraphLike()) {
            cout << "The graph is graph-like!" << endl;
        } else {
            cout << "The graph is not graph-like!" << endl;
        }
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-IDentity", token, 3) == 0) {
        if (zxGraphMgr->getGraph()->isIdentity()) {
            cout << "The graph is an identity!" << endl;
        } else {
            cout << "The graph is not an identity!" << endl;
        }
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-NGADgets", token, 5) == 0) {
        cout << "The graph has " << zxGraphMgr->getGraph()->numGadgets() << " gadgets" << endl;
        return CMD_EXEC_DONE;
    }

    return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
}

void ZXGTestCmd::usage() const {
    cout << "Usage: ZXGTest [-GCX | -Empty | -Valid | -GLike | -IDentity ]" << endl;
}

void ZXGTestCmd::summary() const {
    cout << setw(15) << left << "ZXGTest: "
         << "test ZX-graph structures and functions" << endl;
}

//-----------------------------------------------------------------------------------------------------------
//    ZXGPrint [-Summary | -Inputs | -Outputs | -Vertices | -Edges | -Qubits | -Neighbors | -Analysis]
//-----------------------------------------------------------------------------------------------------------
CmdExecStatus
ZXGPrintCmd::exec(const string &option) {
    // check option
    vector<string> options;
    if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;
    // string token;
    // if (!CmdExec::lexSingleOption(option, token)) return CMD_EXEC_ERROR;

    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGPrint");

    if (options.empty())
        zxGraphMgr->getGraph()->printGraph();
    else if (myStrNCmp("-Summary", options[0], 2) == 0) {
        zxGraphMgr->getGraph()->printGraph();
        cout << setw(30) << left << "#T-gate: " << zxGraphMgr->getGraph()->TCount() << "\n";
        cout << setw(30) << left << "#Non-(Clifford+T)-gate: " << zxGraphMgr->getGraph()->nonCliffordCount(false) << "\n";
        cout << setw(30) << left << "#Non-Clifford-gate: " << zxGraphMgr->getGraph()->nonCliffordCount(true) << "\n";
    } else if (myStrNCmp("-Inputs", options[0], 2) == 0)
        zxGraphMgr->getGraph()->printInputs();
    else if (myStrNCmp("-Outputs", options[0], 2) == 0)
        zxGraphMgr->getGraph()->printOutputs();
    else if (myStrNCmp("-IO", options[0], 3) == 0)
        zxGraphMgr->getGraph()->printIO();
    else if (myStrNCmp("-Vertices", options[0], 2) == 0) {
        if (options.size() == 1)
            zxGraphMgr->getGraph()->printVertices();
        else {
            vector<size_t> candidates;
            for (size_t i = 1; i < options.size(); i++) {
                unsigned id;
                if (myStr2Uns(options[i], id)) candidates.push_back(id);
            }
            zxGraphMgr->getGraph()->printVertices(candidates);
        }
    } else if (myStrNCmp("-Edges", options[0], 2) == 0)
        zxGraphMgr->getGraph()->printEdges();
    else if (myStrNCmp("-Qubits", options[0], 2) == 0) {
        vector<int> candidates;
        for (size_t i = 1; i < options.size(); i++) {
            int qid;
            if (myStr2Int(options[i], qid))
                candidates.push_back(qid);
            else {
                cout << "Warning: " << options[i] << " is not a valid qubit ID!!" << endl;
            }
        }
        zxGraphMgr->getGraph()->printQubits(candidates);
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
        cout << setw(30) << left << "#T-gate: " << zxGraphMgr->getGraph()->TCount() << "\n";
        cout << setw(30) << left << "#Non-(Clifford+T)-gate: " << zxGraphMgr->getGraph()->nonCliffordCount(false) << "\n";
        cout << setw(30) << left << "#Non-Clifford-gate: " << zxGraphMgr->getGraph()->nonCliffordCount(true) << "\n";
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
         << "print info of ZX-graph" << endl;
}

//------------------------------------------------------------------------------------
//    ZXGEdit -RMVertex <-Isolated | (size_t id)... >
//            -RMEdge <(size_t id_s), (size_t id_t)> <-ALL | (EdgeType et)>
//            -ADDVertex <(size_t qubit), (VertexType vt), [Phase phase]>
//            -ADDInput <(size_t qubit)>
//            -ADDOutput <(size_t qubit)>
//            -ADDEdge <(size_t id_s), (size_t id_t), (EdgeType et)>
//------------------------------------------------------------------------------------
CmdExecStatus
ZXGEditCmd::exec(const string &option) {
    // check option
    vector<string> options;
    if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;

    CMD_N_OPTS_AT_LEAST_OR_RETURN(options, 2);
    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGEdit");

    string action = options[0];
    if (myStrNCmp("-RMVertex", action, 4) == 0) {
        if (myStrNCmp("-Isolated", options[1], 2) == 0) {
            CMD_N_OPTS_AT_MOST_OR_RETURN(options, 2);
            zxGraphMgr->getGraph()->removeIsolatedVertices();
            cout << "Note: removing isolated vertices..." << endl;
            return CMD_EXEC_DONE;
        }

        for (size_t i = 1; i < options.size(); i++) {
            unsigned id;
            if (!myStr2Uns(options[i], id)) {
                cerr << "Warning: invalid vertex ID (" << options[i] << ")!!" << endl;
                continue;
            }
            ZXVertex *v = zxGraphMgr->getGraph()->findVertexById(id);
            if (!v) {
                cerr << "Warning: Cannot find vertex with id " << id << " in the graph!!" << endl;
                continue;
            }
            zxGraphMgr->getGraph()->removeVertex(v);
        }
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-RMEdge", action, 4) == 0) {
        CMD_N_OPTS_EQUAL_OR_RETURN(options, 4);

        unsigned id_s, id_t;
        ZXVertex *vs;
        ZXVertex *vt;
        EdgeType etype;

        ZX_CMD_ID_VALID_OR_RETURN(options[1], id_s, "Vertex");
        ZX_CMD_ID_VALID_OR_RETURN(options[2], id_t, "Vertex");
        ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id_s, vs);
        ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id_t, vt);

        if (myStrNCmp("-ALL", options[3], 4) == 0) {
            zxGraphMgr->getGraph()->removeAllEdgesBetween(vs, vt);
        } else {
            ZX_CMD_EDGE_TYPE_VALID_OR_RETURN(options[3], etype);
            zxGraphMgr->getGraph()->removeEdge(vs, vt, etype);
        }

        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-RMGadget", action, 4) == 0) {
        CMD_N_OPTS_EQUAL_OR_RETURN(options, 2);
        unsigned id;
        ZXVertex *v;
        ZX_CMD_ID_VALID_OR_RETURN(options[1], id, "Vertex");
        ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id, v);
        zxGraphMgr->getGraph()->removeGadget(v);
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-ADDVertex", action, 5) == 0) {
        CMD_N_OPTS_BETWEEN_OR_RETURN(options, 3, 4);

        int qid;
        VertexType vt;
        Phase phase;

        ZX_CMD_QUBIT_ID_VALID_OR_RETURN(options[1], qid);
        ZX_CMD_VERTEX_TYPE_VALID_OR_RETURN(options[2], vt);
        if (options.size() == 4)
            ZX_CMD_PHASE_VALID_OR_RETURN(options[3], phase);

        zxGraphMgr->getGraph()->addVertex(qid, vt, phase);
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-ADDInput", action, 5) == 0) {
        CMD_N_OPTS_EQUAL_OR_RETURN(options, 2);

        int qid;
        ZX_CMD_QUBIT_ID_VALID_OR_RETURN(options[1], qid);

        zxGraphMgr->getGraph()->addInput(qid);
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-ADDOutput", action, 5) == 0) {
        CMD_N_OPTS_EQUAL_OR_RETURN(options, 2);

        int qid;
        ZX_CMD_QUBIT_ID_VALID_OR_RETURN(options[1], qid);

        zxGraphMgr->getGraph()->addOutput(qid);
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-ADDEdge", action, 5) == 0) {
        CMD_N_OPTS_EQUAL_OR_RETURN(options, 4);

        unsigned id_s, id_t;
        ZXVertex *vs;
        ZXVertex *vt;
        EdgeType etype;

        ZX_CMD_ID_VALID_OR_RETURN(options[1], id_s, "Vertex");
        ZX_CMD_ID_VALID_OR_RETURN(options[2], id_t, "Vertex");

        ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id_s, vs);
        ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id_t, vt);

        ZX_CMD_EDGE_TYPE_VALID_OR_RETURN(options[3], etype);

        zxGraphMgr->getGraph()->addEdge(vs, vt, etype);
        return CMD_EXEC_DONE;
    }

    if (myStrNCmp("-ADDGadget", action, 5) == 0) {
        CMD_N_OPTS_AT_LEAST_OR_RETURN(options, 3);
        Phase phase;
        ZX_CMD_PHASE_VALID_OR_RETURN(options[1], phase);

        vector<ZXVertex *> verVec;
        for (size_t i = 2; i < options.size(); i++) {
            unsigned id;
            ZXVertex *v;
            ZX_CMD_ID_VALID_OR_RETURN(options[i], id, "Vertex");
            ZX_CMD_VERTEX_ID_IN_GRAPH_OR_RETURN(id, v);
            verVec.push_back(v);
        }
        zxGraphMgr->getGraph()->addGadget(phase, verVec);
        return CMD_EXEC_DONE;
    }

    return errorOption(CMD_OPT_ILLEGAL, action);
}

void ZXGEditCmd::usage() const {
    cout << "Usage: ZXGEdit -RMVertex <-Isolated | (size_t id)... >" << endl;
    cout << "               -RMEdge <(size_t id_s), (size_t id_t)> <-ALL | (EdgeType et)>" << endl;
    cout << "               -ADDVertex <(size_t qubit), (VertexType vt), [Phase phase]>" << endl;
    cout << "               -ADDInput <(size_t qubit)>" << endl;
    cout << "               -ADDOutput <(size_t qubit)>" << endl;
    cout << "               -ADDEdge <(size_t id_s), (size_t id_t), (EdgeType et)>" << endl;
}

void ZXGEditCmd::summary() const {
    cout << setw(15) << left << "ZXGEdit: "
         << "edit ZX-graph" << endl;
}

//----------------------------------------------------------------------
//    ZXGTRaverse
//----------------------------------------------------------------------
CmdExecStatus
ZXGTraverseCmd::exec(const string &option) {
    string token;
    if (!CmdExec::lexNoOption(option)) return CMD_EXEC_ERROR;
    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGTraverse");
    zxGraphMgr->getGraph()->updateTopoOrder();
    return CMD_EXEC_DONE;
}

void ZXGTraverseCmd::usage() const {
    cout << "Usage: ZXGTRaverse" << endl;
}

void ZXGTraverseCmd::summary() const {
    cout << setw(15) << left << "ZXGTRaverse: "
         << "traverse ZX-graph and update topological order of vertices" << endl;
}

//----------------------------------------------------------------------
//    ZXGDraw [-CLI]
//    ZXGDraw <string (path.pdf)>
//----------------------------------------------------------------------
CmdExecStatus
ZXGDrawCmd::exec(const string &option) {
    vector<string> options;
    if (!CmdExec::lexOptions(option, options)) return CMD_EXEC_ERROR;
    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGDraw");
    CMD_N_OPTS_AT_MOST_OR_RETURN(options, 1);
    if (options.size() == 0) {
        // zxGraphMgr->getGraph()->normalize();
        zxGraphMgr->getGraph()->draw();
        return CMD_EXEC_DONE;
    }
    if (myStrNCmp("-CLI", options[0], 4) == 0) {
        // zxGraphMgr->getGraph()->normalize();
        zxGraphMgr->getGraph()->draw();
        return CMD_EXEC_DONE;
    }
    if (!zxGraphMgr->getGraph()->writeTex(options[0])) {
        return CMD_EXEC_ERROR;
    }
    return CMD_EXEC_DONE;
}

void ZXGDrawCmd::usage() const {
    cout << "Usage: ZXGDraw [-CLI]" << endl;
    cout << "Usage: ZXGDraw <string (path.pdf)>" << endl;
}

void ZXGDrawCmd::summary() const {
    cout << setw(15) << left << "ZXGDraw: "
         << "draw ZX-graph" << endl;
}

//----------------------------------------------------------------------
//    ZX2TS
//----------------------------------------------------------------------
CmdExecStatus
ZX2TSCmd::exec(const string &option) {
    if (!CmdExec::lexNoOption(option)) return CMD_EXEC_ERROR;
    zxGraphMgr->getGraph()->toTensor();
    return CMD_EXEC_DONE;
}

void ZX2TSCmd::usage() const {
    cout << "Usage: ZX2TS" << endl;
}

void ZX2TSCmd::summary() const {
    cout << setw(15) << left << "ZX2TS: "
         << "convert ZX-graph to tensor" << endl;
}

//----------------------------------------------------------------------
//    ZXGRead <string Input.(b)zx> [-KEEPid] [-Replace]
//----------------------------------------------------------------------
CmdExecStatus
ZXGReadCmd::exec(const string &option) {  // check option
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

    ZXGraph *bufferGraph = new ZXGraph(0);
    if (!bufferGraph->readZX(fileName, doKeepID)) {
        // REVIEW - This error message is not always accurate
        // cerr << "Error: The format in \"" << fileName << "\" has something wrong!!" << endl;
        delete bufferGraph;
        return CMD_EXEC_ERROR;
    }

    if (doReplace) {
        if (zxGraphMgr->getgListItr() == zxGraphMgr->getGraphList().end()) {
            cout << "Note: ZX-graph list is empty now. Create a new one." << endl;
            zxGraphMgr->addZXGraph(zxGraphMgr->getNextID());
        } else {
            cout << "Note: original ZX-graph is replaced..." << endl;
        }
    } else {
        zxGraphMgr->addZXGraph(zxGraphMgr->getNextID());
    }
    zxGraphMgr->setGraph(bufferGraph);
    return CMD_EXEC_DONE;
}

void ZXGReadCmd::usage() const {
    cout << "Usage: ZXGRead <string Input.(b)zx> [-KEEPid] [-Replace]" << endl;
}

void ZXGReadCmd::summary() const {
    cout << setw(15) << left << "ZXGRead: "
         << "read a file and construct the corresponding ZX-graph" << endl;
}

//----------------------------------------------------------------------
//    ZXGWrite <string Output.<zx | tikz | tex>> [-Complete]
//----------------------------------------------------------------------

// unique_ptr<ArgParseCmdType> ZXGWriteCmd() {
//     auto cmd = make_unique<ArgParseCmdType>("ZXGWrite");

//     cmd->parserDefinition = [](ArgumentParser &parser) {
//         parser.help("write a ZX-graph to a file");
//         parser.addArgument<string>("output-path")
//             .help("the filepath of output file (.zx/.tikz/.tex)");
//         parser.addArgument<string>("-Complete")
//             .help("if specified, ")
//     };

//     cmd->onParseSuccess = [](ArgumentParser const &parser) {
//         ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGWrite");
//         if (!qcirMgr->getQCircuit()->writeQASM(parser["output-path.qasm"])) {
//             cerr << "Error: path " << parser["output-path.qasm"] << " not found!!" << endl;
//             return CMD_EXEC_ERROR;
//         }
//         return CMD_EXEC_DONE;
//     };

//     return cmd;
// }



CmdExecStatus
ZXGWriteCmd::exec(const string &option) {
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
            if (!zxGraphMgr->getGraph()->writeZX(fileName, doComplete)) {
                cerr << "Error: fail to write ZX-Graph to \"" << fileName << "\"!!" << endl;
                return CMD_EXEC_ERROR;
            }
        } else if (myStrNCmp(".tikz", extensionString, 5) == 0) {
            if (!zxGraphMgr->getGraph()->writeTikz(fileName)) {
                cerr << "Error: fail to write Tikz to \"" << fileName << "\"!!" << endl;
                return CMD_EXEC_ERROR;
            }
        } else if (myStrNCmp(".tex", extensionString, 4) == 0) {
            if (!zxGraphMgr->getGraph()->writeTex(fileName, false)) {
                cerr << "Error: fail to write tex to \"" << fileName << "\"!!" << endl;
                return CMD_EXEC_ERROR;
            }
        }
    } else {
        if (!zxGraphMgr->getGraph()->writeZX(fileName, doComplete)) {
            cerr << "Error: fail to write ZX-Graph to \"" << fileName << "\"!!" << endl;
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
         << "write a ZX-graph to a file\n";
}

//----------------------------------------------------------------------
//    ZXGASsign <size_t qubit> <I|O> <VertexType vt> <string Phase>
//----------------------------------------------------------------------
CmdExecStatus
ZXGAssignCmd::exec(const string &option) {
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

    if (!(isInput ? zxGraphMgr->getGraph()->isInputQubit(qid) : zxGraphMgr->getGraph()->isOutputQubit(qid))) {
        cerr << "Error: the specified boundary does not exist!!" << endl;
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
    }

    VertexType vt;
    ZX_CMD_VERTEX_TYPE_VALID_OR_RETURN(options[2], vt);

    Phase phase;
    ZX_CMD_PHASE_VALID_OR_RETURN(options[3], phase);

    zxGraphMgr->getGraph()->assignBoundary(qid, (options[1] == "I"), vt, phase);
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
CmdExecStatus
ZXGAdjointCmd::exec(const string &option) {
    if (!lexNoOption(option)) return CMD_EXEC_ERROR;
    ZX_CMD_GRAPHMGR_NOT_EMPTY_OR_RETURN("ZXGAdjoint");

    zxGraphMgr->getGraph()->adjoint();
    return CMD_EXEC_DONE;
}

void ZXGAdjointCmd::usage() const {
    cout << "Usage: ZXGADJoint" << endl;
}

void ZXGAdjointCmd::summary() const {
    cout << setw(15) << left << "ZXGADJoint: "
         << "adjoint ZX-graph\n";
}
