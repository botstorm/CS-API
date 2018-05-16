# How to build 

~~~~
git clone https://gitlab.com/credits_bc/temp/csconnector.git
cd csconnector
git submodule update --init --recursive
md build64
cd build64
cmake -G "YOUR_COMPILER" ..
~~~~

Will make you 