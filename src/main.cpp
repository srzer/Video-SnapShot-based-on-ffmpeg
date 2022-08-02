#include"VideoContactSheet.h"
using namespace std;
int main(int argc, char** argv){
  if(argc<=1){puts("Error: no input file");return 0;}
  int taskid;cout<<"2 for frame extraction;\n3 for resize and compose a sheet; \n4 for add text.\nEnter which task you want to check:";cin>>taskid;
  if(taskid>=2&&taskid<=4){
    VideoContactSheet process(argv[1]);
    process.work(taskid);
  }
  return 0;
}