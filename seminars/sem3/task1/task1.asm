%macro string 1-*

 %rep %0
   db %1
   db ','
 %rotate 1
 %endrep

%endmacro

string "hello", "another", "world"
