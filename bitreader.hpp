#include <fstream>
#include <vector>

//! Class for reading a file bit-by-bit.
/*!	The class provides an interface for reading a file bit-by-bit. It uses a buffer of given size and returns a bool vector containing the bits.*/
class BitReader {
	private:
		char* buffer; /*!< Character buffer. */
		std::ifstream in; /*!< Input file. */
		long int size; /*!< Buffer size. */
		//! A private function to get the bit on the given index.
		/*! 
			\param Index of bit.
			*/
		bool getBit(int);
	public:
		//! Constructor with filename.
		BitReader(std::string);
		//! Destructor.
		~BitReader();
		//! Read into the buffer.
		/*!
			\return Vector of bools containing the read bits.
			*/
		std::vector<bool> read();
		//! Function checking for EOF.
		bool ready();
};
