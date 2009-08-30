#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

#define USER_NUM 56555
#define ITEM_NUM 123345

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

template < typename A, typename B >
bool GreaterSecond(const pair<A, B> & a, const pair<A,B> & b){
     return a.second > b.second;
}

struct Rating{
     Rating(int uu, int ii)
          : user(uu), item(ii), value(1)
     {}
     int user;
     int item;
     float value;
};

struct User{
     User(){
          begin_tm = INT_MAX;
          end_tm = 0;
          max_line = 0;
          min_line = INT_MAX;
     }
     map<string,float> language;
     map<string,float> repos;
     int begin_tm;
     int end_tm;
     int max_line;
     int min_line;
};

struct Item{
     Item()
          : line(0)
     {}
     map<string,float> language;
     map<string,float> repos;
     int tm, line;
     string all_lang;
     string reponame;
     map<string,float> name;
     map<string,float> lang_line;
};

typedef vector<User> UserList;
typedef vector<Item> ItemList;

string replace(const string & line, char ch1, char ch2){
     string ret;
     for(int i = 0; i < line.length(); ++i){
          if(line[i] == ch1) ret += ch2;
          else ret += line[i];
     }
     return ret;
}

void loadData(vector< Rating > & data, bool train = false){
     string file = "../download/data.txt";
     if(train) file = "../download/training_data.txt";
     ifstream in(file.c_str());
     string line;
     int umax = 0;
     int imax = 0;
     while(getline(in, line)){
          string tmp;
          for(int i = 0; i < line.length(); ++i){
               if(line[i] == ':') tmp += ' ';
               else tmp += line[i];
          }
          istringstream iss(tmp);
          int user,item;
          iss >> user >> item;
          Rating d(user,item);
          data.push_back(d);
          umax = max<int>(umax,user);
          imax = max<int>(imax,item);
     }
     cout << umax << "\t" << imax << "\t" << data.size() << endl;
}

void getTestSet(map<int,int> & us){
     ifstream in("../download/test.txt");
     int u;
     while(in >> u) us[u] = 0;
     in.close();
}

void getTestSet2(map<int,int> & us){
     ifstream in("../download/removed_values.txt");
     string line;
     while(getline(in,line)){
          line = replace(line, ':',' ');
          istringstream iss(line);
          int u,i;
          iss >> u >> i;
          us[u] = i;
     }
     in.close();
}

void splitName(const string & a, map<string,float> & A){
     string tmp;
     for(int i = 0; i < a.length(); ++i){
          if(a[i] == '-' || a[i] == '_' || a[i] == '.')
               tmp += ' ';
          else tmp += a[i];
     }
     istringstream iss(tmp);
     string buf;
     while(iss >> buf){
          if(buf.length() < 4) continue;
          A[buf] = 1;
          if(buf.length() > 6)
               A[buf.substr(0,5)] = 1;
     }
}

void loadUserItemData(UserList & users, ItemList & items, vector<Rating> & data){
     users = UserList(USER_NUM);
     items = ItemList(ITEM_NUM);
     ifstream in0("../download/lang.txt");
     string line;
     while(getline(in0,line)){
          string tmp;
          for(int i = 0; i < line.length(); ++i){
               if(line[i] == ':' || line[i] == ';' || line[i] == ',')
                    tmp += ' ';
               else tmp += line[i];
          }
          istringstream iss(tmp);
          int id, ln;
          string buf;
          iss >> id;
          set<string> lgs;
          while(iss >> buf >> ln){
               items[id].language[buf] = 1;
               items[id].line += ln;
               items[id].lang_line[buf] = (float)(ln) / 1000;
               items[id].all_lang += buf + ":";
               lgs.insert(buf);
          }
          if(lgs.size() < 2) continue;
          for(set<string>::iterator i = lgs.begin(); i != lgs.end(); ++i){
               for(set<string>::iterator j = i; j != lgs.end(); ++j){
                    if(i == j) continue;
                    items[id].language[*i + *j] = 1;
               }
          }
     }
     in0.close();
     ifstream in1("../data/user-lang.txt");
     int uu;
     float pp;
     string ll;
     while(in1 >> uu >> ll >> pp){
          users[uu].language[ll] = pp;
     }
     in1.close();

     ifstream in2("../download/repos2.txt");
     while(getline(in2,line)){
          istringstream iss(line);
          int id,tm;
          string buf,name;
          iss >> id >> buf >> tm >> name;
          items[id].reponame = buf + "/" + name;
          items[id].repos[buf] = 1;
          items[id].tm = tm;
          splitName(name, items[id].name);
     }
     in2.close();
     ifstream in3("../data/user-repos.txt");
     while(in3 >> uu >> ll >> pp){
          users[uu].repos[ll] = pp;
     }
     in3.close();

     for(int i = 0; i < data.size(); ++i){
          int uu = data[i].user;
          int ii = data[i].item;
          users[uu].begin_tm = min<int>(users[uu].begin_tm, items[ii].tm);
          users[uu].end_tm = max<int>(users[uu].end_tm, items[ii].tm);
          users[uu].max_line = max<int>(users[uu].max_line, items[ii].line);
          users[uu].min_line = min<int>(users[uu].min_line, items[ii].line);
     }
}

double average(vector<float> & a){
     double ret = 0;
     double n = 0;
     for(int i = 0; i < a.size(); ++i){
          if(a[i] == 0) continue;
          ret += a[i];
          n += 1;
     }
     return ret/n;
}


double sim(const set<string> & A, const set<string> & B){
     if(A.empty() || B.empty()) return 0;
     double n = A.size() + B.size();
     double co = 0;
     for(set<string>::const_iterator i = A.begin(); i != A.end(); ++i){
          if(B.find(*i) != B.end())
               co += 1;
     }
     return co / (n - co);
}



double sim(const map<string,float> & A, const map<string,float> & B){
     if(A.empty() || B.empty()) return 0;
     double co = 0;
     double n1 = 0;
     double n2 = 0;
     for(map<string,float>::const_iterator i = A.begin(); i != A.end(); ++i){
          n1 += i->second * i->second;
          map<string,float>::const_iterator p = B.find(i->first);
          if(p == B.end()) continue;
          co += i->second * p->second;
     }
     for(map<string,float>::const_iterator i = B.begin(); i != B.end(); ++i)
          n2 += i->second * i->second;
     return co / sqrt(n1 * n2);
}


double sim2(const map<string,float> & A, const map<string,float> & B){
     if(A.empty() || B.empty()) return 0;
     double co = 0;
     for(map<string,float>::const_iterator i = A.begin(); i != A.end(); ++i){
          map<string,float>::const_iterator p = B.find(i->first);
          if(p == B.end()) continue;
          co += i->second * p->second;
     }
     return co;
}


double rand01(){
     return (double)(rand() % 1000000) / 1000000;
}
void initRandomVector(vector< float > & v, int dim, double s){
     v = vector<float>(dim,0);
     for(int i = 0; i < v.size(); ++i){
          double r = s * (rand01() - 0.5);
          v[i] = r;
     }
}
float dot(const vector<float> & a, const vector<float> & b){
     float ret = 0;
     for(int i = 0; i < a.size(); ++i){
          ret += a[i] * b[i];
     }
     return ret;
}
