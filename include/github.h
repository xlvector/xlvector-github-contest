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
          : user(uu), item(ii)
     {}
     int user;
     int item;
};

void loadData(vector< Rating > & data){
     ifstream in("../download/data.txt");
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
     cout << umax << "\t" << imax << endl;
}
