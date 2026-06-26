# SOILHUMIDITY_1  senzor zemeljske vlage

## Opis
Senzor za merjenje vlage v zemlji, temperature, NPK .

## Modbus register mapa
OSNOVNE MERITVE

PRIMER UPORABE 
⚡ Fazne napetosti						
Parameter	Register (dec)	Dolžina	Tip	Faktor	Enota	
Ua	0x2006 (8198)	2	Float	0.1	V	
Ub	0x2008 (8200)	2	Float	0.1	V	
Uc	0x200A (8202)	2	Float	0.1	V	
                        
🔋 Fazni tokovi						
Parameter	Register (dec)	Dolžina	Tip	Faktor	Enota	
Ia	0x200C (8204)	2	Float	0.001	A	
Ib	0x200E (8206)	2	Float	0.001	A	
Ic	0x2010 (8208)	2	Float	0.001	A	
                        
                        
⚙️ Nastavitve in info						
                        
Parameter	Register (dec)	Dolžina	Tip	Opomba		
Software version	0x0000 (0)	1	UInt16	npr. 109 → v1.09		
Programming code	0x0001 (1)	1	UInt16	privzeto 701		
CT direction / polarity	(odvisno od firmware)	1	UInt16	omogoča softversko obrnitev CT		
                        
                        
📖 Kako uporabljati v Modbus Poll						
                        
Function: 03 (Read Holding Registers)						
                        
Start address: npr. 2012						
                        
Quantity: 2 (za Float)						
                        
Display: Float, ABCD (če pokaže napačne vrednosti, poskusi z BACD)						


## Povezave



- [Navodila ]

## Nastavitve
- Slave ID: 11
- Baud rate: 9600, 8N1 (če se uporablja RTU)


## Opombe
- Uporablja CDAB bajtni vrstni red (obdelano v kodi).













