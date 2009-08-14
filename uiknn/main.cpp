#include "github.h"

vector< Rating > data;
vector< vector<int> > iu(ITEM_NUM), ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int,float> > > wu(USER_NUM), wi(ITEM_NUM);

void loadSim(const char * file, vector< vector< pair<int,float> > > & w){
     ifstream in(file);
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

void loadSim(const char * file, vector< map<int,float> > & w){
     ifstream in(file);
     int a, b;
     float s;
     while(in >> a >> b >> s){
          w[a][b] = s;
          w[b][a] = s;
     }
     cout << "load sim finished!" << endl;
}

void predict(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(int v = 0; v < wu[u].size() && v < 100; ++v){
          int vv = wu[u][v].first;
          float wuv = wu[u][v].second;
          for(int i = 0; i < ui[vv].size(); ++i){
               int ii = ui[vv][i];
               if(rated.find(ii) != rated.end()) continue;
               cand[ii] += wuv;
               for(int j = 0; j < wi[ii].size() && j < 40; ++j){
                    int jj = wi[ii][j].first;
                    float wij = wi[ii][j].second;
                    if(rated.find(jj) != rated.end()) continue;
                    cand[jj] += wuv * wij;
               }
          }
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void predictAll(bool train){
     loadData(data, train);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          iu[item].push_back(user);
     }
     int u;
     if(train){
          loadSim("../data2/knnu-iif.txt.0", wu);
          loadSim("../data2/knni-iuf.txt.0", wi);
     }else{
          loadSim("../data2/knnu-iif.txt", wu);
          loadSim("../data2/knni-iuf.txt", wi);
     }
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-knnui.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(map<int,int>::iterator k = test.begin(); k != test.end(); ++k){
          int u = k->first;
          vector< pair<int,float> > ret;
          predict(u, ret);
          
          out << u << "\t";
          for(int i = 0; i < ret.size() && i < 500; ++i)
               out << ret[i].first << "\t" << ret[i].second / ret[0].second << "\t";
          out << endl;
     }
     out.close();
}

int main(int argc, char ** argv){
     bool train = false;
     if(atoi(argv[1]) == 1) train = true;
     predictAll(train);
     return 0;
}

