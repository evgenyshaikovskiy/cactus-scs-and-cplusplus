#include <iostream>
#include <vector>
#include <map>
#include "cpp/sc_addr.hpp"
#include "cpp/sc_memory.hpp"
#include "cpp/sc_iterator.hpp"

ScAddr graph, rrel_arcs, rrel_nodes;

std::vector<ScAddr> visited_once;
std::vector<ScAddr> visited_twice;

// keeps track of current path
std::vector<ScAddr> currentPath;

// saves all cycles that were found
std::vector<std::vector<ScAddr>> cycles;


bool is_in_list(std::vector<ScAddr> vector, ScAddr value)
{
    for (int i = 0; i < vector.size(); i++)
    {
        if (value == vector[i])
        {
            return true;
        }
    }

    return false;
}

ScAddr first_node(const std::unique_ptr<ScMemoryContext>& context)
{
    ScAddr nodes;
    ScAddr printed_vertex = context->CreateNode(ScType::Const);
    ScIterator5Ptr its = context->Iterator5(graph, ScType::EdgeAccessConstPosPerm, ScType(0), ScType::EdgeAccessConstPosPerm, rrel_arcs);

    its->Next();
    nodes = its->Get(2);
    return nodes;
}

void print_element(const std::unique_ptr<ScMemoryContext>& context, ScAddr element)
{
    ScType type;
    type = context->GetElementType(element);

    if (type.IsNode() == ScType::Node)
    {
        std::string data;
        data = context->HelperGetSystemIdtf(element);
        std::cout << data;
    }
    else
    {
        ScAddr elem1, elem2;
        elem1 = context->GetEdgeSource(element);
        elem2 = context->GetEdgeTarget(element);
        std::cout << "(";
        print_element(context, elem1);
        std::cout << " -> ";
        print_element(context, elem2);
        std::cout << ")";
    }
}

bool find_vertex_in_set (const std::unique_ptr<ScMemoryContext>& context,ScAddr element, ScAddr set)
{
    bool find = false;
    ScIterator3Ptr location = context->Iterator3(set,ScType::EdgeAccessConstPosPerm,ScType(0));

    while (location->Next())
    {
        ScAddr loc = location->Get(2);

        if (loc != element)
        {
            find = false;
            continue;
        }
        else
        {
            find = true;
            break;
        }
    }
    return find;
}

void get_edge_vertexes (const std::unique_ptr<ScMemoryContext>& context,ScAddr edge, ScAddr &v1, ScAddr &v2)
{
    v1 = context->GetEdgeSource(edge);
    v2 = context->GetEdgeTarget(edge);
}

int index_of(std::vector<ScAddr> vector, ScAddr vertex)
{
    for (int i = 0; i < vector.size(); i++)
    {
        if (vertex == vector[i])
        {
            return i;
        }
    }

    return -1;
}

void dfs(const std::unique_ptr<ScMemoryContext>& context, ScAddr vertex, ScAddr parent, ScAddr arcs)
{
    if (!parent.IsValid())
    {
        parent = vertex;
    }

    currentPath.push_back(vertex);
    if (is_in_list(visited_once, vertex))
    {
        int firstIndex = index_of(currentPath, vertex);

        std::vector<ScAddr> local_cycle = std::vector<ScAddr>();

        for (int i = firstIndex; i < currentPath.size() - 1; i++)
        {
            local_cycle.push_back(currentPath[i]);
        }

        cycles.push_back(local_cycle);
        currentPath.pop_back();
        return;
    }

    visited_once.push_back(vertex);

    ScIterator5Ptr next_node = context->Iterator5(vertex, ScType::Const, ScType(0), ScType::Const , arcs);
    ScAddr node;

    while (next_node->Next())
    {
        node = next_node->Get(2);

        if (!is_in_list(visited_twice, node) && parent != node)
        {
            dfs(context, node, vertex, arcs);
        }
    }

    next_node = context->Iterator5(ScType(0), ScType::Const, vertex, ScType::Const, arcs);
    while (next_node->Next())
    {
        node = next_node->Get(0);

        if (!is_in_list(visited_twice, node) && parent != node)
        {
            dfs(context, node, vertex, arcs);
        }
    }

    currentPath.pop_back();
    visited_twice.push_back(vertex);
}

void print_graph (const std::unique_ptr<ScMemoryContext>& context)
{
    ScAddr arcs, nodes, v1, v2, printed_vertex;
    bool find;
    printed_vertex = context->CreateNode(ScType::Const);

    print_element(context, graph);
    std::cout << std::endl << "----------------------" << std::endl;

    ScIterator5Ptr it = context->Iterator5(graph,ScType::EdgeAccessConstPosPerm,ScType(0),ScType::EdgeAccessConstPosPerm,rrel_arcs);

    if (it->Next())
    {
        arcs = it->Get(2);

        ScIterator3Ptr arcs_it = context->Iterator3(arcs,ScType::EdgeAccessConstPosPerm,ScType(0));

        while (arcs_it->Next())
        {
            ScAddr t_arc = arcs_it->Get(2);

            get_edge_vertexes (context,t_arc, v1, v2);

            print_element(context, v1);
            std::cout << " -- ";
            print_element(context, v2);
            std::cout << std::endl;

            if (!find_vertex_in_set(context,v1, printed_vertex))
            {
                context->CreateEdge(ScType::EdgeAccessConstPosPerm,printed_vertex, v1);
            }
            if (!find_vertex_in_set(context,v2, printed_vertex))
            {
                context->CreateEdge(ScType::EdgeAccessConstPosPerm,printed_vertex, v2);
            }
        }
    }

    it = context->Iterator5(graph,ScType::EdgeAccessConstPosPerm,ScType(0),ScType::EdgeAccessConstPosPerm,rrel_nodes);

    if (it->Next())
    {
        nodes = it->Get(2);

        ScIterator3Ptr nodes_it = context->Iterator3(nodes,ScType::EdgeAccessConstPosPerm,ScType(0));


        while (nodes_it->Next())
        {

            ScAddr t_node = nodes_it->Get(2);

            find = find_vertex_in_set(context,t_node, printed_vertex);

            if (!find)
            {
                print_element(context, t_node);
                std::cout << std::endl;
            }
        }
    }
}

void test_run(const std::unique_ptr<ScMemoryContext>& context, std::string number_test)
{
    std::string graphName = "graph";
    graphName.append(number_test);
    graph = context->HelperResolveSystemIdtf(graphName);
    rrel_arcs = context->HelperResolveSystemIdtf("rrel_arcs");
    rrel_nodes = context->HelperResolveSystemIdtf("rrel_nodes");
    print_graph(context);

    // is cactus check
    ScAddr arcs_for_graph;

    ScAddr vertex = context->HelperResolveSystemIdtf("A");
    ScAddr parent = context->HelperResolveSystemIdtf("A");
    arcs_for_graph = first_node(context);

    dfs(context, vertex, parent, arcs_for_graph);

    int number_of_cycles = cycles.size();
    std::cout << "RESULT OF DFS:" << std::endl;
    std::cout << "NUMBER OF CYCLES IS " << number_of_cycles << std::endl;

    for(auto & cycle : cycles)
    {
        std::cout << "Cycle: ";
        for (auto & j : cycle)
        {
            print_element(context, j);
            std::cout << " ";
        }

        std::cout << std::endl;
    }

    std::cout << "CHECK FOR CACTUS:" << std::endl;

    bool isCactus = true;
    if (cycles.size() < 2)
    {
        isCactus = false;
    }
    else
    {
        for (int i = 0; i < cycles.size(); i++)
        {
            if (!isCactus)
            {
                break;
            }
            for (int j = i + 1; j < cycles.size(); j++)
            {
                int countOfSameVertexes = 0;
                for (int k = 0; k < cycles[i].size(); k++)
                {
                    if (is_in_list(cycles[j], cycles[i][j]))
                    {
                        countOfSameVertexes++;
                    }
                }

                if (countOfSameVertexes >= 2)
                {
                    isCactus = false;
                    break;
                }
            }
        }
    }

    if (isCactus)
    {
        std::cout << "Answer: " << graphName << " is cactus.." << std::endl;
    }
    else
    {
        std::cout << "Answer: " << graphName << " is not cactus.." << std::endl;
    }

    visited_once.clear();
    visited_twice.clear();
    cycles.clear();
    currentPath.clear();
}

int main()
{
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/evgeny/ostis/ostis-example-app/ostis-web-platform/kb.bin";
    params.config_file = "/home/evgeny/ostis/ostis-example-app/ostis-web-platform/config/sc-web.ini";
    params.ext_path = "/home/evgeny/ostis/ostis-example-app/ostis-web-platform/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    ScMemory mem;
    mem.Initialize(params);

    const std::unique_ptr<ScMemoryContext> context(new ScMemoryContext(sc_access_lvl_make_max,"name"));

    test_run(context, "1");
    test_run(context, "2");
    test_run(context, "3");
    test_run(context, "4");
    test_run(context, "5");

    mem.Shutdown(true);
    return 0;
}