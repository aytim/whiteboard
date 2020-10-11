//! Huffman codes given file.
/*!
	Codes well-formatted .qd files into binary .wb file. The .wb file (stands for whiteboard) contains the width and height of the image on the first four bytes (two and two, respectively) and the number of bad bits on the next three bits, then the data. The function also prints the character-symbol mapping into a .sym file.
	\param Input filename.
	*/
void huffman(std::string);
//!
/*!
	\param Input filename.
	*/
void dehuffman(std::string);
