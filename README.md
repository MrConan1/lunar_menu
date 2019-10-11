# lunar_menu
Decodes &amp; Re-Encodes Lunar Sega Saturn Menu/Item Data
  
Lunar Menu Editor (lunar_menu.exe) Usage
========================================     
lunar_menu.exe decodemenu InputMenuFname [sss]      
lunar_menu.exe encodemenu InputMenuFname InputCsvFname [sss]      
   sss will interpret SSSC JP table as the SSS JP table.       
    
Decoding requires the decompressed binary menu/item data as input (4th binary image embedded within System.dat).  Decoding will output a comma delimited file containing the UTF8 encoded data from the input file as well as important file offsets and control codes.  
  
Following decoding, the 4th column of the .csv file should be modified to contain the english output for the relevant text in column 2 (where applicable).  When ready for encoding, the .csv file should be saved as a UTF8 tab delimited file.  A combination of excel and notepad++ can accomplish this.
  
Encoding takes the original decompressed data and the UTF8 tab delimited English file as input.  Output will encode text with 8 bits per byte.  Control Codes are 16-bit, and records of control codes/text will start and end on 16-bit aligned boundaries.  Not completing on a boundary will result in a filler byte of 0xFF appended at the end.  
