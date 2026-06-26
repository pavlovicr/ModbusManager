# DTSU666-H Energijski merilnik

## Opis
CHINT DTSU666-H je trofazni energijski merilnik za sončne elektrarne.

## Modbus register mapa
OSNOVNE MERITVE


Parameter	Register (dec)	Dolžina	Tip	Faktor	Enota	Opomba
Skupna aktivna energija – Import	0x101E (41246)	2	Float	1	kWh	se povečuje pri uvozu
Skupna aktivna energija – Export	0x1020 (41248)	2	Float	1	kWh	se povečuje pri izvozu
Trenutna aktivna moč (Pt)	0x2012 (8194)	2	Float	0.1	W	+ = import, − = export
Frekvenca	0x2044 (8260)	2	Float	0.01	Hz	
Faktor moči (PFt)	0x202A (8234)	2	Float	0.001	-	+ = uvoz, − = izvoz
                        
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

- [Uradni podatkovni list](https://www.chintglobal.com/content/dam/chint/global/product-center/instruments-meters/electricity-meter/din-rail-meter/dtsu666/manual/DTSU666%20DSSU666%20User%20Manual.pdf)

- [Navodila za namestitev]https://support.huawei.com/enterprise/en/doc/EDOC1100020898


## Nastavitve
- Slave ID: 11
- Baud rate: 9600, 8N1 (če se uporablja RTU)


## Opombe
- Uporablja CDAB bajtni vrstni red (obdelano v kodi).