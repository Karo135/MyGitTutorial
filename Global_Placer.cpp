//
//  Global_Placer.cpp
//  MyCppProj
//
//  Created by Karo Manukyan on 10/3/21.
//

#include "Global_Placer.hpp"

void func() {
    cout<<"other file function"<<endl;
}

Global_Placer::Global_Placer() {
    string str_reg=R"((n[0-9]*)(.)([A-Za-z/0-9]*)(->)([A-Za-z/0-9]*)(.)\W+([0-9]+))";
    common=str_reg;
    opened=false;
    
}

void Global_Placer::read_graph_file_and_classify(string file_name) {
    
    read_netlist_graph.open(file_name);
    if(read_netlist_graph.is_open())
        opened=true;
    if(opened)
    {
        string data;
        smatch match;
        while(read_netlist_graph.good())
        {
            getline(read_netlist_graph,data);
        
            regex_match(data,match,common);
            net_weight[match[1].str()]=stoi(match[7].str());
            if(match[3].str().find("pin")==string::npos)
            {
                
              create_net_inst_map( match[1].str(),match[3].str());
               if(match[5].str().find("pin")!=string::npos)                create_pad_to_inst_connection(match[1].str(), match[3].str(),match[5].str());
            }
            if(match[5].str().find("pin")==string::npos)
            {
               
                 create_net_inst_map( match[1].str(),match[5].str());
               if(match[3].str().find("pin")!=string::npos)
                   create_pad_to_inst_connection( match[1].str(), match[5].str(),match[3].str());
            }
            if(match[3].str().find("pin")==string::npos && match[5].str().find("pin")==string::npos)
            {
                create_net_inst_inst_vector(match[1].str(), match[3].str(), match[5].str());
            }
        }
        transform_net_inst_inst_to_inst_number();
        read_netlist_graph.close();
    }
    else if (opened==false) {
        cout<<"Couldn't open a netlist file , maybe wrong address"<<endl;
    }
}

void Global_Placer::create_net_inst_map(string net_name,string cellpin_name) {
    regex cell_reg("[A-Za-z0-9]*");
    smatch cell_name_match;
    regex_search(cellpin_name,cell_name_match,cell_reg);
    inst_nets_pair[cell_name_match[0].str()].push_back(net_name);
}

void Global_Placer::create_pad_to_inst_connection(std::string net_name, std::string cell_name, std::string pin_name) {
    regex cell_reg("[A-Za-z0-9]*");
   smatch cell_name_match;
   // cout<<cell1<<endl;
   regex_search(cell_name,cell_name_match,cell_reg);
    pad_net_inst_pair[cell_name_match[0].str()].push_back(pin_name);
    pad_net_inst_pair[cell_name_match[0].str()].push_back(net_name);
}

void Global_Placer::create_net_inst_inst_vector(std::string net_name, std::string cell1, std::string cell2) { 
    vector<string> temp;
    temp.push_back(net_name);
    regex cell_reg("[A-Za-z0-9]*");
   smatch cell_name_match;
   // cout<<cell1<<endl;
   regex_search(cell1,cell_name_match,cell_reg);
    //cout<<cell_name_match[0]<<endl;
    temp.push_back(cell_name_match[0].str());
    regex_search(cell2,cell_name_match,cell_reg);
    temp.push_back(cell_name_match[0].str());
    net_inst_inst.push_back(temp);
}

void Global_Placer::transform_net_inst_inst_to_inst_number() {
    int number=0;
    int count=1;
    while(count<3)
    {
        for (int i=0; i<net_inst_inst.size(); i++) {
            if(inst_to_number_code.find(net_inst_inst[i][count])==inst_to_number_code.end())
               {
                inst_to_number_code[net_inst_inst[i][count]]=number;
                number++;
            }
        }
        count++;
    }
}

void Global_Placer::create_Cij_Matrix() {
    long size=inst_to_number_code.size();
    map<string,string>  instplusinst_net;
    Cij_Matrix=vector<vector<double>> (size,vector<double>(size,0));
    for (long i=0; i<net_inst_inst.size(); i++){
        cout<<net_inst_inst[i][1]+"+"+net_inst_inst[i][2]<<endl;
        instplusinst_net[net_inst_inst[i][1]+"+"+net_inst_inst[i][2]]=net_inst_inst[i][0];
    }
    for (long i=0; i<net_inst_inst.size(); i++) {
        Cij_Matrix[inst_to_number_code[net_inst_inst[i][1]]][inst_to_number_code[net_inst_inst[i][2]]]=net_weight[instplusinst_net[net_inst_inst[i][1]+"+"+net_inst_inst[i][2]]];
        Cij_Matrix[inst_to_number_code[net_inst_inst[i][2]]][inst_to_number_code[net_inst_inst[i][1]]]=net_weight[instplusinst_net[net_inst_inst[i][1]+"+"+net_inst_inst[i][2]]];
    }
}

void Global_Placer::create_Aij_Matrix() {
    int index=0;
    Aij_Matrix=Cij_Matrix;
   for (const auto &item:  inst_nets_pair )
   {
       for (const auto &vector: item.second)
       {
           Aij_Matrix[index][index]=Aij_Matrix[index][index]+ net_weight[vector];
       }
       index++;
   }
    for (int i=0; i<Cij_Matrix.size(); i++) {
        for (int j=0; j<Cij_Matrix.size(); j++) {
            if(i!=j&&Aij_Matrix[i][j]!=0)
                Aij_Matrix[i][j]=-1*Aij_Matrix[i][j];
        }
    }
}

void Global_Placer::read_pin_name_location(string pinfile_name) {
    read_pin_location.open(pinfile_name);
    string pin_line;
    
    while(read_pin_location.good())
    {
        getline(read_pin_location,pin_line);
        if(pin_line=="")
            break;
        create_pin_name_location( pin_line);
    }
    read_pin_location.close();
    
}

void Global_Placer::create_pin_name_location(std::string pin_line) {
    regex reg_pin("([a-zA-Z0-9]*) ([0-9]+) ([0-9]+)");
    smatch  pin_match;
    regex_search(pin_line,pin_match,reg_pin);
  
    pin_name_location[pin_match[1].str()].push_back(stod(pin_match[2].str()));

    pin_name_location[pin_match[1].str()].push_back(stod(pin_match[3].str()));
}

void Global_Placer::create_By_vector() {
    By=vector<double> (inst_to_number_code.size(),0);
     int index=0;
       for (auto item: inst_to_number_code)
       {
           if(pad_net_inst_pair.find(item.first)==pad_net_inst_pair.end())
           {
               index++;
               continue;
           }
           for (int i=0;i<pad_net_inst_pair[item.first].size();i++) {
 By[index]=By[index]+pin_name_location[pad_net_inst_pair[item.first][i]][1]*net_weight[pad_net_inst_pair[item.first][i+1]];
               
               i++;
               //index++;
           }
           index++;
       }
}

void Global_Placer::create_Bx_vector() {
    Bx=vector<double>(inst_to_number_code.size(),0);
    int index=0;
      for (auto item: inst_to_number_code)
      {
          if(pad_net_inst_pair.find(item.first)==pad_net_inst_pair.end())
          {
              index++;
              continue;
          }
          for (int i=0;i<pad_net_inst_pair[item.first].size();i++) {
          Bx[index]=Bx[index]+pin_name_location[pad_net_inst_pair[item.first][i]][0]*net_weight[pad_net_inst_pair[item.first][i+1]];
              i++;
          }
          index++;
      }
}

void Global_Placer::create_common_matrix_Y() {
    common_matrix_Y=vector<vector<double>>(inst_to_number_code.size(),vector<double>(inst_to_number_code.size(),0));
    common_matrix_Y=Aij_Matrix;
    int index=0;
    for (auto &item:common_matrix_Y)
    {

        item.push_back(By[index]);

        index++;
    }
    
    
}

void Global_Placer::create_common_matrix_X() {
    common_matrix_X=vector<vector<double>>(inst_to_number_code.size(),vector<double>(inst_to_number_code.size(),0));
    common_matrix_X=Aij_Matrix;
    
    int index=0;
    for (auto &item:common_matrix_X)
    {

        item.push_back((double)Bx[index]);

        index++;
    }
}

void Global_Placer::create_x_location() {
    x_y_vector_calculate(common_matrix_X, common_matrix_X[0].size(),x_location);
    for (auto item : x_location) {
        cout<<item<<" ";
    }
}

void Global_Placer::create_y_location() {
    x_y_vector_calculate(common_matrix_Y, common_matrix_Y[0].size(),y_location);
}

void Global_Placer::x_y_vector_calculate(vector<vector<double>> &matrix, int n_col_count_long, vector<double> &answer) {
    int n=n_col_count_long-1;
    
    vector<vector<double>> temp_matrix=
    
    matrix;
    double temp;
    for (int i=0; i<n; i++) {
        for (int k=i+1; k<n; k++) {
            if (temp_matrix[k][i]>temp_matrix[i][i])
                for (int j=0; j<=n; j++) {
                    temp=temp_matrix[i][j];
                    temp_matrix[i][j]=temp_matrix[k][j];
                    temp_matrix[k][j]=temp;
                    
                }
        }
    }
    
    for (int i=0; i<n-1 ; i++) {
        for (int k=i+1; k<n;k++) {
            temp=temp_matrix[k][i]/temp_matrix[i][i];
            for (int j=0; j<=n; j++) {
                temp_matrix[k][j]=temp_matrix[k][j]-temp*temp_matrix[i][j];
            }
        }
    }
    vector<double> x_vector(n,0);
    for (int i=n-1; i>=0; i--) {
        x_vector[i]=temp_matrix[i][n];
        for (int j=0; j<n; j++) {
            
            if(i!=j) {
                x_vector[i]=x_vector[i]-temp_matrix[i][j]*x_vector[j];
                
            }
            
            
        }
        x_vector[i]=x_vector[i]/temp_matrix[i][i];
    }
    answer= x_vector;
}

void Global_Placer::create_output_global_location(std::string outfile) {
    write_output_file.open(outfile);
    int index = 0;
    cout<<endl;
    
    
    
    
    
    
    
    
  for (const auto &item: inst_to_number_code)
  {
      write_output_file<<item.first<<" "<<x_location[index]<<" "<<y_location[index]<<endl;
      cout<<item.first<<" "<<x_location[index]<<" "<<y_location[index]<<endl;
      index++;
  }
    write_output_file.close();
}


















