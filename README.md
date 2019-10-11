# lunar_menu
Decodes &amp; Re-Encodes Lunar Sega Saturn Menu/Item Data
  
Lunar Menu Editor (lunar_menu.exe) Usage
========================================     
lunar_menu.exe decodemenu InputMenuFname [sss]      
lunar_menu.exe encodemenu InputMenuFname InputCsvFname [sss]      
   sss will interpret SSSC JP table as the SSS JP table.       
    
Encoding assumes UTF8 characters.  Output will encode text with 8 bits per byte.  Control Codes are 16-bit, and records of control codes/text will start and end on 16-bit aligned boundaries.  Not completing on a boundary will result in a filler byte of 0xFF appended at the end.  
