/****************************************************************************
  PackageName  [ zx ]
  Synopsis     [ Define zx package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "./zx_cmd.hpp"

#include <string>

#include "./gflow/gflow_cmd.hpp"
#include "./zxgraph_mgr.hpp"
#include "argparse/arg_parser.hpp"
#include "cli/cli.hpp"
#include "qsyn/qsyn_type.hpp"
#include "util/data_structure_manager.hpp"
#include "util/data_structure_manager_common_cmd.hpp"
#include "zx/simplifier/simp_cmd.hpp"

using namespace dvlab::argparse;
using dvlab::CmdExecResult;
using dvlab::Command;

namespace qsyn::zx {

std::function<bool(size_t const&)> valid_zxvertex_id(ZXGraphMgr const& zxgraph_mgr) {
    return [&](size_t const& id) {
        if (zxgraph_mgr.get()->is_v_id(id)) return true;
        spdlog::error("Cannot find vertex with ID {} in the ZXGraph!!", id);
        return false;
    };
}

std::function<bool(size_t const&)> zxgraph_id_not_exist(ZXGraphMgr const& zxgraph_mgr) {
    return [&](size_t const& id) {
        if (!zxgraph_mgr.is_id(id)) return true;
        spdlog::error("ZXGraph {} already exists!!", id);
        spdlog::info("Use `-Replace` if you want to overwrite it.");
        return false;
    };
}

std::function<bool(int const&)> zxgraph_input_qubit_not_exist(ZXGraphMgr const& zxgraph_mgr) {
    return [&](int const& qid) {
        if (!zxgraph_mgr.get()->is_input_qubit(qid)) return true;
        spdlog::error("This qubit's input already exists!!");
        return false;
    };
}

std::function<bool(int const&)> zxgraph_output_qubit_not_exist(ZXGraphMgr const& zxgraph_mgr) {
    return [&](int const& qid) {
        if (!zxgraph_mgr.get()->is_output_qubit(qid)) return true;
        spdlog::error("This qubit's output already exists!!");
        return false;
    };
}

Command zxgraph_compose_cmd(ZXGraphMgr& zxgraph_mgr) {
    return {"compose",
            [&](ArgumentParser& parser) {
                parser.description("compose with another ZXGraph");

                parser.add_argument<size_t>("id")
                    .constraint(dvlab::utils::valid_mgr_id(zxgraph_mgr))
                    .help("the ID of the ZXGraph to compose with");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                zxgraph_mgr.get()->compose(*zxgraph_mgr.find_by_id(parser.get<size_t>("id")));
                return CmdExecResult::done;
            }};
}

//----------------------------------------------------------------------
//    ZXTensor <size_t id>
//----------------------------------------------------------------------
Command zxgraph_tensor_product_cmd(ZXGraphMgr& zxgraph_mgr) {
    return {"tensor-product",
            [&](ArgumentParser& parser) {
                parser.description("perform tensor-product with another ZXGraph");

                parser.add_argument<size_t>("id")
                    .constraint(dvlab::utils::valid_mgr_id(zxgraph_mgr))
                    .help("the ID of the ZXGraph");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                zxgraph_mgr.get()->tensor_product(*zxgraph_mgr.find_by_id(parser.get<size_t>("id")));
                return CmdExecResult::done;
            }};
}

Command zxgraph_test_cmd(ZXGraphMgr const& zxgraph_mgr) {
    return {"test",
            [](ArgumentParser& parser) {
                parser.description("test ZXGraph structures and functions");

                auto mutex = parser.add_mutually_exclusive_group().required(true);

                mutex.add_argument<bool>("-v", "--valid")
                    .action(store_true)
                    .help("check if the ZXGraph is valid");
                mutex.add_argument<bool>("-e", "--empty")
                    .action(store_true)
                    .help("check if the ZXGraph is empty");
                mutex.add_argument<bool>("-g", "--graph-like")
                    .action(store_true)
                    .help("check if the ZXGraph is graph-like");
                mutex.add_argument<bool>("-i", "--identity")
                    .action(store_true)
                    .help("check if the ZXGraph is equivalent to identity");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                if (parser.parsed("--empty")) {
                    if (zxgraph_mgr.get()->is_empty()) {
                        fmt::println("The graph is empty!");
                    } else {
                        fmt::println("The graph is not empty!");
                    }
                } else if (parser.parsed("--valid")) {
                    if (zxgraph_mgr.get()->is_valid()) {
                        fmt::println("The graph is valid!");
                    } else {
                        fmt::println("The graph is invalid!");
                    }
                } else if (parser.parsed("--graph-like")) {
                    if (zxgraph_mgr.get()->is_graph_like()) {
                        fmt::println("The graph is graph-like!");
                    } else {
                        fmt::println("The graph is not graph-like!");
                    }
                } else if (parser.parsed("--identity")) {
                    if (zxgraph_mgr.get()->is_identity()) {
                        fmt::println("The graph is an identity!");
                    } else {
                        fmt::println("The graph is not an identity!");
                    }
                }
                return CmdExecResult::done;
            }};
}

Command zxgraph_print_cmd(ZXGraphMgr const& zxgraph_mgr) {
    return {"print",
            [&](ArgumentParser& parser) {
                parser.description("print info of ZXGraph");

                auto mutex = parser.add_mutually_exclusive_group();
                mutex.add_argument<bool>("-l", "--list")
                    .action(store_true)
                    .help("print a list of ZXGraphs");
                mutex.add_argument<bool>("-s", "--summary")
                    .action(store_true)
                    .help("print the summary info of ZXGraph");
                mutex.add_argument<bool>("--io")
                    .action(store_true)
                    .help("print the I/O info of ZXGraph");
                mutex.add_argument<bool>("-i", "--inputs")
                    .action(store_true)
                    .help("print the input info of ZXGraph");
                mutex.add_argument<bool>("-o", "--outputs")
                    .action(store_true)
                    .help("print the output info of ZXGraph");
                mutex.add_argument<size_t>("-v", "--vertices")
                    .nargs(NArgsOption::zero_or_more)
                    .constraint(valid_zxvertex_id(zxgraph_mgr))
                    .help("print the vertex info of ZXGraph");
                mutex.add_argument<bool>("-e", "--edges")
                    .action(store_true)
                    .help("print the edges info of ZXGraph");
                mutex.add_argument<int>("-q", "--qubits")
                    .nargs(NArgsOption::zero_or_more)
                    .help("print the vertices of ZXGraph by their qubits");
                mutex.add_argument<size_t>("-n", "--neighbors")
                    .constraint(valid_zxvertex_id(zxgraph_mgr))
                    .help("print the neighbor info of ZXGraph");
                mutex.add_argument<bool>("-d", "--density")
                    .action(store_true)
                    .help("calculate the density of ZXGraph");
            },

            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                if (parser.parsed("--summary")) {
                    zxgraph_mgr.get()->print_graph();
                    fmt::println("{:<29} {}", "#T-gate:", zxgraph_mgr.get()->t_count());
                    fmt::println("{:<29} {}", "#Non-(Clifford+T)-gate: ", zxgraph_mgr.get()->non_clifford_t_count());
                    fmt::println("{:<29} {}", "#Non-Clifford-gate: ", zxgraph_mgr.get()->non_clifford_count());
                } else if (parser.parsed("--io"))
                    zxgraph_mgr.get()->print_io();
                else if (parser.parsed("--list"))
                    zxgraph_mgr.print_list();
                else if (parser.parsed("--inputs"))
                    zxgraph_mgr.get()->print_inputs();
                else if (parser.parsed("--outputs"))
                    zxgraph_mgr.get()->print_outputs();
                else if (parser.parsed("--vertices")) {
                    auto vids = parser.get<std::vector<size_t>>("--vertices");
                    if (vids.empty())
                        zxgraph_mgr.get()->print_vertices();
                    else
                        zxgraph_mgr.get()->print_vertices(vids);
                } else if (parser.parsed("--edges")) {
                    zxgraph_mgr.get()->print_edges();
                } else if (parser.parsed("--qubits")) {
                    auto qids = parser.get<std::vector<int>>("--qubits");
                    zxgraph_mgr.get()->print_vertices_by_qubits(spdlog::level::level_enum::off, qids);
                } else if (parser.parsed("--neighbors")) {
                    auto v = zxgraph_mgr.get()->find_vertex_by_id(parser.get<size_t>("--neighbors"));
                    v->print_vertex();
                    fmt::println("----- Neighbors -----");
                    for (auto [nb, _] : zxgraph_mgr.get()->get_neighbors(v)) {
                        nb->print_vertex();
                    }
                } else if (parser.parsed("--density")) {
                    fmt::println("Density: {}", zxgraph_mgr.get()->density());
                } else {
                    zxgraph_mgr.get()->print_graph();
                }
                return CmdExecResult::done;
            }};
}

Command zxgraph_draw_cmd(ZXGraphMgr const& zxgraph_mgr) {
    return {"draw",
            [](ArgumentParser& parser) {
                parser.description("draw ZXGraph");

                parser.add_argument<std::string>("filepath")
                    .nargs(NArgsOption::optional)
                    .constraint(path_writable)
                    .constraint(allowed_extension({".pdf"}))
                    .help("the output path. Supported extension: .pdf");

                parser.add_argument<bool>("-cli")
                    .action(store_true)
                    .help("print to the terminal. Note that only horizontal wires will be printed");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                if (parser.parsed("filepath")) {
                    if (!zxgraph_mgr.get()->write_pdf(parser.get<std::string>("filepath"))) return CmdExecResult::error;
                }
                if (parser.parsed("-cli")) {
                    spdlog::warn("This feature is deprecated and will be removed in the future.");
                    zxgraph_mgr.get()->draw();
                }

                return CmdExecResult::done;
            }};
}

Command zxgraph_read_cmd(ZXGraphMgr& zxgraph_mgr) {
    return {"read",
            [&](ArgumentParser& parser) {
                parser.description("read a file and construct the corresponding ZXGraph");

                parser.add_argument<std::string>("filepath")
                    .constraint(path_readable)
                    .constraint(allowed_extension({".zx"}))
                    .help("path to the ZX file. Supported extensions: .zx");

                parser.add_argument<bool>("--keep-id")
                    .action(store_true)
                    .help("if set, retain the IDs in the ZX file; otherwise the ID is rearranged to be consecutive");

                parser.add_argument<bool>("-r", "--replace")
                    .action(store_true)
                    .constraint(zxgraph_id_not_exist(zxgraph_mgr))
                    .help("replace the current ZXGraph");
            },
            [&](ArgumentParser const& parser) {
                auto filepath   = parser.get<std::string>("filepath");
                auto do_keep_id = parser.get<bool>("--keep-id");
                auto do_replace = parser.get<bool>("--replace");

                auto buffer_graph = std::make_unique<ZXGraph>();
                if (!buffer_graph->read_zx(filepath, do_keep_id)) {
                    return CmdExecResult::error;
                }

                if (do_replace) {
                    if (zxgraph_mgr.empty()) {
                        spdlog::info("ZXGraph list is empty now. Creating a new ZXGraph...");
                        zxgraph_mgr.add(zxgraph_mgr.get_next_id());
                    } else {
                        spdlog::info("Original ZXGraph is replaced...");
                    }
                    zxgraph_mgr.set(std::move(buffer_graph));
                } else {
                    zxgraph_mgr.add(zxgraph_mgr.get_next_id(), std::move(buffer_graph));
                }
                zxgraph_mgr.get()->set_filename(std::filesystem::path{filepath}.stem());
                return CmdExecResult::done;
            }};
}

Command zxgraph_write_cmd(ZXGraphMgr const& zxgraph_mgr) {
    return {"write",
            [](ArgumentParser& parser) {
                parser.description("write the ZXGraph to a file");

                parser.add_argument<std::string>("filepath")
                    .constraint(path_writable)
                    .constraint(allowed_extension({".zx", ".tikz", ".tex"}))
                    .help("the path to the output ZX file");

                parser.add_argument<bool>("--complete")
                    .action(store_true)
                    .help("if specified, output neighbor information on both vertices of each edge");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                auto const filepath      = parser.get<std::string>("filepath");
                auto const do_complete   = parser.get<bool>("--complete");
                auto const extension_pos = filepath.find_last_of('.');

                auto const extension = (extension_pos == std::string::npos) ? "" : filepath.substr(extension_pos);
                if (extension == ".zx") {
                    if (!zxgraph_mgr.get()->write_zx(filepath, do_complete)) {
                        spdlog::error("Failed to write ZXGraph to \"{}\"!!", filepath);
                        return CmdExecResult::error;
                    }
                } else if (extension == ".tikz") {
                    if (!zxgraph_mgr.get()->write_tikz(filepath)) {
                        spdlog::error("Failed to write Tikz to \"{}\"!!", filepath);
                        return CmdExecResult::error;
                    }
                } else if (extension == ".tex") {
                    if (!zxgraph_mgr.get()->write_tex(filepath)) {
                        spdlog::error("Failed to write Tex to \"{}\"!!", filepath);
                        return CmdExecResult::error;
                    }
                }
                return CmdExecResult::done;
            }};
}

Command zxgraph_assign_boundary_cmd(ZXGraphMgr& zxgraph_mgr) {
    return {"assign",
            [](ArgumentParser& parser) {
                parser.description("assign quantum states to input/output vertex");

                parser.add_argument<int>("qubit")
                    .help("the qubit to assign state to");

                parser.add_argument<std::string>("io")
                    .constraint(choices_allow_prefix({"input", "output"}))
                    .metavar("input/output")
                    .help("add at input or output");

                parser.add_argument<std::string>("vtype")
                    .constraint(choices_allow_prefix({"zspider", "xspider", "hbox"}))
                    .help("the type of ZXVertex");

                parser.add_argument<Phase>("phase")
                    .help("the phase of the vertex");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                auto const qid      = parser.get<int>("qubit");
                auto const is_input = dvlab::str::tolower_string(parser.get<std::string>("io")).starts_with('i');

                if (!(is_input ? zxgraph_mgr.get()->is_input_qubit(qid) : zxgraph_mgr.get()->is_output_qubit(qid))) {
                    spdlog::error("Error: the specified {} vertex does not exist!!", is_input ? "input" : "output");
                    return CmdExecResult::error;
                }

                auto const vtype = str_to_vertex_type(parser.get<std::string>("vtype"));
                assert(vtype.has_value());

                auto const phase = parser.get<Phase>("phase");
                zxgraph_mgr.get()->assign_vertex_to_boundary(qid, is_input, vtype.value(), phase);

                return CmdExecResult::done;
            }};
}

Command zxgraph_adjoint_cmd(ZXGraphMgr& zxgraph_mgr) {
    return {"adjoint",
            [](ArgumentParser& parser) {
                parser.description("adjoint ZXGraph");
            },
            [&](ArgumentParser const& /*parser*/) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                zxgraph_mgr.get()->adjoint();
                return CmdExecResult::done;
            }};
}

Command zxgraph_vertex_add_cmd(ZXGraphMgr& zxgraph_mgr) {
    using namespace std::string_view_literals;
    return {"add",
            [](ArgumentParser& parser) {
                parser.description("add vertices to ZXGraph");

                parser.add_argument<std::string>("vtype")
                    .constraint(choices_allow_prefix({"input", "output", "zspider", "xspider", "hbox"}))
                    .help("the type of the vertex. Choices: `input`, `output`, `zspider`, `xspider`, `hbox`");

                parser.add_argument<QubitIdType>("qubit")
                    .nargs(NArgsOption::optional)
                    .help("the qubit ID of the vertex. If adding input or output vertices, this argument must be unique");

                parser.add_argument<Phase>("phase")
                    .nargs(NArgsOption::optional)
                    .help("the phase of the vertex. Default: 0");
            },
            [&](ArgumentParser const& parser) {
                if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
                auto const vtype_str = parser.get<std::string>("vtype");
                if (dvlab::str::is_prefix_of(vtype_str, "input")) {
                    if (!parser.parsed("qubit")) {
                        spdlog::error("The qubit ID of the input vertex must be specified!!");
                        return CmdExecResult::error;
                    }
                    auto const qid = parser.get<QubitIdType>("qubit");
                    if (zxgraph_mgr.get()->is_input_qubit(qid)) {
                        spdlog::error("Input vertex for qubit {} already exists!!", qid);
                        return CmdExecResult::error;
                    }
                    if (parser.parsed("phase")) {
                        spdlog::error("Input vertex cannot have phase!!");
                        return CmdExecResult::error;
                    }

                    zxgraph_mgr.get()->add_input(qid);

                    spdlog::info("Adding input vertex for qubit {}...", qid);

                    return CmdExecResult::done;
                }

                if (dvlab::str::is_prefix_of(vtype_str, "output")) {
                    if (!parser.parsed("qubit")) {
                        spdlog::error("The qubit ID of the output vertex must be specified!!");
                        return CmdExecResult::error;
                    }
                    auto const qid = parser.get<QubitIdType>("qubit");
                    if (zxgraph_mgr.get()->is_output_qubit(qid)) {
                        spdlog::error("Error: output vertex for qubit {} already exists!!", qid);
                        return CmdExecResult::error;
                    }
                    if (parser.parsed("phase")) {
                        spdlog::error("Error: output vertex cannot have phase!!");
                        return CmdExecResult::error;
                    }

                    zxgraph_mgr.get()->add_output(qid);

                    spdlog::info("Adding output vertex for qubit {}...", qid);

                    return CmdExecResult::done;
                }

                auto const vtype = str_to_vertex_type(vtype_str);
                assert(vtype.has_value());

                auto qid   = parser.parsed("qubit")
                                 ? parser.get<int>("qubit")
                                 : 0;
                auto phase = parser.parsed("phase")
                                 ? parser.get<Phase>("phase")
                                 : ((vtype == VertexType::h_box)
                                        ? Phase(1)
                                        : Phase(0));

                auto v = zxgraph_mgr.get()->add_vertex(qid, vtype.value(), phase);
                spdlog::info("Adding vertex {}...", v->get_id());

                return CmdExecResult::done;
            }};
}

Command zxgraph_vertex_remove_cmd(ZXGraphMgr& zxgraph_mgr) {
    return Command{
        "remove",
        [&](ArgumentParser& parser) {
            auto mutex = parser.add_mutually_exclusive_group().required(true);

            mutex.add_argument<size_t>("ids")
                .constraint(valid_zxvertex_id(zxgraph_mgr))
                .nargs(NArgsOption::zero_or_more)
                .help("the IDs of vertices to remove");

            mutex.add_argument<bool>("-i", "--isolated")
                .action(store_true)
                .help("if set, remove all isolated vertices");
        },
        [&](ArgumentParser const& parser) {
            if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
            if (parser.parsed("ids")) {
                auto ids            = parser.get<std::vector<size_t>>("ids");
                auto vertices_range = ids |
                                      std::views::transform([&](size_t id) { return zxgraph_mgr.get()->find_vertex_by_id(id); }) |
                                      std::views::filter([](ZXVertex* v) { return v != nullptr; });
                for (auto&& v : vertices_range) {
                    spdlog::info("Removing vertex {}...", v->get_id());
                }

                zxgraph_mgr.get()->remove_vertices({vertices_range.begin(), vertices_range.end()});
            } else if (parser.parsed("--isolated")) {
                spdlog::info("Removing isolated vertices...");
                zxgraph_mgr.get()->remove_isolated_vertices();
            }
            return CmdExecResult::done;
        }};
}

Command zxgraph_vertex_cmd(ZXGraphMgr& zxgraph_mgr) {
    auto cmd = Command{
        "vertex",
        [](ArgumentParser& parser) {
            parser.description("add, remove, or edit vertices of ZXGraph");

            parser.add_subparsers().required(true);
        },
        [](ArgumentParser const& /*parser*/) {
            return CmdExecResult::done;
        }};
    cmd.add_subcommand(zxgraph_vertex_add_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_vertex_remove_cmd(zxgraph_mgr));
    return cmd;
}

Command zxgraph_edge_add_cmd(ZXGraphMgr& zxgraph_mgr) {
    return Command{
        "add",
        [&](ArgumentParser& parser) {
            parser.description("add edges to ZXGraph");

            parser.add_argument<size_t>("ids")
                .nargs(2)
                .constraint(valid_zxvertex_id(zxgraph_mgr))
                .metavar("(vs, vt)")
                .help("the IDs to the two vertices to add edges in between");

            parser.add_argument<std::string>("etype")
                .constraint(choices_allow_prefix({"simple", "hadamard"}))
                .help("the edge type to add. Options: simple, hadamard");
        },
        [&](ArgumentParser const& parser) {
            if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
            auto ids = parser.get<std::vector<size_t>>("ids");
            auto vs  = zxgraph_mgr.get()->find_vertex_by_id(ids[0]);
            auto vt  = zxgraph_mgr.get()->find_vertex_by_id(ids[1]);
            assert(vs != nullptr && vt != nullptr);

            auto etype = str_to_edge_type(parser.get<std::string>("etype"));
            assert(etype.has_value());

            if (zxgraph_mgr.get()->is_neighbor(vs, vt, etype.value()) && (vs->is_boundary() || vt->is_boundary())) {
                spdlog::critical("Cannot add edge between boundary vertices {} and {}", vs->get_id(), vt->get_id());
                return CmdExecResult::error;
            }

            auto const had_edge = zxgraph_mgr.get()->is_neighbor(vs, vt, etype.value());

            zxgraph_mgr.get()->add_edge(vs, vt, etype.value());

            if (vs == vt) {
                spdlog::info("Note: converting this self-loop to phase {} on vertex {}...", etype.value() == EdgeType::hadamard ? Phase(1) : Phase(0), vs->get_id());
            } else if (had_edge) {
                auto const has_edge = zxgraph_mgr.get()->is_neighbor(vs, vt, etype.value());
                if (has_edge) {
                    spdlog::info("Note: redundant edge; merging into existing edge ({}, {})...", vs->get_id(), vt->get_id());
                } else {
                    spdlog::info("Note: Hopf edge; cancelling out with existing edge ({}, {})...", vs->get_id(), vt->get_id());
                }
            } else {
                spdlog::info("Adding edge ({}, {}), edge type: {}...", vs->get_id(), vt->get_id(), etype.value());
            }

            return CmdExecResult::done;
        }};
}

Command zxgraph_edge_remove_cmd(ZXGraphMgr& zxgraph_mgr) {
    return Command{
        "remove",
        [&](ArgumentParser& parser) {
            parser.description("remove edges from ZXGraph");

            parser.add_argument<size_t>("ids")
                .nargs(2)
                .constraint(valid_zxvertex_id(zxgraph_mgr))
                .metavar("(vs, vt)")
                .help("the IDs to the two vertices to remove edges in between");

            parser.add_argument<std::string>("etype")
                .constraint(choices_allow_prefix({"simple", "hadamard", "all"}))
                .help("the edge type to remove. Options: simple, hadamard, all (i.e., remove both)");
        },
        [&](ArgumentParser const& parser) {
            if (!dvlab::utils::mgr_has_data(zxgraph_mgr)) return CmdExecResult::error;
            auto ids = parser.get<std::vector<size_t>>("ids");
            auto vs  = zxgraph_mgr.get()->find_vertex_by_id(ids[0]);
            auto vt  = zxgraph_mgr.get()->find_vertex_by_id(ids[1]);
            assert(vs != nullptr && vt != nullptr);

            auto etype = str_to_edge_type(parser.get<std::string>("etype"));

            if (etype.has_value()) {
                spdlog::info("Removing edge ({}, {}), edge type: {}...", vs->get_id(), vt->get_id(), etype.value());
                zxgraph_mgr.get()->remove_edge(vs, vt, etype.value());
            } else {
                spdlog::info("Removing all edges between ({}, {})...", vs->get_id(), vt->get_id());
                zxgraph_mgr.get()->remove_all_edges_between(vs, vt);
            }

            return CmdExecResult::done;
        }};
}

Command zxgraph_edge_cmd(ZXGraphMgr& zxgraph_mgr) {
    auto cmd = Command{
        "edge",
        [](ArgumentParser& parser) {
            parser.description("add, remove, or edit edges of ZXGraph");

            parser.add_subparsers().required(true);
        },
        [](ArgumentParser const& /*parser*/) {
            return CmdExecResult::done;
        }};
    cmd.add_subcommand(zxgraph_edge_add_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_edge_remove_cmd(zxgraph_mgr));
    return cmd;
}

Command zxgraph_cmd(ZXGraphMgr& zxgraph_mgr) {
    using namespace dvlab::utils;

    auto cmd = Command{
        "zx",
        [&](ArgumentParser& parser) {
            parser.description(fmt::format("ZXGraph commands"));
        },
        [&](ArgumentParser const& /* parser */) {
            zxgraph_mgr.print_manager();
            return CmdExecResult::done;
        }};
    cmd.add_subcommand(mgr_list_cmd(zxgraph_mgr));
    cmd.add_subcommand(mgr_checkout_cmd(zxgraph_mgr));
    cmd.add_subcommand(mgr_new_cmd(zxgraph_mgr));
    cmd.add_subcommand(mgr_delete_cmd(zxgraph_mgr));
    cmd.add_subcommand(mgr_copy_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_compose_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_tensor_product_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_print_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_read_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_write_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_draw_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_assign_boundary_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_adjoint_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_test_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_gflow_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_optimize_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_rule_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_vertex_cmd(zxgraph_mgr));
    cmd.add_subcommand(zxgraph_edge_cmd(zxgraph_mgr));
    return cmd;
}

bool add_zx_cmds(dvlab::CommandLineInterface& cli, ZXGraphMgr& zxgraph_mgr) {
    if (!cli.add_command(zxgraph_cmd(zxgraph_mgr))) {
        spdlog::error("Registering \"zx\" commands fails... exiting");
        return false;
    }
    return true;
}

}  // namespace qsyn::zx
