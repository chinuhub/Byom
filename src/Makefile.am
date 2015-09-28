bin_PROGRAMS=a.out
a_out_SOURCES=Test.cpp Parser/Program.cpp Parser/Process.cpp SCTransSystem/SCTransSystem.cpp AFA/AFAut.cpp Parser/QiParser.cpp AFA/AFAState.cpp AFA/PassTwo.cpp AFA/PassThree.cpp AFA/PassFourWeak.cpp AFA/PassFourAFA.cpp 
AUTOMAKE_OPTIONS=subdir-objects
AM_LDFLAGS = -L../boost_1_58_0/stage/lib   -L../augeas-1.4.0/src/.libs -L../z3-master/build -L../libfaudes-2_26g
AM_CPPFLAGS=-I../boost_1_58_0 -I../augeas-1.4.0/src -I../z3-master/src/api -I../libfaudes-2_26g/include
AM_CXXFLAGS = -g -std=c++11  
#-DDBGPRNT   
#LIBS = -lboost_regex -lfa -lboost_chrono -lboost_system -lz3
LIBS = -lz3 -Bstatic -l:libboost_regex.a -l:libfa.a -l:libboost_chrono.a -l:libboost_system.a -l:libfaudes.a -lpthread
