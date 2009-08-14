#include "github.h"

int main(int argc, char ** argv){
     bool train = false;
     if(atoi(argv[1]) == 1) train = true;
     vector< Rating > data;
     loadData(data,train);
     vector<float> nu(USER_NUM,0), ni(ITEM_NUM,0), nic[2];
     vector< set<int> > ui(USER_NUM);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          nu[user]++;
          ui[user].insert(item);
     }
     for(int c = 0; c < 2; ++c) nic[c] = vector<float>(ITEM_NUM,0);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          int c = 0;
          if(nu[user] > 6) c = 1;
          ni[item] += 1 / log(3 + nu[user]);
          nic[c][item] += 1 / log(3 + nu[user]);
     }
     vector< pair<int,float> > ret, retc[2];
     for(int i = 0; i < ni.size(); ++i)
          if(ni[i] > 0)
               ret.push_back(make_pair<int,float>(i, ni[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
     for(int c = 0; c < 2; ++c){
          for(int i = 0; i < nic[c].size(); ++i)
               if(nic[c][i] > 0)
                    retc[c].push_back(make_pair<int,float>(i, nic[c][i]));
          sort(retc[c].begin(), retc[c].end(), GreaterSecond<int,float>);
     }
     
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-pop.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     file = "../ret2/results-pop-nic.txt";
     if(train) file += ".0";
     ofstream out1(file.c_str());
     for(map<int,int>::iterator k = test.begin(); k != test.end(); ++k){
          int u = k->first;
          int c = 0;
          if(nu[u] > 6) c = 1;
          out << u << "\t";
          out1 << u << "\t";
          int t = 0;
          for(int i = 0; i < ret.size() && t < 500; ++i){
               if(ui[u].find(ret[i].first) != ui[u].end()) continue;
               ++t;
               out << ret[i].first << "\t" << ret[i].second / ret[0].second << "\t";
          }
          out << endl;

          t = 0;
          for(int i = 0; i < retc[c].size() && t < 500; ++i){
               if(ui[u].find(ret[i].first) != ui[u].end()) continue;
               ++t;
               out1 << retc[c][i].first << "\t" << retc[c][i].second / retc[c][0].second << "\t";
          }
          out1 << endl;
     }
     out.close();
     out1.close();
     return 0;
}

