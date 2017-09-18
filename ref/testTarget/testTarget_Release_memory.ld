
/* Define each memory region */



  memFlashStart = 0x2000;
  memFlashSize = 0x6000 ;
  memFullRamStart = 0x10000000 ;
  memFullRamSize = 0x2000 ;
  memShareSize = 0x100 ;
  
  
  memRamStart = memFullRamStart ;
  memRamSize = memFullRamSize - memShareSize ;
  memShareStart = memRamStart + memRamSize ;

  
MEMORY
{
  MFlash32 (rx) : ORIGIN = memFlashStart, LENGTH = memFlashSize /* 32K bytes (alias Flash) */  
  RamLoc8 (rwx) : ORIGIN = memRamStart, LENGTH = memRamSize /* 8K bytes (alias RAM) */  
  RamShare (rwx) : ORIGIN = memShareStart, LENGTH = memShareSize /* 256 bytes (alias RAM) */  
}






  /* Define a symbol for the top of each memory region */
  __base_MFlash32 = memFlashStart  ; /* MFlash32 */  
  __base_Flash = memFlashStart ; /* Flash */  
  __top_MFlash32 = memFlashStart + memFlashSize ; /* 32K bytes */  
  __top_Flash = memFlashStart + memFlashSize ; /* 32K bytes */  
  __base_RamLoc8 = memRamStart  ; /* RamLoc8 */  
  __base_RAM = memRamStart ; /* RAM */  
  __top_RamLoc8 = memRamStart + memRamSize ; /* 8K bytes */  
  __top_RAM = memRamStart + memRamSize ; /* 8K bytes */  

  __base_RamShare = memShareStart  ; /* Share mem */  
  __top_RamShare = memShareStart + memShareSize ; /* 8K bytes */  


  