//
//  Global_Placer.hpp
//  MyCppProj
//
//  Created by Karo Manukyan on 10/3/21.
//

#ifndef Global_Placer_hpp
#define Global_Placer_hpp
#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <regex>
#include <string>
using namespace std;
class Global_Placer
{
private:
    fstream read_netlist_graph;
    fstream read_pin_location;
    regex common;
    map<string,vector<string>> inst_nets_pair;
    map<string,double> net_weight;
    map<string,vector<string>> pad_net_inst_pair;
    vector<vector<string>> net_inst_inst;
    map<string ,int> inst_to_number_code;
    vector<vector<double>> Cij_Matrix;
    vector<vector<double>> Aij_Matrix;
    vector<double> Bx;
    vector<double> By;
    map<string,vector<double>> pin_name_location;
    vector<vector<double>> common_matrix_X;
    vector<vector<double>> common_matrix_Y;
    vector<double> y_location;
    vector<double> x_location;
    ofstream write_output_file;
    bool opened  ;
    void create_net_inst_map (string net_name,string cellpin_name); // for read_graph_file_and_classify()
    void create_pad_to_inst_connection (string net_name,string cell_name,string pin_name); //for read_graph_file_and_classify   ()
    void create_net_inst_inst_vector(string net_name,string cell1,string cell2); // for read_graph_file_and_classify()
    void transform_net_inst_inst_to_inst_number(); //for read_graph_file_and_classify()
    void create_pin_name_location(string pin_line); // for read_pin_name_location
    void x_y_vector_calculate (vector<vector<double>> & matrix ,int n_col_count_long ,vector<double> &answer); // for x_location ,y_location
public:
    Global_Placer();
    void read_graph_file_and_classify(string file_name);
    void create_Cij_Matrix();
    void create_Aij_Matrix();
    void read_pin_name_location(string pin_name);
    void create_By_vector();
    void create_Bx_vector();
    void create_common_matrix_Y();
    void create_common_matrix_X();
    void create_x_location();
    void create_y_location();
    void create_output_global_location(string outfile);
};
#endif /* Global_Placer_hpp */
