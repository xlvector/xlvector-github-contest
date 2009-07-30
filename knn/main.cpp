#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int, float> > > w(ITEM_NUM);

void model(){
     loadData(data);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          nu[user]++;
          ni[item]++;
     }
     vector< map<int, int> > co(ITEM_NUM);
     for(int u = 0; u < ui.size(); ++u){
          if(u % 1000 == 0) cout << u << endl;
          for(int i = 0; i < ui[u].size(); ++i){
               int ii = ui[u][i];
               for(int j = i + 1; j < ui[u].size(); ++j){
                    int jj = ui[u][j];
                    co[ii][jj]++;
                    co[jj][ii]++;
               }
          }
     }
     ofstream out("knni-co.txt");
     for(int i = 0; i < co.size(); ++i){
          for(map<int,int>::iterator j = co[i].begin(); j != co[i].end(); ++j){
               double s = (double)(j->second) / (ni[i] + ni[j->first] - (double)(j->second));
               out << i << "\t" << j->first << "\t" << s << endl;
          }
     }
     out.close();
}

void loadSim(){
     ifstream in("knni-co.txt");
     int a, b;
     float s;
     while(in >> a >> b >> s){
          w[a].push_back(make_pair<int,float>(b,s));
     }
     for(int i = 0; i < w.size(); ++i){
          sort(w[i].begin(), w[i].end(), GreaterSecond<int,float>);
     }
     cout << "load sim finished!" << endl;
}

void predict(int u, vector< pair<int,int> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     map<int, int> cand;
     for(int i = 0; i < ui[u].size(); ++i){
          int ii = ui[u][i];
          for(int j = 0; j < w[ii].size() && j < 10; ++j){
               int jj = w[ii][j].first;
               if(rated.find(jj) != rated.end()) continue;
               cand[jj] += 1;
          }
     }
     ret = vector< pair<int,int> >(cand.begin(), cand.end());
     sort(ret.begin(), ret.end(), GreaterSecond<int,int>);
}

void predictAll(){
     loadData(data);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
     }
     int u;
     loadSim();
     ifstream in("../download/test.txt");
     ofstream out("results.txt");
     while(in >> u){
          vector< pair<int,int> > ret;
          predict(u, ret);
          out << u << ":";
          if(!ret.empty()){
               out << ret[0].first;
               for(int i = 1; i < ret.size() && i < 10; ++i)
                    out << "," << ret[i].first;
          }
          out << endl;
     }
     out.close();
}

int main(int argc, char ** argv){
     //model();
     predictAll();
}

