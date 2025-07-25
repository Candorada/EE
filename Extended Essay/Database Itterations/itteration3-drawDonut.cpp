#include <iostream>
#include <string>
#define PI 3.14159265358979626
#include <cmath>
#include <vector>
#include <initializer_list>
using namespace std;
//to learn C i am planning to make a program that draws a donut
//the donut will be drawn in the console
//donut will be drawn using a shader, i will wright the shaderCode in CPP



class vec{
    int size;
    public:
    
    vec operator^(vec o){ //cross product
        int s = min(o.size,size);
        vec c = vec(s);
        vec self = *this;
        switch (s){
        case 3:
            c.x = (vec{self[1],self[2]}^vec{o[1],o[2]}).x;
            c.y = -(vec{self[0],self[2]}^vec{o[0],o[2]}).x;
            c.z = (vec{self[0],self[1]}^vec{o[0],o[1]}).x;
            break;
        default: c.x = self[0]*o[1]-self[1]*o[0];break;}
        return c;
    }
    static vec cross(vec a, vec b){
        return a^b;
    }
    static float angle(vec a, vec b){
        return acos(a*b/(a.magnitude()*b.magnitude()));
    }
    static vec dot(vec a, vec b){
        return a*b;
    }
    vec normalize(){
        float mag = magnitude();
        for (int i = 0; i < size; i++){
            (*this)[i]/=mag;
        }
        return *this;
    }
    template<typename T>
    vec operator*(T m){ // multiplying by a number
        vec n = vec(size);
        for (int i = 0; i < size; i++){
            n[i] = (*this)[i]*m;
        }
        return n;
    }
    template<typename T>
    vec operator/(T m){ // dividing by a number
        vec n = vec(size);
        for (int i = 0; i < size; i++){
            n[i] = (*this)[i]/m;
        }
        return n;
    }
    vec operator+(vec o){ // dividing by a number
        vec n = vec(max(size,o.size));
        for (int i = 0; i < size; i++){
            n[i] = (*this)[i]+o[i];
        }
        return n;
    }
    vec operator-(vec o){ // dividing by a number
        vec n = vec(max(size,o.size));
        for (int i = 0; i < size; i++){
            n[i] = (*this)[i]-o[i];
        }
        return n;
    }
    float operator*(vec o){ //dot product
        vec self = *this;
        int s = min(o.size,size);
        float total = 0;
        for (int i = 0; i < s; i++){
            total += self[i]*o[i];
        }
        return total;
    }
    void setSize(int s){
        int oSize = s;
        for (int i = 3; i >=s; i++){
            (*this)[i] = 0;
        }
        this->size = s;
    }
    static float magnitude(vec v){
        return v.magnitude();
    }
    float magnitude(){
        float total = 0;
        for (int i = 0; i < size; i++){
            total += pow((*this)[i],2);
        }
        return sqrt(total);
    }
    int terms() const {
        return size;
    }
    float x=0,y=0,z=0,w = 0;

    vec(int size){
        this->size = size;
    }
    vec(std::initializer_list<float> args){
        size = args.size();
        for(int i = 0; i < 4 && i<size; i++){
            (*this)[i] = *(args.begin() + i);
        }
    }
    float& operator[](int index){
        switch (index){
            case 0: return x; // works just as well as return this->x;
            case 1: return y; // returns by reference
            case 2: return z;
            case 3: return w;
        default: throw std::out_of_range("index out of range");}
    }
    float** asArray(){
        float** arr = new float*[size];
        for(int i = 0; i < size; i++){
            arr[i] = &(*this)[i];
        }
        return arr;
    }
    float* toArray(){
        float* arr = new float[size];
        for(int i = 0; i < size; i++){
            arr[i] = (*this)[i];
        }
        return arr;
    }
    friend std::ostream& operator<<(std::ostream& os, vec v){
        const int s = v.size;
        os << "vec"<<s<<"(";
        for(int i=0;i<s;i++) 
            os << v[i] << ((i!=s-1)?",":"");
        os<<")";
        return os;
    }
};
class mat{
    public:
    int m,p;
    float* data;
    mat(int m, int p,std::initializer_list<float> args){
        this->m = m;
        this->p = p;
        data = new float[m*p];
        fill(data,data+m*p,0);
        for (int i = 0; i < args.size() && i < m*p; i++){
            data[i] = args.begin()[i];
        }
    }
    std::vector<float> row(int row){
        std::vector<float> r((size_t)p);
        for (size_t i = 0; i < p; i++){
            r[i] = data[row*p + i];
        }
        return r;
    }
    std::vector<float> col(int col){
        std::vector<float> c((size_t)m);
        for (size_t i = 0; i < m; i++){
            c[i] = data[i*m+col];
        }
        return c;
    }
    mat(std::initializer_list<float> args){
        int size = ceil(sqrt(args.size()));
        m = size;
        p = size;
        data = new float[m*p];
        fill(data,data+m*p,0);
        for (int i = 0; i < args.size() && i < m*p; i++){
            data[i] = args.begin()[i];
        }
    }
    mat operator*(float m){
        mat s = *this;
        mat n(s.m,s.p,{});
        for(int i = 0; i < n.m*n.p; i++){
            n.data[i] = s.data[i]*m;
        }
        return n;
    }
    vec operator*(vec vinp){
        mat s = *this;
        if(vinp.terms() != s.p) throw std::out_of_range("matrix and vector dimensions do not match");
        mat v(1,vinp.terms(),{});
        for (int i=0;i<vinp.terms();i++){
            v.data[i] = vinp[i];
        }
        mat newv = v*s;
        vec n(newv.p*newv.m);
        for (int i = 0; i < n.terms(); i++){
            n[i] = newv.data[i];
        }
        return n;
    }
    friend vec operator*(vec v,mat s){
        return s*v;
    }
    mat operator*(mat o){
        mat& s = *this;
        if(s.p != o.m) throw std::out_of_range("matrix dimensions do not match");
        mat n(s.m,o.p,{});
        for(int i = 0; i < n.m*n.p; i++){
            int row = i/n.p;
            int col = i%n.p;
            float sum = 0;
            std::vector<float> r = s.row(row);
            std::vector<float> c = o.col(col);
            for(int j = 0; j < s.p; j++){
                sum += r[j]*c[j];
            }
            n.data[i] = sum;
            // std::cout << i << " "<<n.data[i] << '\n';
        }
        return n;
    }
    friend std::ostream& operator<<(std::ostream& os, mat m) {
        for (int row = 0; row < m.m; row++) {
            os<<'[';
            for (int col = 0; col < m.p; col++) {
                os << m.data[row * m.p + col] << " ";
            }
            os << "\033[1D]\n";  // backspace 1 column, optional
        }
        return os;
    }
};
float sdTorus( vec p, vec t )//c function returns the distance between the point and the torus
{
  vec q = vec{sqrt(p.x*p.x + p.z*p.z)-t.x,p.y};
  return q.magnitude()-t.y;
}
float sdSphere( vec p, float r ){return p.magnitude() - r;}
float sdMap(vec p,float time){
    float deg = time*PI/5.;
    vec pxz = vec{p.x,p.z};
    pxz = pxz*mat{cos(deg),-sin(deg),sin(deg),cos(deg)};
    p.x = pxz.x;
    p.z = pxz.y;
    vec pxy = vec{p.x,p.y};
    deg = time*PI/5.;
    pxy = pxy*mat{cos(deg),-sin(deg),sin(deg),cos(deg)};
    p.x = pxy.x;
    p.y = pxy.y;
    return sdTorus(p,vec{.9,.3});
    //return sdSphere(p,.9);
}
float shaderCode(int x,int y,float time,float width,float height){
    float u = (x*2.-width)/height;
    float v = (y*2.-height)/height;
    const int itter = 10;
    vec lpos{0,-1,-1};
    vec cpos{0,0,-2};
    vec cdir = vec{u,v,1}.normalize();
    float tDist = 0;
    int i;
    for(i=0;i<itter;i++){
        float dist = sdMap(cpos,time);
        tDist += dist;
        cpos = cpos + cdir*dist;
        // std::cout << tDist << '\n';
        if(dist<0.01) break;
    }
    int i2;
    vec ldir = (cpos-lpos).normalize();
    float lightToHitDist = (cpos-lpos).magnitude();
    float ldist = 0;
    for(i2=0;i2<itter;i2++){
        float dist = sdMap(lpos,time);
        ldist += dist;
        lpos = lpos + ldir*dist;
        // std::cout << tDist << '\n';
        if(dist<0.01) break;
    }
    float light = max(0.,20/(4.*3.14*lightToHitDist*lightToHitDist));
    return i!=itter?(
        (cpos-lpos).magnitude()<0.01?light:light/2.
    ):0;
}
int main (){
    // vec point{0,1,2};
    // vec point2{2,1,0};
    // vec cross = point^point2;
    // float dot = point*point2;
    // std::cout << "cross product is " << cross << '\n';
    // std::cout << "dot product is " << dot << '\n';
    // std::cout << "angle between " << point << " and " << point2 << " is " << (vec::angle(point,point2)*180/PI) << '\n';
    // mat m{  1,2,3,
    //         4,5,6,
    //         7,8,9};
    // mat m2{ -1,0,0,
    //         0,-1,0,
    //         0,0,-1};
    // std::cout << (m*m2) << '\n';
    // mat m3(3,1,{1,2,3});
    // std::cout << m3;
    // for (int i = 0; i < m.m; i++) std::cout << m.row(0)[i];
    // std::cout << '\n';
    // mat r{0,-1,
    //       1, 0};
    // vec v{1,1};
    // std::cout << v*r << '\n';
    float time = 0;
    const int w = 30;
    const int h = 30;
    char clrs[8] = " .-+*#@";
    do{
        std::string s = "";
        for (int j=0;j<h;j++){
            for (int i=0;i<w;i++){
                s = s+clrs[(int)max(float(0),min(floor(shaderCode(i,j,time,w,h)*7),(float)6))]+' ';            
            }
            s+="\n";
        }
        time+=0.04;
        std::cout <<s << "--endLine"<< "\033[H"<<std::flush;
    }while(true);
    return 0;

}