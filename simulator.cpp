#include <malloc.h>
#include <stdio.h>
#include <cstdio>
#include <set>
#include <iostream>
#include <bitset>
#include <fstream>
#include <cctype>
#include <vector>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include "project2.h"
using namespace std;
// Define a map to store the register number and its pointer.
map<string,int*> Registers;
// Translate between real_mem address and simulation address.
void* virtualAddressToReal(int va, void* memoryPointer){
    char* real_me = (char*)memoryPointer;
    real_me = real_me + (va-0x400000);
    void* real_mem = (void*)real_me;
    return real_mem;
}

int realAddressToVirtual(void* real_mem, void* memoryPointer){
    long long r = (long long)real_mem;
    long long m = (long long)memoryPointer;
    int va = (int)(r-m) + 0x400000;
    return va;
}

void add(int* rd, int* rs, int* rt){
    *rd = *rs + *rt;
}

void addu(int* rd, int* rs, int* rt){
    *rd = *rs + *rt;    
}

void addi(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm.find('1')==0){
        *rt = *rs + (-1)*(int)((immediate.flip()).to_ulong()+1);
    }else{
        *rt = *rs + immediate.to_ulong();
    }
}

void addiu(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm.find('1')==0){
        *rt = *rs + (-1)*(int)((immediate.flip()).to_ulong()+1);
    }else{
        *rt = *rs + immediate.to_ulong();
    }
}

void nand(int* rd, int* rs, int* rt){
    bitset<32> first(*rs);
    bitset<32> second(*rt);
    string s = first.to_string();
    string t = second.to_string();
    string d = "";
    for (int i=0;i<32;i++){
        if (s[i]=='1' && t[i]=='1'){
           d+="1";  
        }else{
            d+="0";
        }
    }
    bitset<32> third(d);
    *rd = (int)(third.to_ulong());
}

void nandi(int* rt, int* rs, bitset<16> immediate){
    bitset<32> first(*rs);
    bitset<32> second(immediate.to_string());
    string s = first.to_string();
    string imm = second.to_string();
    string t = "";
    for (int i=0;i<32;i++){
        if (s[i]=='1' && imm[i]=='1'){
           t+="1";  
        }else{
            t+="0";
        }
    }
    bitset<32> third(t);
    *rt = (int)(third.to_ulong());
}

void clo(int* rd, int* rs){
    bitset<32> first(*rs);
    string s = first.to_string();
    int d = 0;
    for (int i=0;i<s.length();i++){
        if (s[i]=='1') d++;
    }
    *rd = d;
}

void clz(int* rd, int* rs){
    bitset<32> first(*rs);
    string s = first.to_string();
    int d = 0;
    for (int i=0;i<s.length();i++){
        if (s[i]=='0') d++;
    }
    *rd = d;
}

void ndiv(int* rs, int* rt, int* lo, int* hi){
    *lo = (*rt)/(*rs);
    *hi = (*rt)%(*rs);
}

void ndivu(int* rs, int* rt, int* lo, int* hi){
    if (*rs < 0) *rs = -(*rs);
    if (*rt < 0) *rt = -(*rt);
    *lo = (*rt)/(*rs);
    *hi = (*rt)%(*rs);
}

void mult(int* rs, int* rt, int* lo, int* hi){
    long long result = (*rs)*(*rt);
    bitset<64> r(result);
    bitset<32> low((r.to_string()).substr(32));
    bitset<32> high((r.to_string()).substr(0,32));
    *lo = (int)(low.to_ulong());
    *hi = (int)(high.to_ulong());
}

void multu(int* rs, int* rt, int* lo, int* hi){
    if (*rs<0) *rs = -(*rs);
    if (*rt<0) *rt = -(*rt);
    long long result = (*rs)*(*rt);
    bitset<64> r(result);
    bitset<32> low((r.to_string()).substr(32));
    bitset<32> high((r.to_string()).substr(0,32));
    *lo = (int)(low.to_ulong());
    *hi = (int)(high.to_ulong());
}

void mul(int* rd, int* rs, int* rt){
    *rd = (*rs)*(*rt);
}

void _or(int* rd, int* rs, int* rt){
    bitset<32> first(*rs);
    bitset<32> second(*rt);
    string s = first.to_string();
    string t = second.to_string();
    string d = "";
    for (int i=0;i<32;i++){
        if (s[i]=='0' && t[i]=='0'){
           d+="0";  
        }else{
            d+="1";
        }
    }
    bitset<32> third(d);
    *rd = (int)(third.to_ulong());    
}

void nor(int* rd, int* rs, int* rt){
    bitset<32> first(*rs);
    bitset<32> second(*rt);
    string s = first.to_string();
    string t = second.to_string();
    string d = "";
    for (int i=0;i<32;i++){
        if (s[i]=='0' && t[i]=='0'){
           d+="1";  
        }else{
            d+="0";
        }
    }
    bitset<32> third(d);
    *rd = (int)(third.to_ulong());    
}

void ori(int* rt, int* rs, bitset<16> immediate){
    bitset<32> first(*rs);
    bitset<32> second(immediate.to_string());
    string s = first.to_string();
    string imm = second.to_string();
    string t = "";
    for (int i=0;i<32;i++){
        if (s[i]=='0' && imm[i]=='0'){
           t+="0";  
        }else{
            t+="1";
        }
    }
    bitset<32> third(t);
    *rt = (int)(third.to_ulong());
}

void sll(int* rd, int* rt, int* shamt){
    bitset<32> t(*rt);
    string first = (t.to_string()).substr(*shamt);
    string second = "";
    for (int i=0;i<*shamt;i++){
        second += "0";
    }
    bitset<32> s(first+second);
    *rd = (int)(s.to_ulong());
}

void sllv(int* rd, int* rt, int* rs){
    bitset<32> t(*rt);
    string first = (t.to_string()).substr(*rs);
    string second = "";
    for (int i=0;i<*rs;i++){
        second += "0";
    }
    bitset<32> s(first+second);
    *rd = (int)(s.to_ulong());
}

void srl(int* rd, int* rt, int* shamt){
    bitset<32> t(*rt);
    string first = (t.to_string()).substr(0,32-*shamt);
    string second = "";
    for (int i=0;i<*shamt;i++){
        second += "0";
    }
    bitset<32> s(second+first);
    *rd = (int)(s.to_ulong());
}

void srlv(int* rd, int* rt, int* rs){
    bitset<32> t(*rt);
    string first = (t.to_string()).substr(0,32-*rs);
    string second = "";
    for (int i=0;i<*rs;i++){
        second += "0";
    }
    bitset<32> s(second+first);
    *rd = (int)(s.to_ulong());
}

void sra(int* rd, int* rt, int* shamt){
    bitset<32> t(*rt);
    string first = (t.to_string()).substr(0,32-*shamt);
    string second = "";
    string third = "0";
    if (*rt < 0) third = "1";
    for (int i=0;i<*shamt;i++){
        second += third;
    }
    bitset<32> s(second+first);
    *rd = (int)(s.to_ulong());
}

void srav(int* rd, int* rt, int* rs){
    bitset<32> t(*rt);
    string first = (t.to_string()).substr(0,32-*rs);
    string second = "";
    string third = "0";
    if (*rt < 0) third = "1";
    for (int i=0;i<*rs;i++){
        second += third;
    }
    bitset<32> s(second+first);
    *rd = (int)(s.to_ulong());
}

void sub(int* rd, int* rs, int* rt){
    *rd = *rs - *rt;
}

void subu(int* rd, int* rs, int* rt){
    *rd = *rs - *rt;
}

void nxor(int* rd, int* rs, int* rt){
    bitset<32> first(*rs);
    bitset<32> second(*rt);
    string s = first.to_string();
    string t = second.to_string();
    string d = "";
    for (int i=0;i<32;i++){
        if (s[i]==t[i]){
           d+="0";  
        }else{
            d+="1";
        }
    }
    bitset<32> third(d);
    *rd = (int)(third.to_ulong());    
}

void nxori(int* rt, int* rs, bitset<16> immediate){
    bitset<32> first(*rs);
    bitset<32> second(immediate.to_string());
    string s = first.to_string();
    string imm = second.to_string();
    string t = "";
    for (int i=0;i<32;i++){
        if (s[i]==imm[i]){
           t+="0";  
        }else{
            t+="1";
        }
    }
    bitset<32> third(t);
    *rt = (int)(third.to_ulong());
}

void lui(int* rt, bitset<16> immediate){
    string imm = immediate.to_string();
    bitset<32> t(imm+"0000000000000000");
    *rt = t.to_ulong();
}

void slt(int* rd, int* rs, int* rt){
    if (*rs<*rt){
        *rd = 1;
    }else{
        *rd = 0;
    }
}

void sltu(int* rd, int* rs, int* rt){
    if (*rs < 0) *rs = -(*rs);
    if (*rt < 0) *rt = -(*rt);
    if (*rs<*rt){
        *rd = 1;
    }else{
        *rd = 0;
    }
}

void slti(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1'){
        imm = imm + "1111111111111111";
    }
    bitset<32> m(imm);
    int third = (int)(m.to_ulong());
    if (*rs<third){
        *rt = 1;
    }else{
        *rt = 0;
    }
}

void sltiu(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    bitset<32> m(imm);
    int third = (int)(m.to_ulong());
    if (*rs < 0) *rs = -(*rs);
    if (*rs<third){
        *rt = 1;
    }else{
        *rt = 0;
    }
}

void beq(int* rs, int* rt, bitset<16> immediate, int* PC){
    if (*rs == *rt){
        string imm = immediate.to_string()+"00";
        bitset<18> mm(imm);
        int immed;
        if (imm[0]=='1') {
            immed = -(int)((mm.flip()).to_ulong() + 1);
        }else {
            immed = (int)(mm.to_ulong());
        }
        *PC = *PC + immed;
    }
}

void bgez(int* rs, bitset<16> immediate, int* PC){
    if (*rs >= 0){
        string imm = immediate.to_string()+"00";
        bitset<18> mm(imm);
        int immed;
        if (imm[0]=='1') {
            immed = -(int)((mm.flip()).to_ulong() + 1);
        }else {
            immed = (int)(mm.to_ulong());
        }
        *PC = *PC + immed;
    }
}

void bgtz(int* rs, bitset<16> immediate, int* PC){
    if (*rs > 0){
        string imm = immediate.to_string()+"00";
        bitset<18> mm(imm);
        int immed;
        if (imm[0]=='1') {
            immed = -(int)((mm.flip()).to_ulong() + 1);
        }else {
            immed = (int)(mm.to_ulong());
        }
        *PC = *PC + immed;
    }
}

void blez(int* rs, bitset<16> immediate, int* PC){
    if (*rs <= 0){
        string imm = immediate.to_string()+"00";
        bitset<18> mm(imm);
        int immed;
        if (imm[0]=='1') {
            immed = -(int)((mm.flip()).to_ulong() + 1);
        }else {
            immed = (int)(mm.to_ulong());
        }
        *PC = *PC + immed;
    }
}

void bltz(int* rs, bitset<16> immediate, int* PC){
    if (*rs < 0){
        string imm = immediate.to_string()+"00";
        bitset<18> mm(imm);
        int immed;
        if (imm[0]=='1') {
            immed = -(int)((mm.flip()).to_ulong() + 1);
        }else {
            immed = (int)(mm.to_ulong());
        }
        *PC = *PC + immed;
    }
}

void bne(int* rs, int* rt, bitset<16> immediate, int* PC){
    if (*rs != *rt){
        string imm = immediate.to_string()+"00";
        bitset<18> mm(imm);
        int immed;
        if (imm[0]=='1') {
            immed = -(int)((mm.flip()).to_ulong() + 1);
        }else {
            immed = (int)(mm.to_ulong());
        }
        *PC = *PC + immed;
    }
}

void _j(string target, int* PC){
    bitset<26> t(target);
    int tar = (int)(t.to_ulong());
    *PC = 4*tar;
}

void jal(string target, int* PC, int* ra){
    *ra = *PC;
    bitset<26> t(target);
    int tar = (int)(t.to_ulong());
    *PC = 4*tar;
}

void jalr(int* rs, int* rd, int* PC){
    *rd = *PC;
    *PC = *rs;    
}

void jr(int* rs, int* PC){
    *PC = *rs;
}

void lb(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm = "1111111111111111"+imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    char* byte = (char*)(virtualAddressToReal(address,memoryPointer));
    bitset<8> result(*byte);
    string r = result.to_string();
    if (r[0]=='1') r ="111111111111111111111111" + r;
    bitset<32> target(r);
    *rt = (int)(target.to_ulong());
}

void lbu(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm = "1111111111111111"+imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    char* byte = (char*)(virtualAddressToReal(address,memoryPointer));
    bitset<8> result(*byte);
    string r = result.to_string();
    bitset<32> target(r);
    *rt = (int)(target.to_ulong());
}

void lh(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm = "1111111111111111"+imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    short* half = (short*)(virtualAddressToReal(address,memoryPointer));
    bitset<16> result(*half);
    string r = result.to_string();
    if (r[0]=='1') r = "1111111111111111" + r;
    bitset<32> target(r);
    *rt = (int)(target.to_ulong());
}

void lhu(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm =  "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    short* half = (short*)(virtualAddressToReal(address,memoryPointer));
    bitset<16> result(*half);
    string r = result.to_string();
    bitset<32> target(r);
    *rt = (int)(target.to_ulong());
}

void lw(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm ="1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    int* word = (int*)(virtualAddressToReal(address,memoryPointer));
    bitset<32> result(*word);
    *rt = (int)(result.to_ulong());
}

void lwl(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm ="1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    bitset<32> a(address);
    bitset<32> b(3);
    int n = (int)((a&b).to_ulong()) + 1;
    char* memaddress = (char* )(virtualAddressToReal(address,memoryPointer));  
    char* rtPart = (char* )rt;
    rtPart +=3;
    for (int i = 0;i< n;i++){
        *rtPart = *memaddress;
        bitset<8> m(*memaddress);
        if (i==n-1) break;
        memaddress++;
        rtPart--;
    }   
}

void lwr(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm =  "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    bitset<32> a(address);
    bitset<32> b(3);
    int n = (int)((a&b).to_ulong());
    int n_inv = 3-n;
    char* memaddress = (char* )(virtualAddressToReal(address,memoryPointer));
    char* rtPart = (char* )rt;
    int* hhh = (int*)memaddress;
    bitset<32> hhhh(*hhh);
    for (int i = 0;i<=n_inv;i++){
        *rtPart = *memaddress;
        bitset<8> h(*memaddress);    
        if (i==n_inv) break;
        memaddress++;
        rtPart++;
    }   
}

void sb(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm = "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());    
    char* memaddress = (char* )(virtualAddressToReal(address,memoryPointer));
    char* rtPart = (char*)rt;
    *memaddress = *rtPart;
}

void sh(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm =  "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());    
    short* memaddress = (short* )(virtualAddressToReal(address,memoryPointer));
    memaddress = (short*)(rt);
}

void sw(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm = "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());    
    int* memaddress = (int* )(virtualAddressToReal(address,memoryPointer));
    *memaddress = *rt;
}

void swl(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm = "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    bitset<32> a(address);
    bitset<32> b(3);
    int n = (int)((a&b).to_ulong());
    char* memaddress = (char* )(virtualAddressToReal(address,memoryPointer));
    char* rtPart = (char* )rt;
    rtPart +=3;
    for (int i = 0;i<=n;i++){
        *memaddress = *rtPart;
        if (i==n) break;
        memaddress++;
        rtPart--;
    }   
}

void swr(int* rt, int* rs, bitset<16> immediate){
    string imm = immediate.to_string();
    if (imm[0]=='1') imm =  "1111111111111111" + imm;
    bitset<32> m(imm);
    int address = *rs + (int)(m.to_ulong());
    bitset<32> a(address);
    bitset<32> b(3);
    int n = (int)((a&b).to_ulong());
    int n_inv = 3-n;
    char* memaddress = (char* )(virtualAddressToReal(address,memoryPointer));
    char* rtPart = (char* )rt;
    for (int i = 0;i<=n_inv;i++){
        *memaddress = *rtPart;
        bitset<8> h(*memaddress);
        if (i==n_inv) break;
        memaddress++;
        rtPart++;
    }   
}

void mfhi(int* rd, int* hi){
    *rd = *hi;
}

void mflo(int* rd, int* lo){
    *rd = *lo;
}

void mthi(int* rs, int* hi){
    *hi = *rs;
}

void mtlo(int* rs, int* lo){
    *lo = *rs;
}

// The following are system call functions.
void systemcall_print_int(int* a0, fstream &outfile){
    outfile.flush();
    int* real_mem;
    if (*a0<0xa00000 && *a0>0x400000){
        real_mem = (int*)(virtualAddressToReal(*a0,memoryPointer));
    }else{
        real_mem = (int*)a0;
    }
    if (outfile.is_open()){
        outfile << *real_mem;
    }
    outfile.flush();
}

void systemcall_print_string(int* a0, fstream &outfile){
    char* real_mem = (char*)(virtualAddressToReal(*a0,memoryPointer));
    while (*real_mem!='\0'){
        outfile.flush();
        outfile << *real_mem;
        real_mem++;
    }
    outfile.flush();
}

void systemcall_print_char(int* a0, fstream &outfile){
    outfile.flush();
    char* real_mem;
    if (*a0<0xa00000 && *a0>0x400000){
        real_mem = (char*)(virtualAddressToReal(*a0,memoryPointer));
    }else{
        real_mem = (char*)a0;
    }
    if (outfile.is_open()){
        outfile << *real_mem;
    } 
    outfile.flush();   
}

void systemcall_read_char(int* v0, fstream &infile){
    char r = infile.get();
    *v0 = *(int*)(&r);
    char* test = (char*)(v0);
    string line;
    getline(infile,line);
}

void systemcall_read_int(int* v0, fstream &infile){
    infile.flush();
    string result;
    getline(infile,result);
    int r = atoi(result.c_str());
    *v0 = r;
}

void systemcall_read_string(int* a0, int* a1, fstream &infile){
    string line;
    getline(infile,line);
    char* real_mem = (char*)(virtualAddressToReal(*a0,memoryPointer));
    for (int i=0;i<*a1;i++){
        *real_mem = line[i];
        real_mem++;
    }
}

void systemcall_sbrk(int* a0, int* v0, void* dynamicDataPointer){
    *v0 = realAddressToVirtual(dynamicDataPointer,memoryPointer);
    char* c = (char*)(dynamicDataPointer);
    c = c + *a0;
    dynamicDataPointer = (void*)c;
}

void systemcall_open(int* a0, int* a1, int* a2){
    char* real_mem = (char*)(virtualAddressToReal(*a0,memoryPointer));
    string fileName = "";
    while(*real_mem != '\0'){
        fileName = fileName + *real_mem;
        real_mem++;
    }
    *a0 = open(fileName.c_str(),*a1,*a2);
}

void systemcall_read(int* a0, int* a1, int* a2){   
    void* buf = virtualAddressToReal(*a1,memoryPointer);
    *a0 = read(*a0,buf,*a2);
}

void systemcall_write(int* a0, int* a1, int* a2){
    void* buf = virtualAddressToReal(*a1,memoryPointer);
    *a0 = write(*a0,buf,*a2);
}

void systemcall_close(int* a0){
    close(*a0);
}

void systemcall_exit2(int* a0){
    exit(*a0);
}

void systemcall_exit(){
    exit(0);
}

void systemcall(int* v0, int* a0, int* a1, int* a2, fstream &infile, fstream &outfile){
    if (*v0 == 1){
        systemcall_print_int(a0,outfile);
    }else if(*v0 == 4){
        systemcall_print_string(a0,outfile);
    }else if(*v0 == 11){
        systemcall_print_char(a0,outfile);
    }else if(*v0 == 5){
        systemcall_read_int(v0,infile);
    }else if(*v0 == 8){
        systemcall_read_string(a0,a1,infile);
    }else if(*v0 == 12){
        systemcall_read_char(v0,infile);
    }else if(*v0 == 9){
        systemcall_sbrk(a0,v0,dynamicDataPointer);
    }else if(*v0 == 10){
        systemcall_exit();
    }else if(*v0 == 13){
        systemcall_open(a0,a1,a2);
    }else if(*v0 == 14){
        systemcall_read(a0,a1,a2);
    }else if(*v0 == 15){
        systemcall_write(a0,a1,a2);
    }else if(*v0 == 16){
        systemcall_close(a0);
    }else if(*v0 == 17){
        systemcall_exit2(a0);
    }
}

// Memory simulation.
void memorySimulation(void* &memoryPointer, int* &textPointer, dataBlock* &staticDataPointer, dataBlock* &dynamicDataPointer, string* &stackPointer);
// Put .data in data segment.
void dataSegmentSimulation(dataBlock* &staticDataPointer, dataBlock* &dynamicDataPointer, string inputfileName);
// Put .text in text segment.
void textSegmentSimulation(int* &textPointer, string inputfileName);
// Register simulation.
void registerSimulation(map<string,int*> &Registers);
// Start simulating.
void startSimulating(int* PC, string in_file, string out_file);
// Checkpoint
std::set<int> checkpoints;
void init_checkpoints(string checkpoint_file);
void checkpoint_memory(int ins_count);
void checkpoint_register(int ins_count);
int main(int argc, char * argv[]){
    string asm_file, instruction_file, checkpoint_file, in_file, out_file;
    asm_file = (string)argv[1];
    instruction_file = (string)argv[2];
    checkpoint_file = (string)argv[3];
    in_file = (string)argv[4];
    out_file = (string)argv[5];
    memorySimulation(memoryPointer,textPointer,staticDataPointer,dynamicDataPointer,stackPointer);
    textSegmentSimulation(textPointer,instruction_file);
    dataSegmentSimulation(staticDataPointer,dynamicDataPointer,asm_file);
    registerSimulation(Registers);
    init_checkpoints(checkpoint_file);
    startSimulating(Registers["PC"],in_file,out_file);
    return 0;
}

void memorySimulation(void* &memoryPointer, int* &textPointer, dataBlock* &staticDataPointer, dataBlock* &dynamicDataPointer, string* &stackPointer){
    memoryPointer = malloc(0x600000);
    textPointer = (int*)(memoryPointer);
    staticDataPointer = (dataBlock*)memoryPointer;
    staticDataPointer = staticDataPointer + 262144;
    dynamicDataPointer = staticDataPointer;
    stackPointer = (string*)(memoryPointer);
    stackPointer = stackPointer + 196608;

}

void dataSegmentSimulation(dataBlock* &staticDataPointer, dataBlock* &dynamicDataPointer, string inputfileName){
    fstream inputf;
    inputf.open(inputfileName.c_str());
    string line;
    while (getline(inputf,line)){
        if (line.find(".data") != string ::npos) break;
    }
    while (getline(inputf,line)){
        if (line.find(".text") != string ::npos) break;
        // Remove the comments part.
        if (line.find('#') !=string ::npos){
            line = line.substr(0,line.find('#'));
        }
        if (line.empty()) continue;        
        if (line.find(':') != string ::npos){
            string dataType = line.substr(line.find('.'),line.find(' ',line.find('.'))-line.find('.'));
            int Type;
            if (dataType==".asciiz") Type=1;
            if (dataType==".ascii") Type=2;
            if (dataType==".word") Type=3;
            if (dataType==".byte") Type=4;
            if (dataType==".half") Type=5;
            switch (Type)
            {
            case 1:{
                string content = line.substr(line.find('"')+1,line.find_last_of('"')-line.find('"')-1);
                while (content.find('\\') != string ::npos){
                    char c = content[content.find('\\')+1];
                    switch (c)
                    {
                    case 'a': c = '\a';
                        break;
                    case 'b': c = '\b';
                        break;
                    case 'f': c = '\f';
                        break;
                    case 'n': c = '\n';
                        break;
                    case 'r': c = '\r';
                        break;
                    case 't': c = '\t';
                        break;
                    case 'v': c = '\v';
                        break;
                    case '0': c = '\0';
                        break;
                    case '?': c = '\?';
                        break;
                    default:
                        break;
                    }
                content = content.substr(0,content.find('\\')) + c + content.substr(content.find('\\')+2);
                }
                int dataBlockNumber = (content.length()+1)/4;
                if ((content.length()+1)%4 != 0) dataBlockNumber++; 
                for (int i=0;i<dataBlockNumber;i++){
                    dataBlock blocki;
                    *staticDataPointer = blocki;
                    if (i*4 == content.length()){
                        staticDataPointer->charData.c1 = '\0';
                        break;
                    }else{
                        staticDataPointer->charData.c1 = content[i*4];
                    }
                    if (i*4+1 == content.length()){
                        staticDataPointer->charData.c2 = '\0';
                        break;
                    }else{
                        staticDataPointer->charData.c2 = content[i*4+1];
                    }                    
                    if (i*4+2 == content.length()){
                        staticDataPointer->charData.c3 = '\0';
                        break;
                    }else{
                        staticDataPointer->charData.c3 = content[i*4+2];
                    }
                    if (i*4+3 == content.length()){
                        staticDataPointer->charData.c4 = '\0';
                        break;
                    }else{
                        staticDataPointer->charData.c4 = content[i*4+3];
                    }
                    staticDataPointer++;                    
                }
                staticDataPointer++;                    
                break;}
            case 2:{
                string content = line.substr(line.find('"')+1,line.find_last_of('"')-line.find('"')-1);
                while (content.find('\\') != string ::npos){
                    char c = content[content.find('\\')+1];
                    switch (c)
                    {
                    case 'a': c = '\a';
                        break;
                    case 'b': c = '\b';
                        break;
                    case 'f': c = '\f';
                        break;
                    case 'n': c = '\n';
                        break;
                    case 'r': c = '\r';
                        break;
                    case 't': c = '\t';
                        break;
                    case 'v': c = '\v';
                        break;
                    case '0': c = '\0';
                        break;
                    case '?': c = '\?';
                        break;
                    default:
                        break;
                    }
                    content = content.substr(0,content.find('\\')) + c + content.substr(content.find('\\')+2);
                }
                int dataBlockNumber = (content.length())/4;
                if ((content.length())%4 != 0) dataBlockNumber++;
                for (int i=0;i<dataBlockNumber;i++){
                    dataBlock blocki;
                    *staticDataPointer = blocki;
                    staticDataPointer->charData.c1 = content[i*4];
                    if (i*4+1 == content.length()) break;
                    staticDataPointer->charData.c2 = content[i*4+1];
                    if (i*4+2 == content.length()) break;                  
                    staticDataPointer->charData.c3 = content[i*4+2];
                    if (i*4+3 == content.length()) break;
                    staticDataPointer->charData.c4 = content[i*4+3];
                    if (i*4+4 == content.length()) break;
                    staticDataPointer++;
                   }
                    staticDataPointer++;
                break;}
            case 3:{
                string content = line.substr(line.find(".word")+6);
                int position = 0;
                int dotNumber = 0;
                vector<int> wordTypeData;
                while (content.find(',',position) != string ::npos){
                    dotNumber++;
                    wordTypeData.push_back(atoi((content.substr(position,content.find(',',position)-position)).c_str()));
                    position=content.find(',',position)+1;
                }
                wordTypeData.push_back(atoi((content.substr(position).c_str())));
                int blockNumber = wordTypeData.size();                
                for (int i=0;i<blockNumber;i++){
                    dataBlock block;
                    block.intData = wordTypeData[i];
                    *staticDataPointer = block; 
                    staticDataPointer++;                   
                }
                break;}          
            case 4:{
                string content = line.substr(line.find(".byte")+6);
                int position=0;
                int dotNumber = 0;
                vector<char> byteTypeData;
                while (content.find(',',position) != string ::npos){
                    byteTypeData.push_back(atoi((content.substr(position,content.find(',',position)-position)).c_str()));
                    position=content.find(',',position)+1;
                }
                byteTypeData.push_back(atoi((content.substr(position)).c_str()));
                int blockNumber = dotNumber/4+1;
                for (int i=0;i<blockNumber;i++){
                    dataBlock blocki;
                    *staticDataPointer = blocki;
                    staticDataPointer->charData.c1 = byteTypeData[4*i];
                    if (i*4+1==byteTypeData.size()) break;
                    staticDataPointer->charData.c2 = byteTypeData[4*i+1];
                    if (i*4+2==byteTypeData.size()) break;
                    staticDataPointer->charData.c3 = byteTypeData[4*i+2];
                    if (i*4+3==byteTypeData.size()) break;
                    staticDataPointer->charData.c4 = byteTypeData[4*i+3];
                    if (i*4+4==byteTypeData.size()) break;
                    staticDataPointer++;
                }
                    staticDataPointer++;
                break;}
            case 5:{
                string content = line.substr(line.find(".half")+6);
                int position=0;
                int dotNumber = 0;
                vector<short int> halfTypeData;
                while (content.find(',',position) != string ::npos){
                    dotNumber++;
                    halfTypeData.push_back(atoi((content.substr(position,content.find(',',position)-position)).c_str()));
                    position=content.find(',',position)+1;
                }
                halfTypeData.push_back(atoi((content.substr(position).c_str())));
                int blockNumber = dotNumber/2+1;
                for (int i=0;i<blockNumber;i++){
                    dataBlock blocki;
                    *staticDataPointer = blocki;
                    staticDataPointer->halfData.s1 = halfTypeData[i*2]; 
                    if (i*2+1==halfTypeData.size()) break;
                    staticDataPointer->halfData.s2 = halfTypeData[i*2+1];
                    if (i*2+2==halfTypeData.size()) break;
                    staticDataPointer++;
                }
                staticDataPointer++;
                break;}
            default:
                break;
            }
        }
    }
    dynamicDataPointer = staticDataPointer;
    inputf.close();    
}

void textSegmentSimulation(int* &textPointer, string inputfileName){
    fstream inputf;
    inputf.open(inputfileName.c_str());
    string line;
    while (getline(inputf,line)){
        bitset<32> content(line);
        *textPointer = content.to_ulong();
        textPointer++;
    }
    inputf.close();
}

void registerSimulation(map<string,int*> &Registers){
    // Register store virtual address when store address.
    int* $zero = (int*)malloc(4);   Registers.insert(pair<string,int*>("00000",$zero)); *$zero = 0;
    int* $at = (int*)malloc(4);     Registers.insert(pair<string,int*>("00001",$at));   *$at = 0;
    int* $v0 = (int*)malloc(4);     Registers.insert(pair<string,int*>("00010",$v0));   *$v0 = 0;
    int* $v1 = (int*)malloc(4);     Registers.insert(pair<string,int*>("00011",$v1));   *$v1 = 0;
    int* $a0 = (int*)malloc(4);     Registers.insert(pair<string,int*>("00100",$a0));   *$a0 = 0;
    int* $a1 = (int*)malloc(4);     Registers.insert(pair<string,int*>("00101",$a1));   *$a1 = 0;
    int* $a2 = (int*)malloc(4);     Registers.insert(pair<string,int*>("00110",$a2));   *$a2 = 0;
    int* $a3 = (int*)malloc(4);     Registers.insert(pair<string,int*>("00111",$a3));   *$a3 = 0;
    int* $t0 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01000",$t0));   *$t0 = 0;
    int* $t1 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01001",$t1));   *$t1 = 0;
    int* $t2 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01010",$t2));   *$t2 = 0;
    int* $t3 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01011",$t3));   *$t3 = 0;
    int* $t4 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01100",$t4));   *$t4 = 0;
    int* $t5 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01101",$t5));   *$t5 = 0;
    int* $t6 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01110",$t6));   *$t6 = 0;
    int* $t7 = (int*)malloc(4);     Registers.insert(pair<string,int*>("01111",$t7));   *$t7 = 0;
    int* $s0 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10000",$s0));   *$s0 = 0;
    int* $s1 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10001",$s1));   *$s1 = 0;
    int* $s2 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10010",$s2));   *$s2 = 0;
    int* $s3 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10011",$s3));   *$s3 = 0;
    int* $s4 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10100",$s4));   *$s4 = 0;
    int* $s5 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10101",$s5));   *$s5 = 0;
    int* $s6 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10110",$s6));   *$s6 = 0;
    int* $s7 = (int*)malloc(4);     Registers.insert(pair<string,int*>("10111",$s7));   *$s7 = 0;
    int* $t8 = (int*)malloc(4);     Registers.insert(pair<string,int*>("11000",$t8));   *$t8 = 0;
    int* $t9 = (int*)malloc(4);     Registers.insert(pair<string,int*>("11001",$t9));   *$t9 = 0;
    int* $k0 = (int*)malloc(4);     Registers.insert(pair<string,int*>("11010",$k0));   *$k0 = 0;
    int* $k1 = (int*)malloc(4);     Registers.insert(pair<string,int*>("11011",$k1));   *$k1 = 0;
    int* $gp = (int*)malloc(4);     Registers.insert(pair<string,int*>("11100",$gp));   *$gp = 0x508000;
    int* $sp = (int*)malloc(4);     Registers.insert(pair<string,int*>("11101",$sp));   *$sp = 0xa00000;
    int* $fp = (int*)malloc(4);     Registers.insert(pair<string,int*>("11110",$fp));   *$fp = 0xa00000;
    int* $ra = (int*)malloc(4);     Registers.insert(pair<string,int*>("11111",$ra));   *$ra = 0;
    int* PC = (int*)malloc(4);      Registers.insert(pair<string,int*>("PC",PC));       *PC = 0x400000;
    int* lo = (int*)malloc(4);      Registers.insert(pair<string,int*>("lo",lo));   *lo = 0;
    int* hi = (int*)malloc(4);      Registers.insert(pair<string,int*>("hi",hi));   *hi = 0;
    int* shamt = (int*)malloc(4);   Registers.insert(pair<string,int*>("shamt",shamt));   *shamt = 0;
}

void startSimulating(int* PC, string in_file, string out_file){
    ofstream fout(out_file.c_str());
    if (!fout){
        cout << "Error.";
    }
    fout.close();
    fstream outfile,infile;
    infile.open(in_file.c_str());
    outfile.open(out_file.c_str());
    vector<int> checkNumber;
    set<int>::iterator it;
    for(it=checkpoints.begin ();it!=checkpoints.end ();it++)
    {
        checkNumber.push_back(*it);
    }
    void* text = (void*)textPointer;
    int count = 1;
    int syscount = 1;
    int checkPointCount = 0;
    while (*PC != realAddressToVirtual(text,memoryPointer)){
        if (checkPointCount<checkNumber.size() && count == checkNumber[checkPointCount]+1){
            checkpoint_memory(checkNumber[checkPointCount]);
            checkpoint_register(checkNumber[checkPointCount]);
            checkPointCount++;           
        }
        count++;
        int* real_mem = (int*)(virtualAddressToReal(*PC,memoryPointer));
        *PC = *PC + 4;
        bitset<32> textCode(*real_mem);
        string TextCode = textCode.to_string();
        // Get the instruction type through the opcode and excute the instruction.
        if(TextCode.substr(0,6)=="000000"){
            int* rs = Registers[TextCode.substr(6,5)];
            int* rt = Registers[TextCode.substr(11,5)];
            int* rd = Registers[TextCode.substr(16,5)];
            bitset<5> sam(TextCode.substr(21,5));
            *Registers["shamt"] = (int)(sam.to_ulong());
            int* sa =  Registers["shamt"];
            string fct = TextCode.substr(26);
            if (fct == "100000"){
                add(rd,rs,rt);
            }else if (fct == "100001"){
                addu(rd,rs,rt);
            }else if (fct == "100100"){
                nand(rd,rs,rt);
            }else if (fct == "011010"){
                ndiv(rs,rt,Registers["lo"],Registers["hi"]);
            }else if (fct == "011011"){
                ndivu(rs,rt,Registers["lo"],Registers["hi"]);
            }else if (fct == "001001"){
                jalr(rs,rd,Registers["PC"]);
            }else if (fct == "001000"){
                jr(rs,Registers["PC"]);
            }else if (fct == "010000"){
                mfhi(rd,Registers["hi"]);
            }else if (fct == "010010"){
                mflo(rd,Registers["lo"]);
            }else if (fct == "010001"){
                mthi(rd,Registers["hi"]);
            }else if (fct == "010011"){
                mtlo(rd,Registers["lo"]);
            }else if (fct == "011000"){
                mult(rs,rt,Registers["lo"],Registers["hi"]);
            }else if (fct == "011001"){
                multu(rs,rt,Registers["lo"],Registers["hi"]);
            }else if (fct == "100111"){
                nor(rd,rs,rt);
            }else if (fct == "100101"){
                _or(rd,rs,rt);
            }else if (fct == "000000"){
                sll(rd,rt,sa);
            }else if (fct == "000100"){
                sllv(rd,rt,rs);
            }else if (fct == "101010"){
                slt(rd,rs,rt);
            }else if (fct == "101011"){
                sltu(rd,rs,rt);
            }else if (fct == "000011"){
                sra(rd,rt,sa);
            }else if (fct == "000111"){
                srav(rd,rt,rs);
            }else if (fct == "000010"){
                srl(rd,rt,sa);
            }else if (fct == "000110"){
                srlv(rd,rt,rs);
            }else if (fct == "100010"){
                sub(rd,rs,rt);
            }else if (fct == "100011"){
                subu(rd,rs,rt);
            }else if (fct == "001100"){
                systemcall(Registers["00010"],Registers["00100"],Registers["00101"],Registers["00110"],infile,outfile);
                syscount++;
            }else if (fct == "100110"){
                nxor(rd,rs,rt);
            }  
        }else if(TextCode.substr(0,6)=="000010"){
        string target = TextCode.substr(6);
        _j(target,Registers["PC"]);
    }else if(TextCode.substr(0,6)=="000011"){
        string target = TextCode.substr(6);
        jal(target,Registers["PC"],Registers["11111"]);
    }else{
        string opcode = TextCode.substr(0,6);
        int* rs = Registers[TextCode.substr(6,5)];
        int* rt = Registers[TextCode.substr(11,5)];
        bitset<16> immediate(TextCode.substr(16));
        if(opcode == "001000"){
            addi(rt,rs,immediate);
        }else if(opcode == "001001"){
            addiu(rt,rs,immediate);
        }else if(opcode == "001100"){
            nandi(rt,rs,immediate);
        }else if(opcode == "000100"){
            beq(rs,rt,immediate,Registers["PC"]);
        }else if(opcode == "000001" && TextCode.substr(11,5)=="00001"){
            bgez(rs,immediate,Registers["PC"]);
        }else if(opcode == "000111"){
            bgtz(rs,immediate,Registers["PC"]);
        }else if(opcode == "000110"){
            blez(rs,immediate,Registers["PC"]);
        }else if(opcode == "000001" && TextCode.substr(11,5)=="00000"){
            bltz(rs,immediate,Registers["PC"]);
        }else if(opcode == "000101"){
            bne(rs,rt,immediate,Registers["PC"]);
        }else if(opcode == "100000"){
            lb(rt,rs,immediate);
        }else if(opcode == "100100"){
            lbu(rt,rs,immediate);
        }else if(opcode == "100001"){
            lh(rt,rs,immediate);
        }else if(opcode == "100101"){
            lhu(rt,rs,immediate);
        }else if(opcode == "001111"){
            lui(rt,immediate);
        }else if(opcode == "100011"){
            lw(rt,rs,immediate);
        }else if(opcode == "001101"){
            ori(rt,rs,immediate);
        }else if(opcode == "101000"){
            sb(rt,rs,immediate);
        }else if(opcode == "001010"){
            slti(rt,rs,immediate);
        }else if(opcode == "001011"){
            sltiu(rt,rs,immediate);
        }else if(opcode == "101001"){
            sh(rt,rs,immediate);
        }else if(opcode == "101011"){
            sw(rt,rs,immediate);
        }else if(opcode == "001110"){
            nxori(rt,rs,immediate);
        }else if(opcode == "100010"){
            lwl(rt,rs,immediate);
        }else if(opcode == "100110"){
            lwr(rt,rs,immediate);
        }else if(opcode == "101010"){
            swl(rt,rs,immediate);
        }else if(opcode == "101110"){
            swr(rt,rs,immediate);
        }
    }
}
    infile.close();
    outfile.close();
}

void init_checkpoints(string checkpoint_file){
    FILE *fp = fopen(checkpoint_file.c_str(), "r");
    int tmp, i = 0;
    while(fscanf(fp, "%d", &tmp) != EOF){
        checkpoints.insert(tmp);
    }
}

void checkpoint_memory(int ins_count){
    if (!checkpoints.count(ins_count)){
        return;
    }
    string name = "memory_" + to_string(ins_count) + ".bin";
    FILE * fp = fopen(name.c_str(), "wb");
    fwrite(memoryPointer, 1, 0x600000, fp);
    fclose(fp);
}

void checkpoint_register(int ins_count){
    if (!checkpoints.count(ins_count)){
        return;
    }
    string name = "register_" + to_string(ins_count) + ".bin";
    FILE * fp = fopen(name.c_str(), "wb");
    fwrite((void*)Registers["00000"], 4, 1, fp);
    fwrite((void*)Registers["00001"], 4, 1, fp);
    fwrite((void*)Registers["00010"], 4, 1, fp);
    fwrite((void*)Registers["00011"], 4, 1, fp);
    fwrite((void*)Registers["00100"], 4, 1, fp);
    fwrite((void*)Registers["00101"], 4, 1, fp);
    fwrite((void*)Registers["00110"], 4, 1, fp);
    fwrite((void*)Registers["00111"], 4, 1, fp);
    fwrite((void*)Registers["01000"], 4, 1, fp);
    fwrite((void*)Registers["01001"], 4, 1, fp);
    fwrite((void*)Registers["01010"], 4, 1, fp);
    fwrite((void*)Registers["01011"], 4, 1, fp);
    fwrite((void*)Registers["01100"], 4, 1, fp);
    fwrite((void*)Registers["01101"], 4, 1, fp);
    fwrite((void*)Registers["01110"], 4, 1, fp);
    fwrite((void*)Registers["01111"], 4, 1, fp);
    fwrite((void*)Registers["10000"], 4, 1, fp);
    fwrite((void*)Registers["10001"], 4, 1, fp);
    fwrite((void*)Registers["10010"], 4, 1, fp);
    fwrite((void*)Registers["10011"], 4, 1, fp);
    fwrite((void*)Registers["10100"], 4, 1, fp);
    fwrite((void*)Registers["10101"], 4, 1, fp);
    fwrite((void*)Registers["10110"], 4, 1, fp);
    fwrite((void*)Registers["10111"], 4, 1, fp);
    fwrite((void*)Registers["11000"], 4, 1, fp);
    fwrite((void*)Registers["11001"], 4, 1, fp);
    fwrite((void*)Registers["11010"], 4, 1, fp);
    fwrite((void*)Registers["11011"], 4, 1, fp);
    fwrite((void*)Registers["11100"], 4, 1, fp);
    fwrite((void*)Registers["11101"], 4, 1, fp);
    fwrite((void*)Registers["11110"], 4, 1, fp);
    fwrite((void*)Registers["11111"], 4, 1, fp);
    fwrite((void*)Registers["PC"], 4, 1, fp);
    fwrite((void*)Registers["hi"], 4, 1, fp);
    fwrite((void*)Registers["lo"], 4, 1, fp);

    fclose(fp);

}