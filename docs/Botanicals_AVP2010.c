// RSCF Botanicals

// RSCF type 0 subtype 8 
// magic bytes 03 00 00 F0 01 00 80 52

// DWORD dw1

// dw amt1 
// table (2 dw) x amt1

// dw amt2


// 204 size struct?
// DWORD dw

// amt classes

4b amt{ // 1E
	4b dw1 // 1E
	24b // bbox?
	4b // float 8.9f
	1b b1 // 01
	if b1 == 1 {
		12b // 0.0f 0.0f 0.0f
		12b // 0.0f 4.0f 0.0f
		4b // 0.1f
		4b // 0.4f
		4b // 0.2f
		4b // 50.0f
		4b // 1.0f
		4b // 4.0f
	}
	4b // 1C
	4b // 0
	4b // 18
	4b // 1A
	4b // FFFFFFFF
	4b amt3 // 0
	amt3 * { // hashes
		4b
		4b
	}
	4b amt4 // 0
	amt4 * {
		4b amt5
		amt5 * {
			4b 
			4b amt6 
			amt6 * {
				4b
				1b
				1b
				2b
				4b
				4b
				1b
			}
		}
	}
	
	1b // 0
	4b // 0
	4b // 80.0f
	4b // 80.0f
	4b // 1.0f
	4b // 73.3f
	4b // 1920.0f
	4b // 3
	4b amt7 // 1
	amt7 * {
		4b // 0
		4b // 0
	}
	4b amt8 // 1
	amt8 * {
		4b // 0
		2b // 0
	}
	4b amt9 { // 1
		4b amt { // 1
			4b // 0
		} // 
		4b amt2 { // 1
			2b // 0
		}
		4b amt3 { // 1
			4b amt { // 0 meshes too
				4b   // 
				4b   // 
				4b   // 
				4b   // 
				4b   // 
				4b   // 
				4b   // 
				4b   // material hash
				
				4b amt { // vertex
					12b
					12b
					4b
					4b
					4b
					4b
					4b
					12b
					12b
					4b
				}
				
				4b amt2 { // index
					4b
				}
			}
			
			4b amt2 { // 3 // amount meshes
				4b // -0.0001
				4b // -0.0001
				4b // 1.0f
				4b // 0.5f
				4b // 3.0f
				4b // 5.0f
				4b // 0.5f
				4b // 6A EA 3C 26 material hash
				
				4b amt { // 1C
					12b // 0.0 6.23 0.21 Positions
					12b // -1.0  0.0 0.0 Normals
					4b // FFFFFFFF
					4b // 0              U (UV coords)
					4b // 1.0f           V (UV coords)
					4b // 0
					4b // 1.0f
					12b // 0.0f 1.0f 0.0f
					12b // 0.0f 0.0f -1.0f
					4b // 1.0f
				} // sizeof (72 0x48)
				
				4b amt2 { // 2A
					4b // indexes
				}
			}
			
			4b amt3 { // 0
				4b // 
				4b // 
				4b // 
				4b // 
				4b // 
				4b // 
				4b // 
				4b // 
				
				4b amt { 
					12b 
					4b  
					4b  
					4b  
					4b  
					4b  
					4b  
					4b  
					12b 
					12b 
					12b 
					4b  
				} // sizeof (80 0x50)
				
				4b amt {
					4b
				}
				
			}
			
			4b amt4 { // 0
				4b
				4b
				4b
				4b
				4b
				4b
				4b
				4b
				
				4b amt {
					12b
					4b
					4b
					4b
					4b
					4b
					4b
					4b
					4b
					4b
					4b
					
				}
			}
			
		}
	}
}


4b amount_entries { // 4F4
	4b // 1
	24b // Bound Volume?
	4b amt { // 10
		4b // -21.58   Position x
		4b // -58.09   Position y
		4b // 54.06    Position z
		21b //DC 00 00 00 00 24 00 24
		    //00 FF 00 5E D8 52 08 24
			//4D 17 0B 10 1E
		4b // 0.003
		4b // -0.04
	} // sizeof(41 0x29)
}


