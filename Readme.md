# How to build 

~~~~
git clone 
cd csconnector
git submodule update --init --recursive
mkdir build64
cd build64
cmake -G "YOUR_COMPILER" ..
~~~~ 

### Windows (MSVC)
  open .sln and build from Visual Studio
### Other OS 
  make 


### Requirments

### Windows
Visual Studio 2015 Update 3! or older
### Unix
gcc 4.9 +
#### need to test earlier
clang 3.6 +  
