//: C02:Date1.h
// A first pass at Date.h.
#ifndef DATE1_H
#define DATE1_H
#include <string>
 
class Date {
public:
  // A struct to hold elapsed time:
  struct Duration {
    int years;
    int months;
    int days;
    Duration(int y, int m, int d)
    : years(y), months(m), days(d) {}
  };
  Date();
  Date(int year, int month, int day);
  Date(const std::string&);
  int getYear() const;
  int getMonth() const;
  int getDay() const;
  std::string toString() const;
friend bool operator<(const Date&, const Date&);
friend bool operator>(const Date&, const Date&);
friend bool operator<=(const Date&, const Date&);
friend bool operator>=(const Date&, const Date&);
friend bool operator==(const Date&, const Date&);
friend bool operator!=(const Date&, const Date&);
  friend Duration duration(const Date&, const Date&);
};
#endif // DATE1_H ///:~
