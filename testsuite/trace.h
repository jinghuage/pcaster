//: C03:Trace.h
// Creating a trace file.
#ifndef TRACE_H
#define TRACE_H
#include <fstream>
 
#ifdef TRACEON
std::ofstream TRACEFILE__("TRACE.OUT");
#define cout TRACEFILE__
#endif
 
#endif // TRACE_H ///:~
